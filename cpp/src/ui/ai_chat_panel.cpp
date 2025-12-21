#include "ai_chat_panel.hpp"
#include "core/config.hpp"
#include "core/storage.hpp"
#include <QFrame>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTimer>
#include <QVBoxLayout>

namespace DevPlanner {

const QString AIChatPanel::OPENROUTER_API_URL =
    "https://openrouter.ai/api/v1/chat/completions";

const QString AIChatPanel::SYSTEM_PROMPT =
    R"(Ты — AI-исполнитель Dev Planner. Твоя задача — НЕМЕДЛЕННО выполнять команды пользователя.
ГЛАВНОЕ ПРАВИЛО: ВСЕГДА отвечай ТОЛЬКО JSON-командой! Никакого текста кроме JSON!
ЦВЕТА ЗАДАЧ:
- "жёлтый" = progress
- "красный" = todo
- "зелёный" = done
- "серый" = none
СОЗДАНИЕ:
{"action": "create_task", "title": "...", "description": "...", "status": "todo"}
{"action": "create_tasks_chain", "tasks": [...], "connect": true}
СТАТУСЫ:
{"action": "set_status", "task": 1, "status": "progress"}
СОЕДИНЕНИЯ:
{"action": "connect", "from": 1, "to": 2}
УДАЛЕНИЕ:
{"action": "clear_all"}
РАСПОЛОЖЕНИЕ:
{"action": "arrange_grid"}
{"action": "arrange_tree"})";

ChatMessage::ChatMessage(const QString &text, bool isUser, QWidget *parent)
    : QFrame(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(16, 12, 16, 12);
  layout->setSpacing(6);

  auto *label = new QLabel(isUser ? "ВЫ" : "AI ASSISTANT", this);
  label->setStyleSheet(
      QString(
          "color: %1; font-size: 9px; font-weight: 900; letter-spacing: 2px;")
          .arg(isUser ? "#d900ff" : "#00ff9d"));
  label->setAlignment(isUser ? Qt::AlignRight : Qt::AlignLeft);

  auto *message = new QLabel(text, this);
  message->setWordWrap(true);
  message->setStyleSheet(
      QString("color: %1; font-size: 14px;")
          .arg(isUser ? "#ffffff" : "rgba(255,255,255,0.9)"));
  message->setTextInteractionFlags(Qt::TextSelectableByMouse);

  layout->addWidget(label);
  layout->addWidget(message);

  QString bg = isUser ? "rgba(217, 0, 255, 0.15)" : "rgba(0, 255, 157, 0.08)";
  QString border =
      isUser ? "rgba(217, 0, 255, 0.4)" : "rgba(0, 255, 157, 0.25)";
  QString marginStyle = isUser ? "margin-left: 40px;" : "margin-right: 40px;";

  setStyleSheet(
      QString("ChatMessage { background: %1; border: 1px solid %2; "
              "border-radius: 16px; %3 margin-top: 6px; margin-bottom: 6px; }")
          .arg(bg, border, marginStyle));
}

AIChatPanel::AIChatPanel(QWidget *parent) : GlassmorphismWidget(parent) {
  m_networkManager = new QNetworkAccessManager(this);
  connect(m_networkManager, &QNetworkAccessManager::finished, this,
          &AIChatPanel::onNetworkReply);
  m_apiKey = Storage::loadApiKey();
  m_models = Storage::loadModels(m_currentModel);

  QJsonObject systemMsg;
  systemMsg["role"] = "system";
  systemMsg["content"] = SYSTEM_PROMPT;
  m_messages.append(systemMsg);

  setupUI();
}

