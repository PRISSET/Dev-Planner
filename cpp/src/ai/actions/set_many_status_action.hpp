#ifndef SET_MANY_STATUS_ACTION_HPP
#define SET_MANY_STATUS_ACTION_HPP

#include "../ai_action.hpp"
#include <QJsonArray>

namespace DevPlanner {

class SetManyStatusAction : public AIAction {
public:
  QString name() const override { return "set_many_status"; }

  QString execute(const QJsonObject &data, ActionContext &ctx) override {
    QJsonArray tasks = data["tasks"].toArray();
    QString status = data["status"].toString();

    if (tasks.isEmpty()) {
      return "⚠ Не указаны задачи";
    }

    if (status.isEmpty()) {
      return "⚠ Не указан статус";
    }

    int taskCount = ctx.getTaskCount();
    int changed = 0;

    for (const auto &taskVal : tasks) {
      int idx = taskVal.toInt() - 1;
      if (idx >= 0 && idx < taskCount) {
        ctx.setStatus(idx, status);
        changed++;
      }
    }

    QString statusName;
    if (status == "done")
      statusName = "готово 🟢";
    else if (status == "progress")
      statusName = "в работе 🟡";
    else if (status == "todo")
      statusName = "к выполнению 🔴";
    else
      statusName = status;

    return QString("✓ %1 задач → %2").arg(changed).arg(statusName);
  }
};

} // namespace DevPlanner

#endif
