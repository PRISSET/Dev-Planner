#ifndef SET_STATUS_ACTION_HPP
#define SET_STATUS_ACTION_HPP

#include "../ai_action.hpp"

namespace DevPlanner {

class SetStatusAction : public AIAction {
public:
  QString name() const override { return "set_status"; }

  QString execute(const QJsonObject &data, ActionContext &ctx) override {
    int taskIdx = data["task"].toInt() - 1;
    QString status = data["status"].toString();

    if (taskIdx < 0) {
      return "⚠ Неверный номер задачи";
    }

    if (taskIdx >= ctx.getTaskCount()) {
      return QString("⚠ Задача %1 не существует").arg(taskIdx + 1);
    }

    if (status.isEmpty()) {
      return "⚠ Не указан статус";
    }

    ctx.setStatus(taskIdx, status);

    QString statusName;
    if (status == "done")
      statusName = "готово 🟢";
    else if (status == "progress")
      statusName = "в работе 🟡";
    else if (status == "todo")
      statusName = "к выполнению 🔴";
    else
      statusName = status;

    return QString("✓ Задача %1 → %2").arg(taskIdx + 1).arg(statusName);
  }
};

} // namespace DevPlanner

#endif
