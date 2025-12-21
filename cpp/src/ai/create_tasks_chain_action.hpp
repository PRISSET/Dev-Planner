#ifndef CREATE_TASKS_CHAIN_ACTION_HPP
#define CREATE_TASKS_CHAIN_ACTION_HPP

#include "ai_action.hpp"
#include <functional>

namespace DevPlanner {

class CreateTasksChainAction : public AIAction {
public:
  using CreateCallback = std::function<void(const QString &, const QString &,
                                            const QString &, int, int)>;
  using ConnectCallback = std::function<void(int, int)>;

  CreateTasksChainAction(CreateCallback createCb, ConnectCallback connectCb,
                         std::function<std::pair<int, int>()> posGetter,
                         std::function<int()> counterGetter);

  QString name() const override { return "create_tasks_chain"; }
  QString description() const override {
    return "Создаёт несколько связанных задач";
  }
  QString execute(const QJsonObject &params) override;
  QJsonObject toSchema() const override;

private:
  CreateCallback m_createCb;
  ConnectCallback m_connectCb;
  std::function<std::pair<int, int>()> m_posGetter;
  std::function<int()> m_counterGetter;
};

} // namespace DevPlanner

#endif
