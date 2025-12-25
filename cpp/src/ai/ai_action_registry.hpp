#ifndef AI_ACTION_REGISTRY_HPP
#define AI_ACTION_REGISTRY_HPP

#include "ai_action.hpp"
#include <memory>
#include <string>
#include <unordered_map>

namespace DevPlanner {

class AIActionRegistry {
public:
  static AIActionRegistry &instance();

  void registerAction(std::unique_ptr<AIAction> action);
  QString execute(const QString &actionName, const QJsonObject &data,
                  ActionContext &ctx);
  bool hasAction(const QString &name) const;

private:
  AIActionRegistry() = default;
  std::unordered_map<std::string, std::unique_ptr<AIAction>> m_actions;
};

void registerAllActions();

} // namespace DevPlanner

#endif
