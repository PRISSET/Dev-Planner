#include "ai_action_registry.hpp"
#include "actions/arrange_action.hpp"
#include "actions/clear_all_action.hpp"
#include "actions/connect_action.hpp"
#include "actions/connect_many_action.hpp"
#include "actions/create_chain_action.hpp"
#include "actions/create_task_action.hpp"
#include "actions/delete_action.hpp"
#include "actions/delete_many_action.hpp"
#include "actions/disconnect_action.hpp"
#include "actions/rename_action.hpp"
#include "actions/set_description_action.hpp"
#include "actions/set_many_status_action.hpp"
#include "actions/set_status_action.hpp"

namespace DevPlanner {

AIActionRegistry &AIActionRegistry::instance() {
  static AIActionRegistry registry;
  return registry;
}

void AIActionRegistry::registerAction(std::unique_ptr<AIAction> action) {
  std::string key = action->name().toStdString();
  m_actions[key] = std::move(action);
}

QString AIActionRegistry::execute(const QString &actionName,
                                  const QJsonObject &data, ActionContext &ctx) {
  std::string key = actionName.toStdString();
  auto it = m_actions.find(key);
  if (it != m_actions.end()) {
    return it->second->execute(data, ctx);
  }

  if (actionName.startsWith("arrange_")) {
    auto arrangeIt = m_actions.find("arrange");
    if (arrangeIt != m_actions.end()) {
      QJsonObject modData = data;
      modData["type"] = actionName.mid(8);
      return arrangeIt->second->execute(modData, ctx);
    }
  }

  return QString("⚠ Неизвестное действие: %1").arg(actionName);
}

bool AIActionRegistry::hasAction(const QString &name) const {
  return m_actions.count(name.toStdString()) > 0 || name.startsWith("arrange_");
}

void registerAllActions() {
  auto &reg = AIActionRegistry::instance();
  reg.registerAction(std::make_unique<CreateTaskAction>());
  reg.registerAction(std::make_unique<CreateChainAction>());
  reg.registerAction(std::make_unique<ConnectAction>());
  reg.registerAction(std::make_unique<ConnectManyAction>());
  reg.registerAction(std::make_unique<DisconnectAction>());
  reg.registerAction(std::make_unique<SetStatusAction>());
  reg.registerAction(std::make_unique<SetManyStatusAction>());
  reg.registerAction(std::make_unique<RenameAction>());
  reg.registerAction(std::make_unique<SetDescriptionAction>());
  reg.registerAction(std::make_unique<DeleteAction>());
  reg.registerAction(std::make_unique<DeleteManyAction>());
  reg.registerAction(std::make_unique<ClearAllAction>());
  reg.registerAction(std::make_unique<ArrangeAction>());
}

} // namespace DevPlanner
