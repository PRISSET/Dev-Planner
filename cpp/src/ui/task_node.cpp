#include "task_node.hpp"
#include "core/config.hpp"
#include "node_canvas.hpp"
#include <QHBoxLayout>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QVBoxLayout>

namespace DevPlanner {

TaskNode::TaskNode(qreal x, qreal y, NodeCanvas *canvas, const QString &title,
                   QWidget *parent)
    : GlassmorphismWidget(parent), m_canvas(canvas), m_nodeX(x), m_nodeY(y) {
  setFixedSize(BASE_WIDTH, BASE_HEIGHT);
  setMouseTracking(true);
  setAttribute(Qt::WA_TranslucentBackground);
  move(static_cast<int>(x), static_cast<int>(y));
  setupUI(title);
  show();
}

void TaskNode::setupUI(const QString &title) {
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(15, 12, 15, 12);
  layout->setSpacing(8);

  auto *header = new QHBoxLayout();
  header->setSpacing(10);

  m_statusIndicator = new QLabel(this);
  updateStatusIndicator();

  m_titleEdit = new QLineEdit(title, this);
  m_titleEdit->setContextMenuPolicy(Qt::NoContextMenu);
  m_titleEdit->setStyleSheet(
      "QLineEdit { background: transparent; border: none; color: #ffffff; "
      "font-size: 14px; font-weight: 800; }");
  connect(m_titleEdit, &QLineEdit::textChanged, this,
          &TaskNode::onContentChanged);
  m_titleEdit->installEventFilter(this);

  m_deleteBtn = new QPushButton("×", this);
  m_deleteBtn->setFixedSize(24, 24);
  m_deleteBtn->setStyleSheet(
      "QPushButton { background: transparent; color: rgba(255,255,255,0.3); "
      "border: none; font-size: 20px; } QPushButton:hover { color: #ff0055; }");
  connect(m_deleteBtn, &QPushButton::clicked, this, &TaskNode::onDeleteClicked);
  m_deleteBtn->installEventFilter(this);

  header->addWidget(m_statusIndicator);
  header->addWidget(m_titleEdit, 1);
  header->addWidget(m_deleteBtn);

  m_descEdit = new QTextEdit(this);
  m_descEdit->setContextMenuPolicy(Qt::NoContextMenu);
  m_descEdit->setPlaceholderText("Notes...");
  m_descEdit->setStyleSheet(
      "QTextEdit { background: rgba(0,0,0,0.2); border: 1px solid "
      "rgba(255,255,255,0.05); "
      "border-radius: 12px; color: rgba(255,255,255,0.8); font-size: 13px; "
      "padding: 10px; }");
  connect(m_descEdit, &QTextEdit::textChanged, this,
          &TaskNode::onContentChanged);
  m_descEdit->installEventFilter(this);
  m_descEdit->viewport()->installEventFilter(this);

  if (title.isEmpty()) {
    m_titleEdit->hide();
    layout->setContentsMargins(10, 8, 10, 8);
    layout->setSpacing(4);
    auto *miniHeader = new QHBoxLayout();
    miniHeader->addWidget(m_statusIndicator);
    miniHeader->addStretch();
    miniHeader->addWidget(m_deleteBtn);
    layout->addLayout(miniHeader);
  } else {
    layout->addLayout(header);
  }
  layout->addWidget(m_descEdit, 1);
}

bool TaskNode::eventFilter(QObject *obj, QEvent *event) {
  if (m_canvas && m_canvas->isConnecting()) {
    if (event->type() == QEvent::MouseButtonPress) {
      auto *me = static_cast<QMouseEvent *>(event);
      mousePressEvent(me);
      return true;
    }
  }
  return GlassmorphismWidget::eventFilter(obj, event);
}

void TaskNode::onContentChanged() { emit changed(); }
void TaskNode::onDeleteClicked() { emit deleteRequested(this); }

void TaskNode::updateStatusIndicator() {
  QString c = getStatuses()[m_status].color;
  m_statusIndicator->setStyleSheet(
      QString("background-color: %1; border-radius: 6px; "
              "min-width: 12px; max-width: 12px; "
              "min-height: 12px; max-height: 12px;")
          .arg(c));
}

void TaskNode::setStatus(const QString &s) {
  if (getStatuses().contains(s)) {
    m_status = s;
    updateStatusIndicator();
    update();
    emit changed();
  }
}
void TaskNode::setNodePosition(qreal x, qreal y) {
  m_nodeX = x;
  m_nodeY = y;
}
QString TaskNode::title() const { return m_titleEdit->text(); }
void TaskNode::setTitle(const QString &t) { m_titleEdit->setText(t); }
QString TaskNode::description() const { return m_descEdit->toPlainText(); }
void TaskNode::setDescription(const QString &d) { m_descEdit->setPlainText(d); }
QPointF TaskNode::getCenter() const {
  return QPointF(m_nodeX + BASE_WIDTH / 2.0, m_nodeY + BASE_HEIGHT / 2.0);
}
void TaskNode::updateScale(qreal s) {
  qreal effectiveScale = qMax(s, 0.5);

  setFixedSize(static_cast<int>(BASE_WIDTH * effectiveScale),
               static_cast<int>(BASE_HEIGHT * effectiveScale));

  int titleFontSize = qMax(static_cast<int>(14 * effectiveScale), 9);
  int descFontSize = qMax(static_cast<int>(13 * effectiveScale), 8);
  int btnSize = qMax(static_cast<int>(24 * effectiveScale), 14);
  int indicatorSize = qMax(static_cast<int>(10 * effectiveScale), 6);
  int margins = qMax(static_cast<int>(15 * effectiveScale), 8);

  m_titleEdit->setStyleSheet(
      QString(
          "QLineEdit { background: transparent; border: none; color: #ffffff; "
          "font-size: %1px; font-weight: 800; }")
          .arg(titleFontSize));

  m_descEdit->setStyleSheet(
      QString("QTextEdit { background: rgba(0,0,0,0.2); border: 1px solid "
              "rgba(255,255,255,0.05); "
              "border-radius: %1px; color: rgba(255,255,255,0.8); font-size: "
              "%2px; padding: %3px; }")
          .arg(qMax(static_cast<int>(12 * effectiveScale), 6))
          .arg(descFontSize)
          .arg(qMax(static_cast<int>(10 * effectiveScale), 4)));

  m_deleteBtn->setFixedSize(btnSize, btnSize);
  m_deleteBtn->setStyleSheet(
      QString("QPushButton { background: transparent; color: "
              "rgba(255,255,255,0.3); "
              "border: none; font-size: %1px; } QPushButton:hover { color: "
              "#ff0055; }")
          .arg(qMax(static_cast<int>(20 * effectiveScale), 12)));

  m_statusIndicator->setFixedSize(indicatorSize, indicatorSize);
  QString statusColor = getStatuses()[m_status].color;
  int radius = indicatorSize / 2;
  m_statusIndicator->setStyleSheet(
      QString("background-color: %1; border-radius: %2px; "
              "min-width: %3px; max-width: %3px; "
              "min-height: %3px; max-height: %3px;")
          .arg(statusColor)
          .arg(radius)
          .arg(indicatorSize));

  if (layout()) {
    layout()->setContentsMargins(margins, margins - 3, margins, margins - 3);
  }
}
void TaskNode::setHoverTarget(bool t) {
  if (m_isHoverTarget != t) {
    m_isHoverTarget = t;
    update();
  }
}

void TaskNode::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);

  QColor statusColor(getStatuses()[m_status].color);
  QPainterPath path;
  path.addRoundedRect(rect().adjusted(1, 1, -1, -1), 16, 16);

  painter.setClipPath(path);
  painter.fillPath(path, QColor(12, 12, 20, 250));

  painter.setClipping(false);
  QPen pen(m_isHoverTarget ? statusColor : QColor(60, 60, 80),
           m_isHoverTarget ? 2 : 1);
  painter.setPen(pen);
  painter.drawPath(path);
}

