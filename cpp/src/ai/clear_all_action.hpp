#ifndef CLEAR_ALL_ACTION_HPP
#define CLEAR_ALL_ACTION_HPP

#include "ai_action.hpp"
#include <functional>

namespace DevPlanner {

class ClearAllAction : public AIAction {
public:
  using Callback = std::function<void()>;

  explicit ClearAllAction(Callback callback);

  QString name() const override { return "clear_all"; }
  QString description() const override {
    return "Удаляет все задачи с канваса";
  }
  QString execute(const QJsonObject &params) override;
  QJsonObject toSchema() const override;

private:
  Callback m_callback;
};

} // namespace DevPlanner

#endif