void AIChatPanel::setupUI() {
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(15, 15, 15, 15);
  layout->setSpacing(15);

  auto *headerLayout = new QHBoxLayout();
  auto *header = new QLabel("AI CO-PILOT", this);
  header->setStyleSheet("color: #ffffff; font-weight: 900; letter-spacing: "
                        "3px; font-size: 13px;");
  headerLayout->addWidget(header);
  headerLayout->addStretch();

  auto *apiKeyBtn = new QPushButton("⚙", this);
  apiKeyBtn->setFixedSize(32, 32);
  apiKeyBtn->setCursor(Qt::PointingHandCursor);
  apiKeyBtn->setStyleSheet(
      "QPushButton { background: rgba(255,255,255,0.05); color: #ffffff; "
      "border-radius: 16px; font-size: 16px; } QPushButton:hover { background: "
      "rgba(217,0,255,0.2); }");
  connect(apiKeyBtn, &QPushButton::clicked, this, &AIChatPanel::onApiKeySetup);
  headerLayout->addWidget(apiKeyBtn);

  layout->addLayout(headerLayout);

  m_scrollArea = new QScrollArea(this);
  m_scrollArea->setWidgetResizable(true);
  m_scrollArea->setStyleSheet(
      "QScrollArea { background: transparent; border: none; } "
      "QScrollBar:vertical { background: transparent; width: 4px; } "
      "QScrollBar::handle:vertical { background: rgba(255,255,255,0.1); "
      "border-radius: 2px; }");

  m_messagesWidget = new QWidget(this);
  m_messagesWidget->setStyleSheet("background: transparent;");
  m_messagesLayout = new QVBoxLayout(m_messagesWidget);
  m_messagesLayout->setAlignment(Qt::AlignTop);
  m_messagesLayout->setSpacing(12);
  m_scrollArea->setWidget(m_messagesWidget);
  layout->addWidget(m_scrollArea, 1);

  auto *inputFrame = new QFrame(this);
  inputFrame->setStyleSheet(
      "QFrame { background: rgba(255,255,255,0.05); border: 1px solid "
      "rgba(255,255,255,0.1); border-radius: 20px; }");
  auto *inputLayout = new QHBoxLayout(inputFrame);
  inputLayout->setContentsMargins(15, 5, 5, 5);

  m_inputField = new QLineEdit(this);
  m_inputField->setPlaceholderText("Задайте вопрос...");
  m_inputField->setStyleSheet("QLineEdit { background: transparent; border: "
                              "none; color: #ffffff; font-size: 14px; }");
  connect(m_inputField, &QLineEdit::returnPressed, this,
          &AIChatPanel::onSendClicked);

  m_sendBtn = new QPushButton("➤", this);
  m_sendBtn->setFixedSize(32, 32);
  m_sendBtn->setCursor(Qt::PointingHandCursor);
  m_sendBtn->setStyleSheet(
      "QPushButton { background: #d900ff; color: #ffffff; border-radius: 16px; "
      "font-weight: bold; } QPushButton:hover { background: #ff00ff; }");
  connect(m_sendBtn, &QPushButton::clicked, this, &AIChatPanel::onSendClicked);

  inputLayout->addWidget(m_inputField);
  inputLayout->addWidget(m_sendBtn);
  layout->addWidget(inputFrame);
}

void AIChatPanel::updateModelSelector() {}

void AIChatPanel::setProject(const QString &projectName) {
  if (!m_currentProject.isEmpty()) {
    QJsonArray items;
    for (int i = 1; i < m_messages.size(); ++i)
      items.append(m_messages[i]);
    Storage::saveContext(m_currentProject, items);
  }
  m_currentProject = projectName;
  m_taskCounter = 0;
  QJsonArray saved = Storage::loadContext(projectName);
  m_messages = QJsonArray();
  QJsonObject sys;
  sys["role"] = "system";
  sys["content"] = SYSTEM_PROMPT;
  m_messages.append(sys);
  for (const auto &m : saved)
    m_messages.append(m);
  clearChatUI();
  for (int i = 1; i < m_messages.size(); ++i) {
    QJsonObject m = m_messages[i].toObject();
    addMessageUI(m["content"].toString(), m["role"].toString() == "user");
  }
}

