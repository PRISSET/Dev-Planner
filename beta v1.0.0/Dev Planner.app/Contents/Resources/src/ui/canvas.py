from PyQt6.QtWidgets import QWidget, QGestureEvent, QPinchGesture
from PyQt6.QtCore import Qt, QPoint, QPointF, QTimer
from PyQt6.QtGui import QPainter, QPainterPath, QColor, QPen, QBrush, QLinearGradient
from src.core.config import STATUSES
from src.core.task_node import TaskNode


class ConnectionOverlay(QWidget):
    def __init__(self, canvas):
        super().__init__(canvas)
        self.canvas = canvas
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents)
        self.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground)
        self.setStyleSheet("background: transparent;")
    
    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        
        for node1, node2 in self.canvas.connections:
            self.canvas.draw_connection(painter, node1, node2)
        
        if self.canvas.connecting_from:
            node = self.canvas.connecting_from
            start = QPointF(
                node.x() + node.width() / 2,
                node.y() + node.height() / 2
            )
            end = self.canvas.mouse_pos
            
            if self.canvas.hover_target:
                target = self.canvas.hover_target
                end = QPointF(
                    target.x() + target.width() / 2,
                    target.y() + target.height() / 2
                )
            
            pen = QPen(QColor("#00ffff"), 3)
            pen.setStyle(Qt.PenStyle.CustomDashLine)
            pen.setDashPattern([5, 5])
            pen.setDashOffset(self.canvas.line_dash_offset)
            painter.setPen(pen)
            
            path = QPainterPath()
            path.moveTo(start)
            ctrl_offset = abs(end.x() - start.x()) / 2
            ctrl_offset = max(ctrl_offset, 50)
            path.cubicTo(QPointF(start.x() + ctrl_offset, start.y()),
                         QPointF(end.x() - ctrl_offset, end.y()), end)
            painter.drawPath(path)
        
        painter.end()


