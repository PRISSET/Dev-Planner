#ifndef CREATE_TASK_ACTION_HPP
#define CREATE_TASK_ACTION_HPP

#include "../ai_action.hpp"

namespace DevPlanner {

class CreateTaskAction : public AIAction {
public:
  QString name() const override { return "create_task"; }

  QString execute(const QJsonObject &data, ActionContext &ctx) override {
    QString title = data["title"].toString("Новая задача");
    QString desc = data["description"].toString();
    QString status = data["status"].toString("todo");

    int &counter = ctx.getPositionCounter();
    int col = counter % 3;
    int row = counter / 3;
    int x = 50 + col * 250;
    int y = 50 + row * 180;
    counter++;

    ctx.createTask(title, desc, status, x, y);
    return QString("✓ %1").arg(title);
  }
};

} // namespace DevPlanner

#endif
