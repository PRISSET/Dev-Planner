#include "glassmorphism_widget.hpp"
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>

namespace DevPlanner {

GlassmorphismWidget::GlassmorphismWidget(QWidget *parent) : QWidget(parent) {}

void GlassmorphismWidget::setBlurRadius(qreal radius) {
  m_blurRadius = radius;
  update();
}

void GlassmorphismWidget::setBackgroundColor(const QColor &color) {
  m_bgColor = color;
  update();
}

void GlassmorphismWidget::setBackgroundOpacity(int opacity) {
  m_opacity = opacity;
  update();
}

void GlassmorphismWidget::setBorderColor(const QColor &color) {
  m_borderColor = color;
  update();
}

void GlassmorphismWidget::setBorderRadius(qreal radius) {
  m_borderRadius = radius;
  update();
}

void GlassmorphismWidget::setBorderWidth(qreal width) {
  m_borderWidth = width;
  update();
}

void GlassmorphismWidget::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event)

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  QPainterPath path;
  path.addRoundedRect(rect().adjusted(1, 1, -1, -1), m_borderRadius,
                      m_borderRadius);

  QLinearGradient bgGradient(0, 0, 0, height());
  bgGradient.setColorAt(0, QColor(10, 10, 20, 160));
  bgGradient.setColorAt(1, QColor(5, 5, 10, 180));
  painter.fillPath(path, QBrush(bgGradient));

  QLinearGradient borderGradient(0, 0, width(), height());
  borderGradient.setColorAt(0, QColor(255, 255, 255, 80));
  borderGradient.setColorAt(0.5, QColor(255, 255, 255, 20));
  borderGradient.setColorAt(1, QColor(255, 255, 255, 40));

  QPen pen(QBrush(borderGradient), m_borderWidth);
  painter.setPen(pen);
  painter.drawPath(path);
}

} // namespace DevPlanner