void AIChatPanel::setTasksInfo(const QString &info) {
  addMessageUI(info, false);
}
void AIChatPanel::clearChatUI() {
  while (m_messagesLayout->count()) {
    auto *i = m_messagesLayout->takeAt(0);
    if (i->widget())
      i->widget()->deleteLater();
    delete i;
  }
}
void AIChatPanel::onClearChat() {
  clearChatUI();
  m_messages = QJsonArray();
  QJsonObject sys;
  sys["role"] = "system";
  sys["content"] = SYSTEM_PROMPT;
  m_messages.append(sys);
}
void AIChatPanel::addMessageUI(const QString &text, bool isUser) {
  m_messagesLayout->addWidget(new ChatMessage(text, isUser, m_messagesWidget));
  QTimer::singleShot(50, this, &AIChatPanel::scrollToBottom);
}
void AIChatPanel::scrollToBottom() {
  m_scrollArea->verticalScrollBar()->setValue(
      m_scrollArea->verticalScrollBar()->maximum());
}
void AIChatPanel::onSendClicked() { sendMessage(); }

void AIChatPanel::sendMessage() {
  QString text = m_inputField->text().trimmed();
  if (text.isEmpty() || m_apiKey.isEmpty())
    return;
  addMessageUI(text, true);
  QJsonObject msg;
  msg["role"] = "user";
  msg["content"] = text;
  m_messages.append(msg);
  m_inputField->clear();
  m_inputField->setEnabled(false);
  m_sendBtn->setEnabled(false);
  QNetworkRequest req{QUrl(OPENROUTER_API_URL)};
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  req.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());
  QJsonObject data;
  data["model"] = m_currentModel;
  data["messages"] = m_messages;
  m_networkManager->post(req, QJsonDocument(data).toJson());
}

void AIChatPanel::onNetworkReply(QNetworkReply *reply) {
  m_inputField->setEnabled(true);
  m_sendBtn->setEnabled(true);
  if (reply->error() == QNetworkReply::NoError) {
    QString content = QJsonDocument::fromJson(reply->readAll())
                          .object()["choices"]
                          .toArray()[0]
                          .toObject()["message"]
                          .toObject()["content"]
                          .toString();
    QJsonObject msg;
    msg["role"] = "assistant";
    msg["content"] = content;
    m_messages.append(msg);
    processAIResponse(content);
  }
  reply->deleteLater();
}

void AIChatPanel::processAIResponse(const QString &content) {
  auto objects = extractAllJson(content);
  QStringList results;
  for (const auto &obj : objects) {
    if (obj.contains("actions")) {
      for (const auto &a : obj["actions"].toArray())
        results.append(executeAction(a.toObject()));
    } else
      results.append(executeAction(obj));
  }
  addMessageUI(results.isEmpty() ? cleanJsonFromText(content)
                                 : "✓ " + results.join("\n✓ "),
               false);
}

QString AIChatPanel::formatAIMessage(const QString &content) { return content; }
QString AIChatPanel::cleanJsonFromText(const QString &text) {
  QString c = text;
  c.replace(QRegularExpression(R"(\{[^{}]*\})"), "");
  return c.simplified().isEmpty() ? "Готово" : c;
}

QList<QJsonObject> AIChatPanel::extractAllJson(const QString &text) {
  QList<QJsonObject> res;
  for (int i = 0; i < text.length(); ++i) {
    if (text[i] == '{') {
      int d = 0, s = i;
      for (int j = i; j < text.length(); ++j) {
        if (text[j] == '{')
          d++;
        else if (text[j] == '}') {
          d--;
          if (d == 0) {
            auto doc = QJsonDocument::fromJson(text.mid(s, j - s + 1).toUtf8());
            if (doc.isObject())
              res.append(doc.object());
            i = j;
            break;
          }
        }
      }
    }
  }
  return res;
}

