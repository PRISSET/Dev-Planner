#ifndef RENAME_ACTION_HPP
#define RENAME_ACTION_HPP

#include "../ai_action.hpp"

namespace DevPlanner {

class RenameAction : public AIAction {
public:
  QString name() const override { return "rename"; }

  QString execute(const QJsonObject &data, ActionContext &ctx) override {
    int taskIdx = data["task"].toInt() - 1;
    QString title = data["title"].toString();

    if (taskIdx < 0 || taskIdx >= ctx.getTaskCount()) {
      return QString("⚠ Задача %1 не существует").arg(taskIdx + 1);
    }

    if (title.isEmpty()) {
      return "⚠ Не указано новое название";
    }

    ctx.setTitle(taskIdx, title);
    return QString("✓ Задача %1 → \"%2\"").arg(taskIdx + 1).arg(title);
  }
};

} // namespace DevPlanner

#endif
