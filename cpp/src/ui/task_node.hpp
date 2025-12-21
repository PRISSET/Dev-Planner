#ifndef TASK_NODE_HPP
#define TASK_NODE_HPP

#include "glassmorphism_widget.hpp"
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPoint>
#include <QPushButton>
#include <QTextEdit>

namespace DevPlanner {

class NodeCanvas;

class TaskNode : public GlassmorphismWidget {
  Q_OBJECT

public:
  static constexpr int BASE_WIDTH = 220;
  static constexpr int BASE_HEIGHT = 140;

  explicit TaskNode(qreal x, qreal y, NodeCanvas *canvas,
                    const QString &title = "Новая задача",
                    QWidget *parent = nullptr);

  // Position in canvas coordinates
  qreal nodeX() const { return m_nodeX; }
  qreal nodeY() const { return m_nodeY; }
  void setNodePosition(qreal x, qreal y);

  // Status
  QString status() const { return m_status; }
  void setStatus(const QString &status);

  // Data
  QString title() const;
  void setTitle(const QString &title);
  QString description() const;
  void setDescription(const QString &desc);

  QPointF getCenter() const;

  // Scale
  void updateScale(qreal scale);

  // Hover target for connections
  void setHoverTarget(bool isTarget);
  bool isHoverTarget() const { return m_isHoverTarget; }

  // Serialization
  QJsonObject getData() const;
  void loadData(const QJsonObject &data);

signals:
  void changed();
  void deleteRequested(TaskNode *node);
  void connectionRequested(TaskNode *fromNode);

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;
  bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
  void onContentChanged();
  void onDeleteClicked();
  void showContextMenu(const QPoint &pos);
  void startConnection();

private:
  void setupUI(const QString &title);
  void updateStatusIndicator();
  QIcon createColorIcon(const QString &colorHex, int size = 16);

  NodeCanvas *m_canvas;
  qreal m_nodeX;
  qreal m_nodeY;
  QString m_status = "none";

  QLabel *m_statusIndicator;
  QLineEdit *m_titleEdit;
  QTextEdit *m_descEdit;
  QPushButton *m_deleteBtn;

  bool m_isDragging = false;
  QPoint m_dragOffset;
  bool m_isHoverTarget = false;
};

} // namespace DevPlanner

#endif // TASK_NODE_HPP
