#include "modern_button.hpp"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

namespace DevPlanner {

ModernButton::ModernButton(const QString &t, const QColor &c, QWidget *p)
    : QPushButton(t, p), m_accentColor(c) {
  setCursor(Qt::PointingHandCursor);
  setFixedHeight(40);
  setFont(QFont("SF Pro Display", 11, QFont::Bold));
  m_glowAnimation = new QPropertyAnimation(this, "glowIntensity", this);
  m_glowAnimation->setDuration(200);
}

void ModernButton::setAccentColor(const QColor &c) {
  m_accentColor = c;
  update();
}
void ModernButton::setGlowIntensity(qreal i) {
  m_glowIntensity = i;
  update();
}
void ModernButton::animateGlow(qreal t) {
  m_glowAnimation->stop();
  m_glowAnimation->setStartValue(m_glowIntensity);
  m_glowAnimation->setEndValue(t);
  m_glowAnimation->start();
}
void ModernButton::enterEvent(QEnterEvent *e) {
  QPushButton::enterEvent(e);
  animateGlow(1.0);
}
void ModernButton::leaveEvent(QEvent *e) {
  QPushButton::leaveEvent(e);
  animateGlow(0.0);
}
void ModernButton::mousePressEvent(QMouseEvent *e) {
  QPushButton::mousePressEvent(e);
  m_isPressed = true;
  update();
}
void ModernButton::mouseReleaseEvent(QMouseEvent *e) {
  QPushButton::mouseReleaseEvent(e);
  m_isPressed = false;
  update();
}

void ModernButton::paintEvent(QPaintEvent *e) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  QRectF r(1, 1, width() - 2, height() - 2);
  QPainterPath path;
  path.addRoundedRect(r, 12, 12);
  QColor bg = m_accentColor;
  bg.setAlpha(m_isPressed ? 100 : (m_glowIntensity * 40 + 20));
  p.fillPath(path, bg);
  QPen pen(m_accentColor, 1.5);
  if (m_glowIntensity > 0) {
    for (int i = 3; i >= 1; --i) {
      QColor gc = m_accentColor;
      gc.setAlpha(m_glowIntensity * 40 / i);
      p.setPen(QPen(gc, i * 3));
      p.drawPath(path);
    }
  }
  p.setPen(pen);
  p.drawPath(path);
  p.setPen(Qt::white);
  p.setFont(font());
  p.drawText(r, Qt::AlignCenter, text());
}

} // namespace DevPlanner