QString AIChatPanel::executeAction(const QJsonObject &data) {
  QString a = data["action"].toString();
  if (a == "create_task") {
    auto pos = getTaskPosition();
    emit taskCreated(data["title"].toString(), data["description"].toString(),
                     data["status"].toString("todo"), pos.first, pos.second);
    return QString("✓ %1").arg(data["title"].toString());
  } else if (a == "create_tasks_chain") {
    QJsonArray tasks = data["tasks"].toArray();
    bool doConnect = data["connect"].toBool(true);
    int startIdx = m_taskCounter;
    for (const auto &tv : tasks) {
      QJsonObject t = tv.toObject();
      auto pos = getTaskPosition();
      emit taskCreated(t["title"].toString(), t["description"].toString(),
                       t["status"].toString("todo"), pos.first, pos.second);
    }
    if (doConnect && tasks.size() > 1) {
      for (int i = 0; i < tasks.size() - 1; ++i) {
        emit tasksConnect(startIdx + i, startIdx + i + 1);
      }
    }
    return QString("✓ Создано %1 задач").arg(tasks.size());
  } else if (a == "connect") {
    emit tasksConnect(data["from"].toInt() - 1, data["to"].toInt() - 1);
    return QString("✓ Соединено %1 → %2")
        .arg(data["from"].toInt())
        .arg(data["to"].toInt());
  } else if (a == "connect_many") {
    QJsonArray conns = data["connections"].toArray();
    for (const auto &c : conns) {
      QJsonArray pair = c.toArray();
      if (pair.size() >= 2) {
        emit tasksConnect(pair[0].toInt() - 1, pair[1].toInt() - 1);
      }
    }
    return QString("✓ Соединено %1 связей").arg(conns.size());
  } else if (a == "disconnect") {
    emit disconnectTasks(data["from"].toInt() - 1, data["to"].toInt() - 1);
    return QString("✓ Разъединено");
  } else if (a == "set_status") {
    emit taskUpdateStatus(data["task"].toInt() - 1, data["status"].toString());
    return QString("✓ Статус → %1").arg(data["status"].toString());
  } else if (a == "set_many_status") {
    QString status = data["status"].toString();
    for (const auto &t : data["tasks"].toArray()) {
      emit taskUpdateStatus(t.toInt() - 1, status);
    }
    return QString("✓ Статус %1 задач → %2")
        .arg(data["tasks"].toArray().size())
        .arg(status);
  } else if (a == "rename") {
    emit taskRename(data["task"].toInt() - 1, data["title"].toString());
    return QString("✓ Переименовано");
  } else if (a == "set_description") {
    emit taskUpdateDesc(data["task"].toInt() - 1,
                        data["description"].toString());
    return QString("✓ Описание обновлено");
  } else if (a == "delete") {
    emit taskDelete(data["task"].toInt() - 1);
    return QString("✓ Удалено");
  } else if (a == "delete_many") {
    QList<int> ids;
    for (const auto &t : data["tasks"].toArray())
      ids.append(t.toInt() - 1);
    emit tasksDeleteMany(ids);
    return QString("✓ Удалено %1").arg(ids.size());
  } else if (a == "clear_all") {
    emit clearAllTasks();
    m_taskCounter = 0;
    return "✓ Очищено";
  } else if (a == "get_tasks") {
    emit requestTasks();
    return "✓ Список задач";
  } else if (a.startsWith("arrange_")) {
    emit arrangeTasks(a.mid(8));
    return QString("✓ Расставлено: %1").arg(a.mid(8));
  } else if (!a.isEmpty()) {
    return QString("⚠ Неизвестное действие: %1").arg(a);
  }
  return "";
}

QString AIChatPanel::describeAction(const QJsonObject &data) { return ""; }
QPair<int, int> AIChatPanel::getTaskPosition() {
  int c = m_taskCounter % 3, r = m_taskCounter / 3;
  m_taskCounter++;
  return {50 + c * 250, 50 + r * 180};
}
void AIChatPanel::onApiKeySetup() {
  bool ok;
  QString k = QInputDialog::getText(this, "API", "Key:", QLineEdit::Normal,
                                    m_apiKey, &ok);
  if (ok) {
    m_apiKey = k.trimmed();
    Storage::saveApiKey(m_apiKey);
  }
}
void AIChatPanel::onQuickAction(const QString &text) {}
void AIChatPanel::onAddModel() {}
void AIChatPanel::onModelChanged(int index) {}

} // namespace DevPlanner
