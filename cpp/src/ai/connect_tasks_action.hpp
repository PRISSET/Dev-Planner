#ifndef CONNECT_TASKS_ACTION_HPP
#define CONNECT_TASKS_ACTION_HPP

#include "ai_action.hpp"
#include <functional>

namespace DevPlanner {

class ConnectTasksAction : public AIAction {
public:
  using Callback = std::function<void(int, int)>;

  explicit ConnectTasksAction(Callback callback);

  QString name() const override { return "connect"; }
  QString description() const override { return "Соединяет две задачи линией"; }
  QString execute(const QJsonObject &params) override;
  QJsonObject toSchema() const override;

private:
  Callback m_callback;
};

} // namespace DevPlanner

#endif
