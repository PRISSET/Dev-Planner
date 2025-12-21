#include "rename_task_action.hpp"

namespace DevPlanner {

RenameTaskAction::RenameTaskAction(Callback callback) : m_callback(callback) {}

QString RenameTaskAction::execute(const QJsonObject &params) {
  int task = params["task"].toInt() - 1;
  QString title = params["title"].toString();
  m_callback(task, title);
  return QString("Задача %1 переименована в: %2")
      .arg(params["task"].toInt())
      .arg(title);
}

QJsonObject RenameTaskAction::toSchema() const {
  QJsonObject schema;
  schema["action"] = "rename";
  schema["task"] = "int - номер задачи";
  schema["title"] = "string - новое название";
  return schema;
}

} // namespace DevPlanner
