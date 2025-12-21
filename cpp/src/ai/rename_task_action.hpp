#ifndef RENAME_TASK_ACTION_HPP
#define RENAME_TASK_ACTION_HPP

#include "ai_action.hpp"
#include <functional>

namespace DevPlanner {

class RenameTaskAction : public AIAction {
public:
  using Callback = std::function<void(int, const QString &)>;

  explicit RenameTaskAction(Callback callback);

  QString name() const override { return "rename"; }
  QString description() const override { return "Переименовывает задачу"; }
  QString execute(const QJsonObject &params) override;
  QJsonObject toSchema() const override;

private:
  Callback m_callback;
};

} // namespace DevPlanner

#endif