class NodeCanvas(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.nodes = []
        self.connections = []
        self.scale = 1.0
        self.offset = QPointF(0, 0)
        self.panning = False
        self.pan_start = QPoint()
        self.connecting_from = None
        self.mouse_pos = QPointF()
        self.main_window = None
        self.hover_target = None
        self.line_dash_offset = 0
        self.line_animation_timer = None
        
        self.setMinimumSize(800, 600)
        self.setMouseTracking(True)
        self.setStyleSheet("background-color: #0a0a0a;")
        self.setFocusPolicy(Qt.FocusPolicy.StrongFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_AcceptTouchEvents, True)
        self.grabGesture(Qt.GestureType.PinchGesture)
        
        self.connection_overlay = ConnectionOverlay(self)
    
    def add_node(self, x, y):
        node = TaskNode(x, y, self)
        self.nodes.append(node)
        self.update_node_position(node)
        self.update()
        if self.main_window:
            self.main_window.schedule_autosave()
        return node
    
    def remove_node(self, node):
        self.connections = [(a, b) for a, b in self.connections if a != node and b != node]
        self.nodes.remove(node)
        node.deleteLater()
        self.update()
        if self.main_window:
            self.main_window.schedule_autosave()
    
    def update_node_position(self, node):
        screen_x = node.node_x * self.scale + self.offset.x()
        screen_y = node.node_y * self.scale + self.offset.y()
        node.move(int(screen_x), int(screen_y))
        node.update_scale(self.scale)
    
    def update_all_nodes(self):
        for node in self.nodes:
            self.update_node_position(node)
        self.connection_overlay.update()

    def start_connection(self, node):
        self.connecting_from = node
        self.setCursor(Qt.CursorShape.CrossCursor)
        self.line_dash_offset = 0
        self.line_animation_timer = QTimer(self)
        self.line_animation_timer.timeout.connect(self._animate_line)
        self.line_animation_timer.start(50)
    
    def _animate_line(self):
        self.line_dash_offset = (self.line_dash_offset + 2) % 20
        self.connection_overlay.update()
    
    def cancel_connection(self):
        if self.hover_target:
            self.hover_target.set_hover_target(False)
            self.hover_target = None
        self.connecting_from = None
        self.setCursor(Qt.CursorShape.ArrowCursor)
        if self.line_animation_timer:
            self.line_animation_timer.stop()
            self.line_animation_timer = None
        self.connection_overlay.update()
    
    def complete_connection(self, target_node):
        if self.connecting_from and self.connecting_from != target_node:
            if (self.connecting_from, target_node) not in self.connections:
                if (target_node, self.connecting_from) not in self.connections:
                    self.connections.append((self.connecting_from, target_node))
                    if self.main_window:
                        self.main_window.schedule_autosave()
        if self.hover_target:
            self.hover_target.set_hover_target(False)
            self.hover_target = None
        self.connecting_from = None
        self.setCursor(Qt.CursorShape.ArrowCursor)
        if self.line_animation_timer:
            self.line_animation_timer.stop()
            self.line_animation_timer = None
        self.update()
        self.connection_overlay.update()

    def resizeEvent(self, event):
        super().resizeEvent(event)
        self.connection_overlay.setGeometry(self.rect())
        self.connection_overlay.raise_()
    
    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        painter.fillRect(self.rect(), QColor(10, 10, 10))
        
        grid_size = int(50 * self.scale)
        if grid_size > 10:
            painter.setPen(QPen(QColor(30, 30, 30), 1))
            start_x = int(self.offset.x() % grid_size)
            start_y = int(self.offset.y() % grid_size)
            for x in range(start_x, self.width(), grid_size):
                painter.drawLine(x, 0, x, self.height())
            for y in range(start_y, self.height(), grid_size):
                painter.drawLine(0, y, self.width(), y)
        
        painter.end()
    
    def draw_connection(self, painter, node1, node2):
        start = QPointF(
            node1.x() + node1.width() / 2,
            node1.y() + node1.height() / 2
        )
        end = QPointF(
            node2.x() + node2.width() / 2,
            node2.y() + node2.height() / 2
        )
        
        color1 = QColor(STATUSES[node1.status]['color'])
        color2 = QColor(STATUSES[node2.status]['color'])
        
        gradient = QLinearGradient(start, end)
        gradient.setColorAt(0, color1)
        gradient.setColorAt(1, color2)
        
        painter.setPen(QPen(QBrush(gradient), 3))
        
        path = QPainterPath()
        path.moveTo(start)
        ctrl_offset = abs(end.x() - start.x()) / 2
        path.cubicTo(QPointF(start.x() + ctrl_offset, start.y()),
                     QPointF(end.x() - ctrl_offset, end.y()), end)
        painter.drawPath(path)

    def wheelEvent(self, event):
        pixel_delta = event.pixelDelta()
        angle_delta = event.angleDelta()
        modifiers = event.modifiers()
        
        if modifiers & (Qt.KeyboardModifier.ControlModifier | Qt.KeyboardModifier.MetaModifier):
            delta = angle_delta.y()
            if delta != 0:
                zoom_factor = 1.1 if delta > 0 else 0.9
                self._apply_zoom(zoom_factor, event.position())
            event.accept()
            return
        
        if pixel_delta.x() != 0 or pixel_delta.y() != 0:
            self.offset += QPointF(pixel_delta.x(), pixel_delta.y())
        elif angle_delta.x() != 0 or angle_delta.y() != 0:
            self.offset += QPointF(angle_delta.x() / 2, angle_delta.y() / 2)
        
        self.update_all_nodes()
        self.update()
        event.accept()
    
    def _apply_zoom(self, zoom_factor, mouse_pos):
        old_scale = self.scale
        self.scale *= zoom_factor
        self.scale = max(0.2, min(4.0, self.scale))
        
        if old_scale != self.scale:
            self.offset = mouse_pos - (mouse_pos - self.offset) * (self.scale / old_scale)
            self.update_all_nodes()
            self.update()
            if self.main_window and hasattr(self.main_window, 'update_zoom_label'):
                self.main_window.update_zoom_label()
    
    def zoom_in(self):
        center = QPointF(self.width() / 2, self.height() / 2)
        self._apply_zoom(1.2, center)
    
    def zoom_out(self):
        center = QPointF(self.width() / 2, self.height() / 2)
        self._apply_zoom(0.8, center)
    
    def zoom_reset(self):
        self.scale = 1.0
        self.offset = QPointF(0, 0)
        self.update_all_nodes()
        self.update()
        if self.main_window and hasattr(self.main_window, 'update_zoom_label'):
            self.main_window.update_zoom_label()
    
    def event(self, event):
        # Обработка pinch gesture через QGestureEvent
        if event.type() == event.Type.Gesture:
            return self.gestureEvent(event)
        elif event.type() == event.Type.NativeGesture:
            try:
                gesture_type = event.gestureType()
                if gesture_type == Qt.NativeGestureType.ZoomNativeGesture:
                    zoom_value = event.value()
                    if zoom_value != 0:
                        zoom_factor = 1.0 + zoom_value
                        self._apply_zoom(zoom_factor, event.position())
                    return True
            except Exception:
                pass
        return super().event(event)
    
    def gestureEvent(self, event):
        pinch = event.gesture(Qt.GestureType.PinchGesture)
        if pinch:
            change_flags = pinch.changeFlags()
            if change_flags & QPinchGesture.ChangeFlag.ScaleFactorChanged:
                scale_factor = pinch.scaleFactor()
                center = pinch.centerPoint()
                self._apply_zoom(scale_factor, center)
            return True
        return False

    def mousePressEvent(self, event):
        if event.button() == Qt.MouseButton.MiddleButton:
            self.panning = True
            self.pan_start = event.pos()
            self.setCursor(Qt.CursorShape.ClosedHandCursor)
        elif event.button() == Qt.MouseButton.LeftButton:
            if self.connecting_from:
                self.cancel_connection()
            elif event.modifiers() & Qt.KeyboardModifier.AltModifier:
                self.panning = True
                self.pan_start = event.pos()
                self.setCursor(Qt.CursorShape.ClosedHandCursor)
    
    def mouseDoubleClickEvent(self, event):
        if event.button() == Qt.MouseButton.LeftButton:
            canvas_x = (event.position().x() - self.offset.x()) / self.scale
            canvas_y = (event.position().y() - self.offset.y()) / self.scale
            self.add_node(canvas_x - 110, canvas_y - 70)
    
    def mouseMoveEvent(self, event):
        self.mouse_pos = event.position()
        if self.panning:
            delta = event.pos() - self.pan_start
            self.offset += QPointF(delta.x(), delta.y())
            self.pan_start = event.pos()
            self.update_all_nodes()
            self.update()
        elif self.connecting_from:
            self.connection_overlay.update()
    
    def mouseReleaseEvent(self, event):
        if event.button() in (Qt.MouseButton.MiddleButton, Qt.MouseButton.LeftButton):
            if self.panning:
                self.panning = False
                self.setCursor(Qt.CursorShape.ArrowCursor)

    def clear_all(self):
        for node in self.nodes[:]:
            node.deleteLater()
        self.nodes = []
        self.connections = []
        self.update()
        self.connection_overlay.update()
    
    def get_stats(self):
        stats = {k: 0 for k in STATUSES.keys()}
        for node in self.nodes:
            stats[node.status] += 1
        return stats

    def get_project_data(self):
        nodes_data = [node.get_data() for node in self.nodes]
        connections_data = []
        for n1, n2 in self.connections:
            connections_data.append([self.nodes.index(n1), self.nodes.index(n2)])
        
        return {
            'nodes': nodes_data,
            'connections': connections_data,
            'scale': self.scale,
            'offset_x': self.offset.x(),
            'offset_y': self.offset.y()
        }
    
    def load_project_data(self, data):
        self.clear_all()
        
        self.scale = data.get('scale', 1.0)
        self.offset = QPointF(data.get('offset_x', 0), data.get('offset_y', 0))
        
        for node_data in data.get('nodes', []):
            node = TaskNode(node_data['x'], node_data['y'], self)
            node.load_data(node_data)
            self.nodes.append(node)
            self.update_node_position(node)
        
        for conn in data.get('connections', []):
            if len(conn) == 2:
                idx1, idx2 = conn
                if 0 <= idx1 < len(self.nodes) and 0 <= idx2 < len(self.nodes):
                    self.connections.append((self.nodes[idx1], self.nodes[idx2]))
        
        self.update()
        self.connection_overlay.update()
