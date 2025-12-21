#ifndef NODE_CANVAS_HPP
#define NODE_CANVAS_HPP

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QPair>
#include <QPointF>
#include <QTimer>
#include <QWidget>

namespace DevPlanner {

class TaskNode;

// Overlay widget for drawing connections on top of nodes
class ConnectionOverlay : public QWidget {
  Q_OBJECT
public:
  explicit ConnectionOverlay(class NodeCanvas *canvas);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  NodeCanvas *m_canvas;
};

class NodeCanvas : public QWidget {
  Q_OBJECT

public:
  explicit NodeCanvas(QWidget *parent = nullptr);
  ~NodeCanvas();

  // Node management
  TaskNode *addNode(qreal x, qreal y, bool emitChanged = true);
  void removeNode(TaskNode *node, bool emitChanged = true);
  void clearAll();

  // View transformation
  qreal scale() const { return m_scale; }
  QPointF offset() const { return m_offset; }

  void zoomIn();
  void zoomOut();
  void zoomReset();

  // Connection mode
  bool isConnecting() const { return m_connectingFrom != nullptr; }
  TaskNode *getConnectingFrom() const { return m_connectingFrom; }
  void startConnection(TaskNode *node);
  void completeConnection(TaskNode *targetNode, bool emitChanged = true);
  void cancelConnection();

  // Hover target
  void setHoverTarget(TaskNode *node);
  TaskNode *hoverTarget() const { return m_hoverTarget; }

  // Mouse position for drawing connection line
  void updateMousePosition(const QPointF &pos);
  QPointF mousePosition() const { return m_mousePos; }

  // Connection overlay
  void updateConnectionOverlay();

  // Connections
  const QList<QPair<TaskNode *, TaskNode *>> &connections() const {
    return m_connections;
  }
  void addConnection(TaskNode *from, TaskNode *to);
  void removeConnection(TaskNode *from, TaskNode *to);

  // Update node display position
  void updateNodePosition(TaskNode *node);
  void updateAllNodes();

  // Stats
  QMap<QString, int> getStats() const;

  // Serialization
  QJsonObject getProjectData() const;
  void loadProjectData(const QJsonObject &data);

  // For drawing connections
  qreal lineDashOffset() const { return m_lineDashOffset; }

  // Access to nodes
  const QList<TaskNode *> &nodes() const { return m_nodes; }

signals:
  void changed();
  void zoomChanged(int percent);

protected:
  void paintEvent(QPaintEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  bool event(QEvent *event) override;

private slots:
  void animateLine();
  void onNodeChanged();
  void onNodeDeleteRequested(TaskNode *node);
  void onNodeConnectionRequested(TaskNode *node);

private:
  void applyZoom(qreal factor, const QPointF &mousePos);
  void drawConnection(QPainter &painter, TaskNode *node1, TaskNode *node2);

  QList<TaskNode *> m_nodes;
  QList<QPair<TaskNode *, TaskNode *>> m_connections;

  qreal m_scale = 1.0;
  QPointF m_offset{0, 0};

  bool m_isPanning = false;
  QPoint m_panStart;

  TaskNode *m_connectingFrom = nullptr;
  TaskNode *m_hoverTarget = nullptr;
  QPointF m_mousePos;

  qreal m_lineDashOffset = 0;
  QTimer *m_lineAnimationTimer = nullptr;

  ConnectionOverlay *m_connectionOverlay;

  friend class ConnectionOverlay;
};

} // namespace DevPlanner

#endif // NODE_CANVAS_HPP
