#ifndef LIVE_BACKGROUND_HPP
#define LIVE_BACKGROUND_HPP

#include <QColor>
#include <QList>
#include <QPointF>
#include <QTimer>
#include <QWidget>

namespace DevPlanner {

class LiveBackground : public QWidget {
  Q_OBJECT
public:
  explicit LiveBackground(QWidget *parent = nullptr);

protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private slots:
  void updateAnimation();

private:
  struct Blob {
    QPointF pos;
    QPointF velocity;
    qreal radius;
    QColor color;
    qreal phase;
  };

  QList<Blob> m_blobs;
  QTimer *m_timer;
  qreal m_time = 0;
};

} // namespace DevPlanner

#endif
