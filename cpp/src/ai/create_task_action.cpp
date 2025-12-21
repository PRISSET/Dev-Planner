#include "create_task_action.hpp"

namespace DevPlanner {

CreateTaskAction::CreateTaskAction(
    Callback callback, std::function<std::pair<int, int>()> posGetter)
    : m_callback(callback), m_posGetter(posGetter) {}

QString CreateTaskAction::execute(const QJsonObject &params) {
  QString title = params["title"].toString();
  QString desc = params["description"].toString();
  QString status = params["status"].toString("todo");
  auto pos = m_posGetter();
  m_callback(title, desc, status, pos.first, pos.second);
  return QString("Создана задача: %1").arg(title);
}

QJsonObject CreateTaskAction::toSchema() const {
  QJsonObject schema;
  schema["action"] = "create_task";
  schema["title"] = "string - название задачи";
  schema["description"] = "string - описание задачи";
  schema["status"] = "string - статус: todo, progress, done, none";
  return schema;
}

} // namespace DevPlanner
