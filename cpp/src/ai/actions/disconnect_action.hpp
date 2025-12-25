#ifndef DISCONNECT_ACTION_HPP
#define DISCONNECT_ACTION_HPP

#include "../ai_action.hpp"

namespace DevPlanner {

class DisconnectAction : public AIAction {
public:
  QString name() const override { return "disconnect"; }

  QString execute(const QJsonObject &data, ActionContext &ctx) override {
    int from = data["from"].toInt() - 1;
    int to = data["to"].toInt() - 1;

    if (from < 0 || to < 0) {
      return "⚠ Неверные номера задач";
    }

    ctx.disconnectTasks(from, to);
    return QString("✓ Разъединено %1 ↛ %2").arg(from + 1).arg(to + 1);
  }
};

} // namespace DevPlanner

#endif
