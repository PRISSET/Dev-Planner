#ifndef GLASSMORPHISM_WIDGET_HPP
#define GLASSMORPHISM_WIDGET_HPP

#include <QColor>
#include <QPainterPath>
#include <QWidget>

namespace DevPlanner {

class GlassmorphismWidget : public QWidget {
  Q_OBJECT

public:
  explicit GlassmorphismWidget(QWidget *parent = nullptr);

  void setBlurRadius(qreal radius);
  void setBackgroundColor(const QColor &color);
  void setBackgroundOpacity(int opacity);
  void setBorderColor(const QColor &color);
  void setBorderRadius(qreal radius);
  void setBorderWidth(qreal width);

  qreal blurRadius() const { return m_blurRadius; }
  QColor backgroundColor() const { return m_bgColor; }

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  qreal m_blurRadius = 20.0;
  QColor m_bgColor = QColor(20, 20, 30, 180);
  QColor m_borderColor = QColor(255, 255, 255, 40);
  qreal m_borderRadius = 15.0;
  qreal m_borderWidth = 1.0;
  int m_opacity = 180;
};

} // namespace DevPlanner

#endif // GLASSMORPHISM_WIDGET_HPP
