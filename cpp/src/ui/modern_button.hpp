#ifndef MODERN_BUTTON_HPP
#define MODERN_BUTTON_HPP

#include <QColor>
#include <QPropertyAnimation>
#include <QPushButton>

namespace DevPlanner {

class ModernButton : public QPushButton {
  Q_OBJECT
  Q_PROPERTY(qreal glowIntensity READ glowIntensity WRITE setGlowIntensity)

public:
  explicit ModernButton(const QString &text,
                        const QColor &accentColor = QColor("#d900ff"),
                        QWidget *parent = nullptr);

  void setAccentColor(const QColor &color);
  QColor accentColor() const { return m_accentColor; }

  void setGlowIntensity(qreal intensity);
  qreal glowIntensity() const { return m_glowIntensity; }

protected:
  void paintEvent(QPaintEvent *event) override;
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  QColor m_accentColor;
  qreal m_glowIntensity = 0.0;
  bool m_isPressed = false;
  QPropertyAnimation *m_glowAnimation = nullptr;

  void animateGlow(qreal targetIntensity);
};

} // namespace DevPlanner

#endif // MODERN_BUTTON_HPP
