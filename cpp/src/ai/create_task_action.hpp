#ifndef CREATE_TASK_ACTION_HPP
#define CREATE_TASK_ACTION_HPP

#include "ai_action.hpp"
#include <functional>

namespace DevPlanner {

class CreateTaskAction : public AIAction {
public:
  using Callback = std::function<void(const QString &, const QString &,
                                      const QString &, int, int)>;

  explicit CreateTaskAction(Callback callback,
                            std::function<std::pair<int, int>()> posGetter);

  QString name() const override { return "create_task"; }
  QString description() const override {
    return "Создаёт новую задачу на канвасе";
  }
  QString execute(const QJsonObject &params) override;
  QJsonObject toSchema() const override;

private:
  Callback m_callback;
  std::function<std::pair<int, int>()> m_posGetter;
};

} // namespace DevPlanner

#endif
