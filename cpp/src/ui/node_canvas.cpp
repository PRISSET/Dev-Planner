#include "node_canvas.hpp"
#include "core/config.hpp"
#include "task_node.hpp"
#include <QGestureEvent>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPinchGesture>
#include <QRadialGradient>
#include <QRandomGenerator>
#include <QWheelEvent>

namespace DevPlanner {

ConnectionOverlay::ConnectionOverlay(NodeCanvas *canvas)
    : QWidget(canvas), m_canvas(canvas) {
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setAttribute(Qt::WA_TranslucentBackground);
  hide();
}

void ConnectionOverlay::paintEvent(QPaintEvent *event) {}

NodeCanvas::NodeCanvas(QWidget *parent) : QWidget(parent) {
  setMinimumSize(800, 600);
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);
  setAttribute(Qt::WA_AcceptTouchEvents, true);
  setAttribute(Qt::WA_OpaquePaintEvent, true);
  grabGesture(Qt::PinchGesture);
  m_connectionOverlay = new ConnectionOverlay(this);

  QList<QColor> colors = {QColor(138, 43, 226, 35), QColor(180, 0, 180, 30),
                          QColor(100, 20, 200, 35), QColor(200, 0, 120, 30),
                          QColor(60, 0, 160, 35),   QColor(0, 100, 180, 25)};

  auto *rng = QRandomGenerator::global();
  for (int i = 0; i < 6; ++i) {
    Blob b;
    b.pos = QPointF(rng->bounded(1920), rng->bounded(1080));
    b.velocity = QPointF((rng->bounded(200) - 100) / 80.0,
                         (rng->bounded(200) - 100) / 80.0);
    b.radius = 300 + rng->bounded(200);
    b.color = colors[i];
    m_blobs.append(b);
  }

  m_blobTimer = new QTimer(this);
  connect(m_blobTimer, &QTimer::timeout, this, &NodeCanvas::updateBlobs);
  m_blobTimer->start(33);
}

NodeCanvas::~NodeCanvas() { clearAll(); }

TaskNode *NodeCanvas::addNode(qreal x, qreal y, bool emitChanged) {
  QString title = m_noteMode ? "" : "New Task";
  TaskNode *node = new TaskNode(x, y, this, title, this);
  m_nodes.append(node);
  node->updateScale(m_scale);
  updateNodePosition(node);
  connect(node, &TaskNode::changed, this, &NodeCanvas::onNodeChanged);
  connect(node, &TaskNode::deleteRequested, this,
          &NodeCanvas::onNodeDeleteRequested);
  connect(node, &TaskNode::connectionRequested, this,
          &NodeCanvas::onNodeConnectionRequested);
  update();
  if (emitChanged)
    emit changed();
  return node;
}

void NodeCanvas::removeNode(TaskNode *node, bool emitChanged) {
  if (!m_nodes.contains(node))
    return;
  m_connections.erase(std::remove_if(m_connections.begin(), m_connections.end(),
                                     [node](const auto &c) {
                                       return c.first == node ||
                                              c.second == node;
                                     }),
                      m_connections.end());
  m_nodes.removeOne(node);
  node->deleteLater();
  update();
  if (emitChanged)
    emit changed();
}

void NodeCanvas::clearAll() {
  for (auto *n : m_nodes)
    n->deleteLater();
  m_nodes.clear();
  m_connections.clear();
  update();
}

void NodeCanvas::updateNodePosition(TaskNode *n) {
  n->move(static_cast<int>(n->nodeX() * m_scale + m_offset.x()),
          static_cast<int>(n->nodeY() * m_scale + m_offset.y()));
}

void NodeCanvas::updateAllNodes() {
  for (auto *n : m_nodes)
    updateNodePosition(n);
  update();
}

void NodeCanvas::startConnection(TaskNode *n) {
  m_connectingFrom = n;
  setCursor(Qt::CrossCursor);
  update();
}

void NodeCanvas::animateLine() { update(); }

void NodeCanvas::cancelConnection() {
  if (m_hoverTarget)
    m_hoverTarget->setHoverTarget(false);
  m_hoverTarget = nullptr;
  m_connectingFrom = nullptr;
  setCursor(Qt::ArrowCursor);
  update();
}

void NodeCanvas::completeConnection(TaskNode *t, bool emitChanged) {
  if (m_connectingFrom && m_connectingFrom != t) {
    bool ex = false;
    for (const auto &c : m_connections)
      if ((c.first == m_connectingFrom && c.second == t) ||
          (c.first == t && c.second == m_connectingFrom)) {
        ex = true;
        break;
      }
    if (!ex) {
      m_connections.append(qMakePair(m_connectingFrom, t));
      if (emitChanged)
        emit changed();
    }
  }
  if (m_hoverTarget)
    m_hoverTarget->setHoverTarget(false);
  m_hoverTarget = nullptr;
  m_connectingFrom = nullptr;
  setCursor(Qt::ArrowCursor);
  update();
}

