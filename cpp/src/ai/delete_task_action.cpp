#include "delete_task_action.hpp"

namespace DevPlanner {

DeleteTaskAction::DeleteTaskAction(Callback callback) : m_callback(callback) {}

QString DeleteTaskAction::execute(const QJsonObject &params) {
  int task = params["task"].toInt() - 1;
  m_callback(task);
  return QString("Задача %1 удалена").arg(params["task"].toInt());
}

QJsonObject DeleteTaskAction::toSchema() const {
  QJsonObject schema;
  schema["action"] = "delete";
  schema["task"] = "int - номер задачи для удаления";
  return schema;
}

} // namespace DevPlanner
