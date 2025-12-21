#include "clear_all_action.hpp"

namespace DevPlanner {

ClearAllAction::ClearAllAction(Callback callback) : m_callback(callback) {}

QString ClearAllAction::execute(const QJsonObject &params) {
  Q_UNUSED(params);
  m_callback();
  return "Все задачи удалены";
}

QJsonObject ClearAllAction::toSchema() const {
  QJsonObject schema;
  schema["action"] = "clear_all";
  return schema;
}

} // namespace DevPlanner
