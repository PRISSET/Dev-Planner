#ifndef AI_ACTION_HPP
#define AI_ACTION_HPP

#include <QJsonObject>
#include <QString>
#include <functional>

namespace DevPlanner {

struct ActionContext {
  std::function<void(const QString &, const QString &, const QString &, int,
                     int)>
      createTask;
  std::function<void(int, int)> connectTasks;
  std::function<void(int, int)> disconnectTasks;
  std::function<void(int, const QString &)> setStatus;
  std::function<void(int, const QString &)> setTitle;
  std::function<void(int, const QString &)> setDescription;
  std::function<void(int)> deleteTask;
  std::function<void()> clearAll;
  std::function<void(const QString &)> arrange;
  std::function<int()> getTaskCount;
  std::function<int &()> getPositionCounter;
};

class AIAction {
public:
  virtual ~AIAction() = default;
  virtual QString name() const = 0;
  virtual QString execute(const QJsonObject &data, ActionContext &ctx) = 0;
};

} // namespace DevPlanner

#endif
