#ifndef SET_DESCRIPTION_ACTION_HPP
#define SET_DESCRIPTION_ACTION_HPP

#include "../ai_action.hpp"

namespace DevPlanner {

class SetDescriptionAction : public AIAction {
public:
  QString name() const override { return "set_description"; }

  QString execute(const QJsonObject &data, ActionContext &ctx) override {
    int taskIdx = data["task"].toInt() - 1;
    QString desc = data["description"].toString();

    if (taskIdx < 0 || taskIdx >= ctx.getTaskCount()) {
      return QString("⚠ Задача %1 не существует").arg(taskIdx + 1);
    }

    ctx.setDescription(taskIdx, desc);
    return QString("✓ Описание задачи %1 обновлено").arg(taskIdx + 1);
  }
};

} // namespace DevPlanner

#endif
