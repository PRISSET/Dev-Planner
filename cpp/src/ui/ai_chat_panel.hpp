#ifndef AI_CHAT_PANEL_HPP
#define AI_CHAT_PANEL_HPP

#include "glassmorphism_widget.hpp"
#include <QComboBox>
#include <QFrame>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPair>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace DevPlanner {

class ChatMessage : public QFrame {
  Q_OBJECT
public:
  explicit ChatMessage(const QString &text, bool isUser,
                       QWidget *parent = nullptr);
};

class AIChatPanel : public GlassmorphismWidget {
  Q_OBJECT

public:
  static const QString OPENROUTER_API_URL;
  static const QString SYSTEM_PROMPT;

  explicit AIChatPanel(QWidget *parent = nullptr);
  void setProject(const QString &projectName);
  void setTasksInfo(const QString &info);
  void setTaskCounter(int count) { m_taskCounter = count; }

signals:
  void taskCreated(const QString &title, const QString &description,
                   const QString &status, int x, int y);
  void tasksConnect(int fromIdx, int toIdx);
  void taskUpdateStatus(int taskIdx, const QString &status);
  void taskUpdateDesc(int taskIdx, const QString &description);
  void taskRename(int taskIdx, const QString &title);
  void taskDelete(int taskIdx);
  void tasksDeleteMany(const QList<int> &tasks);
  void clearAllTasks();
  void requestTasks();
  void arrangeTasks(const QString &type);
  void disconnectTasks(int fromIdx, int toIdx);

private slots:
  void onSendClicked();
  void onNetworkReply(QNetworkReply *reply);
  void onApiKeySetup();
  void onClearChat();
  void onQuickAction(const QString &text);
  void onAddModel();
  void onModelChanged(int index);
  void scrollToBottom();

private:
  void setupUI();
  void sendMessage();
  void processAIResponse(const QString &content);
  QString formatAIMessage(const QString &content);
  void addMessageUI(const QString &text, bool isUser);
  void clearChatUI();
  void updateModelSelector();

  QList<QJsonObject> extractAllJson(const QString &text);
  QString executeAction(const QJsonObject &data);
  QString describeAction(const QJsonObject &data);
  QString cleanJsonFromText(const QString &text);
  QPair<int, int> getTaskPosition();

  QNetworkAccessManager *m_networkManager;
  QString m_apiKey;
  QString m_currentModel;
  QStringList m_models;
  QJsonArray m_messages;
  QString m_currentProject;
  int m_taskCounter = 0;

  QVBoxLayout *m_messagesLayout;
  QWidget *m_messagesWidget;
  QScrollArea *m_scrollArea;
  QLineEdit *m_inputField;
  QPushButton *m_sendBtn;
  QComboBox *m_modelSelector;
  QLabel *m_statusLabel;
};

} // namespace DevPlanner

#endif
