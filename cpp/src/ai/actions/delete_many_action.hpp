#ifndef DELETE_MANY_ACTION_HPP
#define DELETE_MANY_ACTION_HPP

#include "../ai_action.hpp"
#include <QJsonArray>
#include <QList>
#include <algorithm>

namespace DevPlanner {

class DeleteManyAction : public AIAction {
public:
  QString name() const override { return "delete_many"; }

  QString execute(const QJsonObject &data, ActionContext &ctx) override {
    QJsonArray tasks = data["tasks"].toArray();

    if (tasks.isEmpty()) {
      return "⚠ Не указаны задачи для удаления";
    }

    QList<int> indices;
    int taskCount = ctx.getTaskCount();

    for (const auto &taskVal : tasks) {
      int idx = taskVal.toInt() - 1;
      if (idx >= 0 && idx < taskCount) {
        indices.append(idx);
      }
    }

    std::sort(indices.begin(), indices.end(), std::greater<int>());

    for (int idx : indices) {
      ctx.deleteTask(idx);
    }

    return QString("✓ Удалено %1 задач").arg(indices.size());
  }
};

} // namespace DevPlanner

#endif
