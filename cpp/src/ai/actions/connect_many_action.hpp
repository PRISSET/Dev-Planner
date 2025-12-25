#ifndef CONNECT_MANY_ACTION_HPP
#define CONNECT_MANY_ACTION_HPP

#include "../ai_action.hpp"
#include <QJsonArray>

namespace DevPlanner {

class ConnectManyAction : public AIAction {
public:
  QString name() const override { return "connect_many"; }

  QString execute(const QJsonObject &data, ActionContext &ctx) override {
    QJsonArray connections = data["connections"].toArray();

    if (connections.isEmpty()) {
      return "⚠ Нет соединений";
    }

    int count = 0;
    int taskCount = ctx.getTaskCount();

    for (const auto &connVal : connections) {
      QJsonArray pair = connVal.toArray();
      if (pair.size() >= 2) {
        int from = pair[0].toInt() - 1;
        int to = pair[1].toInt() - 1;

        if (from >= 0 && to >= 0 && from < taskCount && to < taskCount) {
          ctx.connectTasks(from, to);
          count++;
        }
      }
    }

    return QString("✓ Создано %1 соединений").arg(count);
  }
};

} // namespace DevPlanner

#endif
