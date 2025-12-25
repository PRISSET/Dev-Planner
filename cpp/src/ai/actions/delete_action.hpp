#ifndef DELETE_ACTION_HPP
#define DELETE_ACTION_HPP

#include "../ai_action.hpp"

namespace DevPlanner {

class DeleteAction : public AIAction {
public:
  QString name() const override { return "delete"; }

  QString execute(const QJsonObject &data, ActionContext &ctx) override {
    int taskIdx = data["task"].toInt() - 1;

    if (taskIdx < 0 || taskIdx >= ctx.getTaskCount()) {
      return QString("⚠ Задача %1 не существует").arg(taskIdx + 1);
    }

    ctx.deleteTask(taskIdx);
    return QString("✓ Задача %1 удалена").arg(taskIdx + 1);
  }
};

} // namespace DevPlanner

#endif
