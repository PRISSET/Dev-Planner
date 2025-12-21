#ifndef AI_ACTION_MANAGER_HPP
#define AI_ACTION_MANAGER_HPP

#include "ai_action.hpp"
#include <QJsonArray>
#include <map>
#include <memory>

namespace DevPlanner {

class AIActionManager {
public:
  AIActionManager() = default;

  void registerAction(std::unique_ptr<AIAction> action);
  QString executeAction(const QJsonObject &data);
  QString generatePrompt() const;
  QJsonArray getAllSchemas() const;

private:
  std::map<QString, std::unique_ptr<AIAction>> m_actions;
};

} // namespace DevPlanner

#endif
