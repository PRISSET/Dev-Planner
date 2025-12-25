#ifndef CREATE_CHAIN_ACTION_HPP
#define CREATE_CHAIN_ACTION_HPP

#include "../ai_action.hpp"
#include <QJsonArray>

namespace DevPlanner {

class CreateChainAction : public AIAction {
public:
  QString name() const override { return "create_tasks_chain"; }

  QString execute(const QJsonObject &data, ActionContext &ctx) override {
    QJsonArray tasks = data["tasks"].toArray();
    bool doConnect = data["connect"].toBool(true);

    if (tasks.isEmpty()) {
      return "⚠ Нет задач для создания";
    }

    int startIdx = ctx.getTaskCount();
    int &counter = ctx.getPositionCounter();

    for (const auto &taskVal : tasks) {
      QJsonObject task = taskVal.toObject();
      QString title = task["title"].toString("Задача");
      QString desc = task["description"].toString();
      QString status = task["status"].toString("todo");

      int col = counter % 3;
      int row = counter / 3;
      int x = 50 + col * 250;
      int y = 50 + row * 180;
      counter++;

      ctx.createTask(title, desc, status, x, y);
    }

    if (doConnect && tasks.size() > 1) {
      for (int i = 0; i < tasks.size() - 1; ++i) {
        ctx.connectTasks(startIdx + i, startIdx + i + 1);
      }
    }

    return QString("✓ Создано %1 задач").arg(tasks.size());
  }
};

} // namespace DevPlanner

#endif
