#include "set_status_action.hpp"

namespace DevPlanner {

SetStatusAction::SetStatusAction(Callback callback) : m_callback(callback) {}

QString SetStatusAction::execute(const QJsonObject &params) {
  int task = params["task"].toInt() - 1;
  QString status = params["status"].toString();
  m_callback(task, status);
  return QString("Статус задачи %1 → %2")
      .arg(params["task"].toInt())
      .arg(status);
}

QJsonObject SetStatusAction::toSchema() const {
  QJsonObject schema;
  schema["action"] = "set_status";
  schema["task"] = "int - номер задачи";
  schema["status"] = "string - статус: todo (красный), progress (жёлтый), done "
                     "(зелёный), none (серый)";
  return schema;
}

} // namespace DevPlanner
