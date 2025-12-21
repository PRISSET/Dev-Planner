#ifndef DELETE_TASK_ACTION_HPP
#define DELETE_TASK_ACTION_HPP

#include "ai_action.hpp"
#include <functional>

namespace DevPlanner {

class DeleteTaskAction : public AIAction {
public:
  using Callback = std::function<void(int)>;

  explicit DeleteTaskAction(Callback callback);

  QString name() const override { return "delete"; }
  QString description() const override { return "Удаляет задачу по номеру"; }
  QString execute(const QJsonObject &params) override;
  QJsonObject toSchema() const override;

private:
  Callback m_callback;
};

} // namespace DevPlanner

#endif
