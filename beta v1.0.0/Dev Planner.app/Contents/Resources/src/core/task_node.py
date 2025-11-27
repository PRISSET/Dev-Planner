from PyQt6.QtWidgets import QWidget, QVBoxLayout, QHBoxLayout, QLabel, QPushButton, QLineEdit, QTextEdit, QMenu, QGraphicsDropShadowEffect
from PyQt6.QtCore import Qt, QPoint, QPointF, QRectF, QTimer, QPropertyAnimation, QEasingCurve
from PyQt6.QtGui import QPainter, QPainterPath, QColor, QPen, QPixmap, QIcon
from src.core.config import STATUSES


def create_color_icon(color_hex, size=16):
    pixmap = QPixmap(size, size)
    pixmap.fill(Qt.GlobalColor.transparent)
    painter = QPainter(pixmap)
    painter.setRenderHint(QPainter.RenderHint.Antialiasing)
    painter.setBrush(QColor(color_hex))
    painter.setPen(Qt.PenStyle.NoPen)
    painter.drawEllipse(2, 2, size - 4, size - 4)
    painter.end()
    return QIcon(pixmap)


class TaskNode(QWidget):
    BASE_WIDTH = 220
    BASE_HEIGHT = 140
    
    def __init__(self, x, y, canvas, title="Новая задача"):
        super().__init__(canvas)
        self.canvas = canvas
        self.node_x = x
        self.node_y = y
        self.status = 'none'
        self.connections = []
        self.dragging = False
        self.drag_offset = QPoint()
        self.connecting = False
        self.is_hover_target = False
        
        self.setFixedSize(self.BASE_WIDTH, self.BASE_HEIGHT)
        self.setMouseTracking(True)
        self.move(int(x), int(y))
        self.setStyleSheet("background: transparent;")
        
        self._setup_ui(title)
        self.show()
    
    def update_scale(self, scale):
        new_width = int(self.BASE_WIDTH * scale)
        new_height = int(self.BASE_HEIGHT * scale)
        self.setFixedSize(new_width, new_height)
    
    def _setup_ui(self, title):
        self.main_layout = QVBoxLayout(self)
        self.main_layout.setContentsMargins(12, 10, 12, 10)
        self.main_layout.setSpacing(6)
        
        header = QHBoxLayout()
        header.setSpacing(8)
        
        self.status_indicator = QLabel()
        self.status_indicator.setFixedSize(12, 12)
        self.update_status_indicator()
        
        self.title_edit = QLineEdit(title)
        self.title_edit.setContextMenuPolicy(Qt.ContextMenuPolicy.NoContextMenu)
        self.title_edit.setStyleSheet("""
            QLineEdit {
                background: transparent;
                border: none;
                outline: none;
                color: #ffffff;
                font-size: 13px;
                font-weight: bold;
                selection-background-color: rgba(0,255,255,0.3);
            }
            QLineEdit:focus {
                border: none;
                outline: none;
            }
        """)
        self.title_edit.textChanged.connect(self.on_change)

        self.delete_btn = QPushButton("×")
        self.delete_btn.setFixedSize(20, 20)
        self.delete_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        self.delete_btn.setStyleSheet("""
            QPushButton { background: transparent; color: #ff0055; border: none; font-size: 16px; font-weight: bold; }
            QPushButton:hover { color: #ffffff; }
        """)
        self.delete_btn.clicked.connect(self.delete_self)
        
        header.addWidget(self.status_indicator)
        header.addWidget(self.title_edit, 1)
        header.addWidget(self.delete_btn)
        
        self.desc_edit = QTextEdit()
        self.desc_edit.setPlaceholderText("Описание...")
        self.desc_edit.setContextMenuPolicy(Qt.ContextMenuPolicy.NoContextMenu)
        self.desc_edit.setStyleSheet("""
            QTextEdit {
                background: rgba(0,0,0,0.3);
                border: none;
                border-radius: 6px;
                color: #cccccc;
                font-size: 11px;
                padding: 5px;
                selection-background-color: rgba(0,255,255,0.3);
            }
            QTextEdit:focus {
                border: none;
                outline: none;
            }
        """)
        self.desc_edit.textChanged.connect(self.on_change)
        
        self.title_edit.installEventFilter(self)
        self.desc_edit.installEventFilter(self)
        self.desc_edit.viewport().installEventFilter(self)
        self.delete_btn.installEventFilter(self)
        
        self.main_layout.addLayout(header)
        self.main_layout.addWidget(self.desc_edit, 1)
    
    def eventFilter(self, obj, event):
        if self.canvas.connecting_from:
            if event.type() in (
                event.Type.MouseButtonPress,
                event.Type.MouseButtonRelease,
                event.Type.MouseButtonDblClick,
                event.Type.FocusIn,
                event.Type.KeyPress,
            ):
                if event.type() == event.Type.MouseButtonPress:
                    self.mousePressEvent(event)
                return True
        return super().eventFilter(obj, event)
    
    def on_change(self):
        if hasattr(self.canvas, 'main_window') and self.canvas.main_window:
            self.canvas.main_window.schedule_autosave()
    
    def update_status_indicator(self):
        color = STATUSES[self.status]['color']
        self.status_indicator.setStyleSheet(f"""
            background-color: {color};
            border-radius: 6px;
            border: 2px solid {color};
        """)
    
    def set_status(self, status):
        self.status = status
        self.update_status_indicator()
        self.canvas.update()
        self.on_change()
    
    def delete_self(self):
        self.canvas.remove_node(self)
    
    def get_center(self):
        return QPointF(self.node_x + self.BASE_WIDTH/2, self.node_y + self.BASE_HEIGHT/2)

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        
        color = QColor(STATUSES[self.status]['color'])
        path = QPainterPath()
        path.addRoundedRect(QRectF(0, 0, self.width(), self.height()), 10, 10)
        
        if self.is_hover_target:
            painter.fillPath(path, QColor(0, 255, 255, 40))
            painter.setPen(QPen(QColor("#00ffff"), 3))
        else:
            painter.fillPath(path, QColor(20, 20, 20, 240))
            painter.setPen(QPen(color, 2))
        painter.drawPath(path)
        painter.end()
    
    def set_hover_target(self, is_target):
        if self.is_hover_target != is_target:
            self.is_hover_target = is_target
            self.update()
    
    def mousePressEvent(self, event):
        if self.canvas.connecting_from:
            self.set_hover_target(False)
            if self.canvas.connecting_from != self:
                self.canvas.complete_connection(self)
            else:
                self.canvas.cancel_connection()
            event.accept()
            return
            
        if event.button() == Qt.MouseButton.LeftButton:
            self.dragging = True
            self.drag_offset = event.pos()
            self.raise_()
        elif event.button() == Qt.MouseButton.RightButton:
            self.show_context_menu(event.globalPosition().toPoint())
    
    def enterEvent(self, event):
        if self.canvas.connecting_from and self.canvas.connecting_from != self:
            self.set_hover_target(True)
            self.canvas.hover_target = self
    
    def leaveEvent(self, event):
        if self.is_hover_target:
            self.set_hover_target(False)
            if self.canvas.hover_target == self:
                self.canvas.hover_target = None
    
    def mouseMoveEvent(self, event):
        if self.canvas.connecting_from:
            global_pos = self.mapToParent(event.pos())
            self.canvas.mouse_pos = QPointF(global_pos.x(), global_pos.y())
            self.canvas.connection_overlay.update()
            return
            
        if self.dragging:
            new_pos = self.mapToParent(event.pos() - self.drag_offset)
            self.move(new_pos)
            self.node_x = (new_pos.x() - self.canvas.offset.x()) / self.canvas.scale
            self.node_y = (new_pos.y() - self.canvas.offset.y()) / self.canvas.scale
            self.canvas.update()
            self.canvas.connection_overlay.update()
    
    def mouseReleaseEvent(self, event):
        self.dragging = False
        self.on_change()
    
    def show_context_menu(self, pos):
        menu = QMenu(self)
        menu.setStyleSheet("""
            QMenu { background-color: #1a1a1a; border: 1px solid #333333; border-radius: 5px; padding: 5px; }
            QMenu::item { color: #ffffff; padding: 8px 20px; border-radius: 3px; }
            QMenu::item:selected { background-color: #333333; }
        """)
        
        status_menu = menu.addMenu("Статус")
        status_menu.setStyleSheet("""
            QMenu { background-color: #1a1a1a; border: 1px solid #333333; border-radius: 5px; padding: 5px; }
            QMenu::item { color: #ffffff; padding: 8px 20px; border-radius: 3px; }
            QMenu::item:selected { background-color: #333333; }
        """)
        for key, value in STATUSES.items():
            color = value['color']
            icon = create_color_icon(color)
            action = status_menu.addAction(icon, value['name'])
            action.triggered.connect(lambda checked, k=key: self.set_status(k))
        
        menu.addSeparator()
        connect_action = menu.addAction("Соединить с...")
        connect_action.triggered.connect(self.start_connection)
        menu.addSeparator()
        delete_action = menu.addAction("Удалить")
        delete_action.triggered.connect(self.delete_self)
        menu.exec(pos)
    
    def start_connection(self):
        self.canvas.start_connection(self)
    
    def get_data(self):
        return {
            'title': self.title_edit.text(),
            'description': self.desc_edit.toPlainText(),
            'status': self.status,
            'x': self.node_x,
            'y': self.node_y
        }
    
    def load_data(self, data):
        self.title_edit.setText(data.get('title', 'Новая задача'))
        self.desc_edit.setPlainText(data.get('description', ''))
        self.status = data.get('status', 'none')
        self.update_status_indicator()