void NodeCanvas::setHoverTarget(TaskNode *n) { m_hoverTarget = n; }
void NodeCanvas::updateMousePosition(const QPointF &p) {
  m_mousePos = p;
  update();
}
void NodeCanvas::updateConnectionOverlay() { update(); }
void NodeCanvas::addConnection(TaskNode *f, TaskNode *t) {
  if (f && t && f != t) {
    m_connections.append(qMakePair(f, t));
    update();
  }
}
void NodeCanvas::removeConnection(TaskNode *f, TaskNode *t) {
  m_connections.erase(std::remove_if(m_connections.begin(), m_connections.end(),
                                     [f, t](const auto &c) {
                                       return (c.first == f && c.second == t) ||
                                              (c.first == t && c.second == f);
                                     }),
                      m_connections.end());
  update();
}
void NodeCanvas::applyZoom(qreal f, const QPointF &p) {
  qreal old = m_scale;
  m_scale = qBound(0.2, m_scale * f, 4.0);
  if (old != m_scale) {
    m_offset = p - (p - m_offset) * (m_scale / old);
    for (auto *n : m_nodes) {
      n->updateScale(m_scale);
      updateNodePosition(n);
    }
    update();
    emit zoomChanged(static_cast<int>(m_scale * 100));
  }
}
void NodeCanvas::zoomIn() { applyZoom(1.2, rect().center()); }
void NodeCanvas::zoomOut() { applyZoom(0.8, rect().center()); }
void NodeCanvas::zoomReset() {
  m_scale = 1.0;
  m_offset = QPointF(0, 0);
  for (auto *n : m_nodes) {
    n->updateScale(m_scale);
    updateNodePosition(n);
  }
  update();
  emit zoomChanged(100);
}
QMap<QString, int> NodeCanvas::getStats() const {
  QMap<QString, int> s;
  for (auto *n : m_nodes)
    s[n->status()]++;
  return s;
}

void NodeCanvas::paintEvent(QPaintEvent *e) {
  Q_UNUSED(e);
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, false);

  p.fillRect(rect(), QColor(8, 8, 15));

  for (const auto &b : m_blobs) {
    QRadialGradient grad(b.pos, b.radius);
    grad.setColorAt(0, b.color);
    grad.setColorAt(1, Qt::transparent);
    p.fillRect(rect(), grad);
  }

  int gs = static_cast<int>(50 * m_scale);
  if (gs > 15) {
    p.setPen(QPen(QColor(255, 255, 255, 12), 1));
    int sx = static_cast<int>(m_offset.x()) % gs;
    int sy = static_cast<int>(m_offset.y()) % gs;
    int w = width(), h = height();
    for (int x = sx; x < w; x += gs)
      p.drawLine(x, 0, x, h);
    for (int y = sy; y < h; y += gs)
      p.drawLine(0, y, w, y);
  }

  p.setRenderHint(QPainter::Antialiasing, true);
  for (const auto &conn : m_connections) {
    drawConnection(p, conn.first, conn.second);
  }

  if (m_connectingFrom) {
    QPointF start(m_connectingFrom->x() + m_connectingFrom->width() / 2.0,
                  m_connectingFrom->y() + m_connectingFrom->height() / 2.0);
    QPointF end = m_mousePos;
    if (m_hoverTarget) {
      end = QPointF(m_hoverTarget->x() + m_hoverTarget->width() / 2.0,
                    m_hoverTarget->y() + m_hoverTarget->height() / 2.0);
    }
    QPen pen(QColor(217, 0, 255, 200), 2);
    pen.setStyle(Qt::DashLine);
    p.setPen(pen);
    QPainterPath path;
    path.moveTo(start);
    qreal ctrl = qMax(qAbs(end.x() - start.x()) / 2.0, 50.0);
    path.cubicTo(QPointF(start.x() + ctrl, start.y()),
                 QPointF(end.x() - ctrl, end.y()), end);
    p.drawPath(path);
  }
}

