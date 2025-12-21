#include "live_background.hpp"
#include <QPainter>
#include <QRadialGradient>
#include <QRandomGenerator>
#include <QtMath>

namespace DevPlanner {

LiveBackground::LiveBackground(QWidget *parent) : QWidget(parent) {
  setAttribute(Qt::WA_TransparentForMouseEvents);
  lower();

  QList<QColor> colors = {QColor(138, 43, 226), QColor(255, 20, 147),
                          QColor(0, 191, 255), QColor(255, 69, 0)};

  auto *rng = QRandomGenerator::global();
  for (int i = 0; i < 6; ++i) {
    Blob b;
    b.pos = QPointF(rng->bounded(1920), rng->bounded(1080));
    b.velocity = QPointF((rng->bounded(100) - 50) / 200.0,
                         (rng->bounded(100) - 50) / 200.0);
    b.radius = 200 + rng->bounded(200);
    b.color = colors[i % colors.size()];
    b.phase = rng->bounded(1000) / 100.0;
    m_blobs.append(b);
  }

  m_timer = new QTimer(this);
  connect(m_timer, &QTimer::timeout, this, &LiveBackground::updateAnimation);
  m_timer->start(50);
}

void LiveBackground::updateAnimation() {
  m_time += 0.02;

  for (auto &b : m_blobs) {
    b.pos += b.velocity;

    if (b.pos.x() < -b.radius)
      b.pos.setX(width() + b.radius);
    if (b.pos.x() > width() + b.radius)
      b.pos.setX(-b.radius);
    if (b.pos.y() < -b.radius)
      b.pos.setY(height() + b.radius);
    if (b.pos.y() > height() + b.radius)
      b.pos.setY(-b.radius);
  }

  update();
}

void LiveBackground::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, false);

  p.fillRect(rect(), QColor(8, 8, 15));

  for (const auto &b : m_blobs) {
    QRadialGradient grad(b.pos, b.radius);
    QColor c = b.color;
    c.setAlpha(25);
    grad.setColorAt(0, c);
    grad.setColorAt(1, Qt::transparent);
    p.fillRect(rect(), grad);
  }
}

void LiveBackground::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
  setGeometry(parentWidget()->rect());
  lower();
}

} // namespace DevPlanner
