#include "ai_chat_panel.hpp"
#include "../ai/ai_action_registry.hpp"
#include "core/config.hpp"
#include "core/storage.hpp"
#include <QFrame>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSslError>
#include <QTimer>
#include <QVBoxLayout>

namespace DevPlanner {

const QString AIChatPanel::OPENROUTER_API_URL =
    "https://openrouter.ai/api/v1/chat/completions";

const QString AIChatPanel::SYSTEM_PROMPT =
    R"(Ты — AI-ассистент Dev Planner. Помогай управлять задачами.

ПРАВИЛА:
1. Для ДЕЙСТВИЙ над задачами ВСЕГДА возвращай ТОЛЬКО чистый JSON без текста
2. Для ВОПРОСОВ и советов отвечай текстом

ДЕЙСТВИЯ (только JSON, без текста вокруг):
{"action": "create_task", "title": "...", "description": "...", "status": "todo"}
{"actions": [{"action": "create_task", "title": "...", "description": "...", "status": "todo"}, ...]}
{"action": "set_status", "task": 1, "status": "done"}
{"action": "set_many_status", "tasks": [1,2,3,4], "status": "done"}
{"action": "connect", "from": 1, "to": 2}
{"action": "connect_many", "connections": [[1,2],[2,3]]}
{"action": "rename", "task": 1, "title": "..."}
{"action": "delete", "task": 1}
{"action": "clear_all"}
{"action": "arrange_tree"}

СТАТУСЫ: todo, progress, done, none
Нумерация задач с 1.

ПРИМЕРЫ:
"сделай все задачи готовыми" → {"action": "set_many_status", "tasks": [1,2,3,4], "status": "done"}
"создай 3 задачи" → {"actions": [{"action": "create_task", "title": "Задача 1", "description": "", "status": "todo"}, ...]}
"что такое agile?" → Отвечай текстом)";

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

#ifdef Q_OS_WIN
  connect(m_networkManager, &QNetworkAccessManager::sslErrors, this,
          [](QNetworkReply *reply, const QList<QSslError> &errors) {
            reply->ignoreSslErrors();
          });
#endif

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

void AIChatPanel::setTasksInfo(const QString &info) { m_tasksContext = info; }
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

  QString fullContent = text;
  if (!m_tasksContext.isEmpty()) {
    fullContent =
        QString("ТЕКУЩИЕ ЗАДАЧИ:\n%1\n\nЗАПРОС: %2").arg(m_tasksContext, text);
  }

  QJsonObject msg;
  msg["role"] = "user";
  msg["content"] = fullContent;
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
    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    if (doc.isObject() && doc.object().contains("choices")) {
      QString content = doc.object()["choices"]
                            .toArray()[0]
                            .toObject()["message"]
                            .toObject()["content"]
                            .toString();
      QJsonObject msg;
      msg["role"] = "assistant";
      msg["content"] = content;
      m_messages.append(msg);
      processAIResponse(content);
    } else if (doc.object().contains("error")) {
      QString errMsg = doc.object()["error"].toObject()["message"].toString();
      addMessageUI("❌ API Error: " + errMsg, false);
    } else {
      addMessageUI("❌ Неверный ответ от API", false);
    }
  } else {
    QString errorStr = reply->errorString();
    addMessageUI("❌ Ошибка сети: " + errorStr, false);
  }
  reply->deleteLater();
}

void AIChatPanel::processAIResponse(const QString &content) {
  auto objects = extractAllJson(content);
  QStringList results;
  for (const auto &obj : objects) {
    if (obj.contains("actions")) {
      for (const auto &a : obj["actions"].toArray()) {
        QString r = executeAction(a.toObject());
        if (!r.isEmpty())
          results.append(r);
      }
    } else {
      QString r = executeAction(obj);
      if (!r.isEmpty())
        results.append(r);
    }
  }

  QString textPart = cleanJsonFromText(content);
  QString display;

  if (!textPart.isEmpty() && textPart != "Готово") {
    display = textPart;
    if (!results.isEmpty()) {
      display += "\n\n" + results.join("\n");
    }
  } else if (!results.isEmpty()) {
    display = results.join("\n");
  } else {
    display = content;
  }

  addMessageUI(display, false);
}

QString AIChatPanel::formatAIMessage(const QString &content) { return content; }
QString AIChatPanel::cleanJsonFromText(const QString &text) {
  QString result;
  int depth = 0;
  for (int i = 0; i < text.length(); ++i) {
    QChar c = text[i];
    if (c == '{') {
      depth++;
    } else if (c == '}') {
      depth--;
    } else if (depth == 0) {
      result += c;
    }
  }
  result = result.simplified();
  return result.isEmpty() ? "" : result;
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
  QString actionName = data["action"].toString();
  if (actionName.isEmpty())
    return "";

  ActionContext ctx;

  ctx.createTask = [this](const QString &title, const QString &desc,
                          const QString &status, int x, int y) {
    emit taskCreated(title, desc, status, x, y);
  };

  ctx.connectTasks = [this](int from, int to) { emit tasksConnect(from, to); };

  ctx.disconnectTasks = [this](int from, int to) {
    emit disconnectTasks(from, to);
  };

  ctx.setStatus = [this](int idx, const QString &status) {
    emit taskUpdateStatus(idx, status);
  };

  ctx.setTitle = [this](int idx, const QString &title) {
    emit taskRename(idx, title);
  };

  ctx.setDescription = [this](int idx, const QString &desc) {
    emit taskUpdateDesc(idx, desc);
  };

  ctx.deleteTask = [this](int idx) { emit taskDelete(idx); };

  ctx.clearAll = [this]() { emit clearAllTasks(); };

  ctx.arrange = [this](const QString &type) { emit arrangeTasks(type); };

  ctx.getTaskCount = [this]() -> int { return m_taskCounter; };

  ctx.getPositionCounter = [this]() -> int & { return m_taskCounter; };

  return AIActionRegistry::instance().execute(actionName, data, ctx);
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
