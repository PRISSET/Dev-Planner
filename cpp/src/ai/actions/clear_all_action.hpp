#ifndef CLEAR_ALL_ACTION_HPP
#define CLEAR_ALL_ACTION_HPP

#include "../ai_action.hpp"

namespace DevPlanner {

class ClearAllAction : public AIAction {
public:
  QString name() const override { return "clear_all"; }

  QString execute(const QJsonObject &data, ActionContext &ctx) override {
    Q_UNUSED(data);
    int count = ctx.getTaskCount();
    ctx.clearAll();
    ctx.getPositionCounter() = 0;
    return QString("✓ Удалено %1 задач, холст очищен").arg(count);
  }
};

} // namespace DevPlanner

#endif
