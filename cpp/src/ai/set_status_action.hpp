#ifndef SET_STATUS_ACTION_HPP
#define SET_STATUS_ACTION_HPP

#include "ai_action.hpp"
#include <functional>

namespace DevPlanner {

class SetStatusAction : public AIAction {
public:
  using Callback = std::function<void(int, const QString &)>;

  explicit SetStatusAction(Callback callback);

  QString name() const override { return "set_status"; }
  QString description() const override { return "Устанавливает статус задачи"; }
  QString execute(const QJsonObject &params) override;
  QJsonObject toSchema() const override;

private:
  Callback m_callback;
};

} // namespace DevPlanner

#endif
