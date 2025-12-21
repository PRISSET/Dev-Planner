#include "ai_action_manager.hpp"
#include <QJsonDocument>

namespace DevPlanner {

void AIActionManager::registerAction(std::unique_ptr<AIAction> action) {
  QString name = action->name();
  m_actions[name] = std::move(action);
}

QString AIActionManager::executeAction(const QJsonObject &data) {
  QString actionName = data["action"].toString();
  auto it = m_actions.find(actionName);
  if (it != m_actions.end()) {
    return it->second->execute(data);
  }
  return QString("Неизвестное действие: %1").arg(actionName);
}

QString AIActionManager::generatePrompt() const {
  QStringList lines;
  lines << "Ты — AI-ассистент Dev Planner. Помогаешь планировать разработку.";
  lines << "";
  lines << "ВАЖНО: Для создания/изменения/удаления задач отвечай ТОЛЬКО "
           "JSON-командами.";
  lines << "Для обычных вопросов можешь отвечать текстом.";
  lines << "";
  lines << "ДОСТУПНЫЕ КОМАНДЫ:";

  for (const auto &pair : m_actions) {
    QJsonObject schema = pair.second->toSchema();
    lines << QString("• %1: %2").arg(pair.first, pair.second->description());
    lines << QString("  %1").arg(QString::fromUtf8(
        QJsonDocument(schema).toJson(QJsonDocument::Compact)));
  }

  lines << "";
  lines << "СТАТУСЫ ЗАДАЧ:";
  lines << "• todo (красный) - нужно сделать";
  lines << "• progress (жёлтый) - в процессе";
  lines << "• done (зелёный) - выполнено";
  lines << "• none (серый) - без статуса";
  lines << "";
  lines << "Для нескольких действий используй: {\"actions\": [...]}";

  return lines.join("\n");
}

QJsonArray AIActionManager::getAllSchemas() const {
  QJsonArray schemas;
  for (const auto &pair : m_actions) {
    schemas.append(pair.second->toSchema());
  }
  return schemas;
}

} // namespace DevPlanner