void TaskNode::mousePressEvent(QMouseEvent *e) {
  if (m_canvas && m_canvas->isConnecting()) {
    setHoverTarget(false);
    if (m_canvas->getConnectingFrom() != this)
      m_canvas->completeConnection(this);
    else
      m_canvas->cancelConnection();
    e->accept();
    return;
  }
  if (e->button() == Qt::LeftButton) {
    m_isDragging = true;
    m_dragOffset = e->pos();
    raise();
  } else if (e->button() == Qt::RightButton)
    showContextMenu(e->globalPosition().toPoint());
}

void TaskNode::mouseMoveEvent(QMouseEvent *e) {
  if (m_canvas && m_canvas->isConnecting()) {
    QPoint p = mapToParent(e->pos());
    m_canvas->updateMousePosition(QPointF(p));
    return;
  }
  if (m_isDragging) {
    QPoint p = mapToParent(e->pos() - m_dragOffset);
    move(p);
    if (m_canvas) {
      m_nodeX = (p.x() - m_canvas->offset().x()) / m_canvas->scale();
      m_nodeY = (p.y() - m_canvas->offset().y()) / m_canvas->scale();
      m_canvas->update();
      m_canvas->updateConnectionOverlay();
    }
  }
}

void TaskNode::mouseReleaseEvent(QMouseEvent *e) {
  m_isDragging = false;
  emit changed();
}
void TaskNode::enterEvent(QEnterEvent *e) {
  if (m_canvas && m_canvas->isConnecting() &&
      m_canvas->getConnectingFrom() != this) {
    setHoverTarget(true);
    m_canvas->setHoverTarget(this);
  }
}
void TaskNode::leaveEvent(QEvent *e) {
  if (m_isHoverTarget) {
    setHoverTarget(false);
    if (m_canvas)
      m_canvas->setHoverTarget(nullptr);
  }
}

void TaskNode::showContextMenu(const QPoint &p) {
  QMenu menu(this);
  menu.setStyleSheet(
      "QMenu { background: rgba(30,30,40,240); border: 1px solid "
      "rgba(255,255,255,0.1); border-radius: 10px; color: #ffffff; padding: "
      "5px; } QMenu::item:selected { background: rgba(217,0,255,0.3); }");
  auto *s = menu.addMenu("Status");
  for (auto it = getStatuses().begin(); it != getStatuses().end(); ++it) {
    auto *a = s->addAction(it.value().name);
    QString k = it.key();
    connect(a, &QAction::triggered, this, [this, k]() { setStatus(k); });
  }
  menu.addSeparator();
  connect(menu.addAction("Connect"), &QAction::triggered, this,
          [this]() { emit connectionRequested(this); });
  connect(menu.addAction("Delete"), &QAction::triggered, this,
          &TaskNode::onDeleteClicked);
  menu.exec(p);
}

void TaskNode::startConnection() { emit connectionRequested(this); }
QJsonObject TaskNode::getData() const {
  QJsonObject o;
  o["title"] = m_titleEdit->text();
  o["description"] = m_descEdit->toPlainText();
  o["status"] = m_status;
  o["x"] = m_nodeX;
  o["y"] = m_nodeY;
  return o;
}
void TaskNode::loadData(const QJsonObject &d) {
  m_titleEdit->setText(d["title"].toString());
  m_descEdit->setPlainText(d["description"].toString());
  m_status = d["status"].toString("none");
  updateStatusIndicator();
}

} // namespace DevPlanner
