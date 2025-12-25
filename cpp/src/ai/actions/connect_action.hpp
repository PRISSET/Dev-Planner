#ifndef CONNECT_ACTION_HPP
#define CONNECT_ACTION_HPP

#include "../ai_action.hpp"

namespace DevPlanner {

class ConnectAction : public AIAction {
public:
  QString name() const override { return "connect"; }

  QString execute(const QJsonObject &data, ActionContext &ctx) override {
    int from = data["from"].toInt() - 1;
    int to = data["to"].toInt() - 1;

    if (from < 0 || to < 0) {
      return "⚠ Неверные номера задач";
    }

    if (from >= ctx.getTaskCount() || to >= ctx.getTaskCount()) {
      return QString("⚠ Задачи %1 или %2 не существуют")
          .arg(from + 1)
          .arg(to + 1);
    }

    ctx.connectTasks(from, to);
    return QString("✓ Соединено %1 → %2").arg(from + 1).arg(to + 1);
  }
};

} // namespace DevPlanner

#endif