void NodeCanvas::drawConnection(QPainter &p, TaskNode *n1, TaskNode *n2) {
  QPointF s(n1->x() + n1->width() / 2.0, n1->y() + n1->height() / 2.0);
  QPointF e(n2->x() + n2->width() / 2.0, n2->y() + n2->height() / 2.0);
  QLinearGradient g(s, e);
  g.setColorAt(0, getStatuses()[n1->status()].color);
  g.setColorAt(1, getStatuses()[n2->status()].color);
  p.setPen(QPen(QBrush(g), 2, Qt::SolidLine, Qt::RoundCap));
  QPainterPath path;
  path.moveTo(s);
  qreal ctrl = qAbs(e.x() - s.x()) / 2.0;
  path.cubicTo(QPointF(s.x() + ctrl, s.y()), QPointF(e.x() - ctrl, e.y()), e);
  p.drawPath(path);
}

void NodeCanvas::wheelEvent(QWheelEvent *e) {
  if (e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier))
    applyZoom(e->angleDelta().y() > 0 ? 1.1 : 0.9, e->position());
  else {
    m_offset += (e->pixelDelta().isNull() ? QPointF(e->angleDelta().x() / 2.0,
                                                    e->angleDelta().y() / 2.0)
                                          : QPointF(e->pixelDelta()));
    updateAllNodes();
  }
  e->accept();
}

void NodeCanvas::mousePressEvent(QMouseEvent *e) {
  if (e->button() == Qt::MiddleButton ||
      (e->button() == Qt::LeftButton && (e->modifiers() & Qt::AltModifier))) {
    m_isPanning = true;
    m_panStart = e->pos();
    setCursor(Qt::ClosedHandCursor);
  } else if (e->button() == Qt::LeftButton && m_connectingFrom)
    cancelConnection();
}

void NodeCanvas::mouseMoveEvent(QMouseEvent *e) {
  m_mousePos = e->position();
  if (m_isPanning) {
    m_offset += e->position() - QPointF(m_panStart);
    m_panStart = e->pos();
    updateAllNodes();
  } else if (m_connectingFrom) {
    update();
  }
}

void NodeCanvas::mouseReleaseEvent(QMouseEvent *e) {
  m_isPanning = false;
  setCursor(Qt::ArrowCursor);
}

void NodeCanvas::mouseDoubleClickEvent(QMouseEvent *e) {
  if (e->button() == Qt::LeftButton)
    addNode((e->position().x() - m_offset.x()) / m_scale -
                TaskNode::BASE_WIDTH / 2.0,
            (e->position().y() - m_offset.y()) / m_scale -
                TaskNode::BASE_HEIGHT / 2.0);
}

void NodeCanvas::resizeEvent(QResizeEvent *e) { QWidget::resizeEvent(e); }

bool NodeCanvas::event(QEvent *e) {
  if (e->type() == QEvent::Gesture) {
    auto *ge = static_cast<QGestureEvent *>(e);
    if (auto *p = static_cast<QPinchGesture *>(ge->gesture(Qt::PinchGesture))) {
      if (p->changeFlags() & QPinchGesture::ScaleFactorChanged)
        applyZoom(p->scaleFactor(), p->centerPoint());
      return true;
    }
  }
  return QWidget::event(e);
}

void NodeCanvas::onNodeChanged() { emit changed(); }
void NodeCanvas::onNodeDeleteRequested(TaskNode *n) { removeNode(n); }
void NodeCanvas::onNodeConnectionRequested(TaskNode *n) { startConnection(n); }

QJsonObject NodeCanvas::getProjectData() const {
  QJsonArray nd, cd;
  for (auto *n : m_nodes)
    nd.append(n->getData());
  for (const auto &c : m_connections) {
    int i1 = m_nodes.indexOf(c.first), i2 = m_nodes.indexOf(c.second);
    if (i1 >= 0 && i2 >= 0) {
      QJsonArray ca;
      ca.append(i1);
      ca.append(i2);
      cd.append(ca);
    }
  }
  QJsonObject o;
  o["nodes"] = nd;
  o["connections"] = cd;
  o["scale"] = m_scale;
  o["offset_x"] = m_offset.x();
  o["offset_y"] = m_offset.y();
  return o;
}

void NodeCanvas::loadProjectData(const QJsonObject &d) {
  clearAll();
  m_scale = d["scale"].toDouble(1.0);
  m_offset = QPointF(d["offset_x"].toDouble(), d["offset_y"].toDouble());
  QJsonArray nd = d["nodes"].toArray();
  for (const auto &nv : nd) {
    auto o = nv.toObject();
    auto *n = addNode(o["x"].toDouble(), o["y"].toDouble(), false);
    n->loadData(o);
  }
  QJsonArray cd = d["connections"].toArray();
  for (const auto &cv : cd) {
    auto c = cv.toArray();
    if (c.size() == 2)
      addConnection(m_nodes[c[0].toInt()], m_nodes[c[1].toInt()]);
  }
  update();
}

void NodeCanvas::updateBlobs() {
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

} // namespace DevPlanner
