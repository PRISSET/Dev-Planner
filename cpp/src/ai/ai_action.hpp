#ifndef AI_ACTION_HPP
#define AI_ACTION_HPP

#include <QJsonObject>
#include <QString>

namespace DevPlanner {

class AIAction {
public:
  virtual ~AIAction() = default;

  virtual QString name() const = 0;
  virtual QString description() const = 0;
  virtual QString execute(const QJsonObject &params) = 0;
  virtual QJsonObject toSchema() const = 0;
};

} // namespace DevPlanner

#endif
