#ifndef LIVE_BACKGROUND_HPP
#define LIVE_BACKGROUND_HPP

#include <QColor>
#include <QList>
#include <QPointF>
#include <QTimer>
#include <QWidget>

namespace DevPlanner {

struct NebulaBlob {
  QPointF pos;
  QPointF velocity;
  qreal baseRadius;
  qreal currentRadius;
  QColor color;
  qreal pulsePhase;
  qreal pulseSpeed;
};

class LiveBackground : public QWidget {
  Q_OBJECT
public:
  explicit LiveBackground(QWidget *parent = nullptr);

  void pause();
  void resume();

protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private slots:
  void updateAnimation();

private:
  void initBlobs();

  QTimer *m_timer;
  QList<NebulaBlob> m_blobs;
  qreal m_time;
  bool m_paused = false;
};

} // namespace DevPlanner

#endif // LIVE_BACKGROUND_HPP
