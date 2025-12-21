#include "live_background.hpp"
#include <QPainter>
#include <QRadialGradient>
#include <QRandomGenerator>
#include <QtMath>

namespace DevPlanner {

LiveBackground::LiveBackground(QWidget *parent) : QWidget(parent), m_time(0.0) {
  setAttribute(Qt::WA_TransparentForMouseEvents);

  QVector<QColor> colors = {
      QColor("#4c1d95"), // Deep Violet
      QColor("#be185d"), // Pink
      QColor("#0e7490"), // Cyan
      QColor("#1e3a8a"), // Deep Blue
      QColor("#701a75"), // Fuchsia
      QColor("#1d4ed8")  // Brilliant Blue
  };

  // Create more blobs for a richer background
  for (int i = 0; i < 12; ++i) {
    NebulaBlob b;
    b.pos = QPointF(QRandomGenerator::global()->bounded(1920),
                    QRandomGenerator::global()->bounded(1080));
    b.velocity =
        QPointF(QRandomGenerator::global()->generateDouble() * 0.8 - 0.4,
                QRandomGenerator::global()->generateDouble() * 0.8 - 0.4);
    b.baseRadius = QRandomGenerator::global()->bounded(300, 700);
    b.currentRadius = b.baseRadius;
    b.color = colors[i % colors.size()];
    b.pulsePhase = QRandomGenerator::global()->generateDouble() * M_PI * 2.0;
    b.pulseSpeed = QRandomGenerator::global()->generateDouble() * 0.02 + 0.01;
    m_blobs.append(b);
  }

  m_timer = new QTimer(this);
  connect(m_timer, &QTimer::timeout, this, &LiveBackground::updateAnimation);
  m_timer->start(16);
}

void LiveBackground::pause() { m_paused = true; }

void LiveBackground::resume() { m_paused = false; }

void LiveBackground::updateAnimation() {
  if (m_paused)
    return;
  m_time += 0.01;

  for (auto &b : m_blobs) {
    // Move
    b.pos += b.velocity;

    // Wrap around screen with margin
    qreal margin = b.baseRadius;
    if (b.pos.x() < -margin)
      b.pos.setX(width() + margin);
    if (b.pos.x() > width() + margin)
      b.pos.setX(-margin);
    if (b.pos.y() < -margin)
      b.pos.setY(height() + margin);
    if (b.pos.y() > height() + margin)
      b.pos.setY(-margin);

    // Breathe (Pulsate)
    b.pulsePhase += b.pulseSpeed;
    qreal pulse = (qSin(b.pulsePhase) + 1.0) * 0.5;       // 0.0 to 1.0
    b.currentRadius = b.baseRadius * (0.8 + pulse * 0.4); // 80% to 120% size
  }

  update();
}

void LiveBackground::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);

  // Deep Black Space
  p.fillRect(rect(), QColor("#020204"));

  // Draw Breathing Nebula Blobs
  for (const auto &b : m_blobs) {
    QRadialGradient g(b.pos, b.currentRadius);

    // Modulate alpha based on pulse for extra "breathing" effect
    qreal pulse = (qSin(b.pulsePhase) + 1.0) * 0.5;
    int alpha =
        static_cast<int>(15 + pulse * 15); // Pulsates between 15 and 30 alpha

    QColor c = b.color;
    c.setAlpha(alpha);

    g.setColorAt(0, c);
    g.setColorAt(1, Qt::transparent);

    p.setBrush(g);
    p.setPen(Qt::NoPen);
    p.drawEllipse(b.pos, b.currentRadius, b.currentRadius);
  }

  // Very subtle static-ish grid to bind it together
  p.setPen(QPen(QColor(255, 255, 255, 5), 1));
  int gridSize = 100;
  for (int x = 0; x < width(); x += gridSize)
    p.drawLine(x, 0, x, height());
  for (int y = 0; y < height(); y += gridSize)
    p.drawLine(0, y, width(), y);
}

void LiveBackground::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
}

} // namespace DevPlanner
