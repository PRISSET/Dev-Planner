#include "create_tasks_chain_action.hpp"
#include <QJsonArray>

namespace DevPlanner {

CreateTasksChainAction::CreateTasksChainAction(
    CreateCallback createCb, ConnectCallback connectCb,
    std::function<std::pair<int, int>()> posGetter,
    std::function<int()> counterGetter)
    : m_createCb(createCb), m_connectCb(connectCb), m_posGetter(posGetter),
      m_counterGetter(counterGetter) {}

QString CreateTasksChainAction::execute(const QJsonObject &params) {
  QJsonArray tasks = params["tasks"].toArray();
  bool doConnect = params["connect"].toBool(true);
  int startIdx = m_counterGetter();

  for (const auto &tv : tasks) {
    QJsonObject t = tv.toObject();
    auto pos = m_posGetter();
    m_createCb(t["title"].toString(), t["description"].toString(),
               t["status"].toString("todo"), pos.first, pos.second);
  }

  if (doConnect && tasks.size() > 1) {
    for (int i = 0; i < tasks.size() - 1; ++i) {
      m_connectCb(startIdx + i, startIdx + i + 1);
    }
  }

  return QString("Создано %1 задач").arg(tasks.size());
}

QJsonObject CreateTasksChainAction::toSchema() const {
  QJsonObject schema;
  schema["action"] = "create_tasks_chain";
  schema["tasks"] = "[{title, description, status}, ...] - массив задач";
  schema["connect"] = "bool - соединять ли задачи (по умолч. true)";
  return schema;
}

} // namespace DevPlanner
