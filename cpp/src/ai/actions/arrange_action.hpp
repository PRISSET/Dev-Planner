#ifndef ARRANGE_ACTION_HPP
#define ARRANGE_ACTION_HPP

#include "../ai_action.hpp"

namespace DevPlanner {

class ArrangeAction : public AIAction {
public:
  QString name() const override { return "arrange"; }

  QString execute(const QJsonObject &data, ActionContext &ctx) override {
    QString type = data["type"].toString("grid");
    ctx.arrange(type);

    QString typeName;
    if (type == "grid")
      typeName = "сеткой";
    else if (type == "tree")
      typeName = "деревом";
    else if (type == "horizontal")
      typeName = "горизонтально";
    else if (type == "vertical")
      typeName = "вертикально";
    else
      typeName = type;

    return QString("✓ Расположено %1").arg(typeName);
  }
};

} // namespace DevPlanner

#endif
