#include "connect_tasks_action.hpp"

namespace DevPlanner {

ConnectTasksAction::ConnectTasksAction(Callback callback)
    : m_callback(callback) {}

QString ConnectTasksAction::execute(const QJsonObject &params) {
  int from = params["from"].toInt() - 1;
  int to = params["to"].toInt() - 1;
  m_callback(from, to);
  return QString("Соединено: %1 → %2")
      .arg(params["from"].toInt())
      .arg(params["to"].toInt());
}

QJsonObject ConnectTasksAction::toSchema() const {
  QJsonObject schema;
  schema["action"] = "connect";
  schema["from"] = "int - номер первой задачи";
  schema["to"] = "int - номер второй задачи";
  return schema;
}

} // namespace DevPlanner
