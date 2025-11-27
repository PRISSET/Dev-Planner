"""Базовые виджеты приложения"""
from PyQt6.QtWidgets import QFrame, QPushButton, QListWidget, QMenu
from PyQt6.QtCore import Qt
from PyQt6.QtGui import QFont


class TransparentPanel(QFrame):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("panel")
        self.setStyleSheet("""
            #panel {
                background-color: rgba(20, 20, 20, 220);
                border-radius: 15px;
                border: 1px solid rgba(255, 255, 255, 20);
            }
        """)


class ModernButton(QPushButton):
    def __init__(self, text, color="#00ffff", parent=None):
        super().__init__(text, parent)
        self.setCursor(Qt.CursorShape.PointingHandCursor)
        self.setFixedHeight(36)
        self.setFont(QFont(".AppleSystemUIFont", 9, QFont.Weight.Bold))
        self._color = color
        self.setStyleSheet(f"""
            QPushButton {{
                background-color: transparent;
                color: {color};
                border: 1px solid {color};
                border-radius: 8px;
                padding: 5px 12px;
            }}
            QPushButton:hover {{
                background-color: {color}30;
                color: #ffffff;
            }}
            QPushButton:pressed {{
                background-color: {color}50;
            }}
        """)


class ProjectListWidget(QListWidget):
    def __init__(self, main_window, parent=None):
        super().__init__(parent)
        self.main_window = main_window
        self.setStyleSheet("""
            QListWidget {
                background-color: transparent;
                border: none;
                color: #ffffff;
                font-size: 12px;
            }
            QListWidget::item {
                padding: 10px;
                border-radius: 8px;
                margin: 2px 0;
            }
            QListWidget::item:selected {
                background-color: rgba(0, 255, 255, 0.2);
                border: 1px solid #00ffff;
            }
            QListWidget::item:hover {
                background-color: rgba(255, 255, 255, 0.1);
            }
        """)
        self.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.customContextMenuRequested.connect(self.show_context_menu)
    
    def show_context_menu(self, pos):
        item = self.itemAt(pos)
        if item:
            menu = QMenu(self)
            menu.setStyleSheet("""
                QMenu {
                    background-color: #1a1a1a;
                    border: 1px solid #333333;
                    border-radius: 5px;
                    padding: 5px;
                }
                QMenu::item {
                    color: #ffffff;
                    padding: 8px 20px;
                    border-radius: 3px;
                }
                QMenu::item:selected {
                    background-color: #333333;
                }
            """)
            
            rename_action = menu.addAction("Переименовать")
            rename_action.triggered.connect(lambda: self.main_window.rename_project(item))
            
            delete_action = menu.addAction("Удалить")
            delete_action.triggered.connect(lambda: self.main_window.delete_project(item))
            
            menu.exec(self.mapToGlobal(pos))


class CollapsiblePanel(QFrame):
    """Сворачиваемая панель с плавной анимацией"""
    def __init__(self, title="", parent=None):
        super().__init__(parent)
        self.is_collapsed = False
        self.title = title
        self.content_widget = None
        self.expanded_width = 200
        self._animation = None
        
        self.setObjectName("collapsible_panel")
        self.setStyleSheet("""
            #collapsible_panel {
                background-color: rgba(20, 20, 20, 220);
                border-radius: 15px;
                border: 1px solid rgba(255, 255, 255, 20);
            }
        """)
        
        from PyQt6.QtWidgets import QVBoxLayout, QHBoxLayout, QLabel
        from PyQt6.QtCore import QPropertyAnimation, QEasingCurve
        
        self.main_layout = QVBoxLayout(self)
        self.main_layout.setContentsMargins(0, 0, 0, 0)
        self.main_layout.setSpacing(0)
        
        # Кнопка сворачивания
        self.toggle_btn = QPushButton("◀")
        self.toggle_btn.setFixedSize(28, 28)
        self.toggle_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        self.toggle_btn.setStyleSheet("""
            QPushButton {
                background: rgba(0,255,255,0.2);
                color: #00ffff;
                border: none;
                border-radius: 14px;
                font-size: 14px;
                font-weight: bold;
            }
            QPushButton:hover { background: rgba(0,255,255,0.4); }
        """)
        self.toggle_btn.clicked.connect(self.toggle_collapse)
        
        # Вертикальный заголовок (виден когда свёрнуто)
        self.title_label = QLabel(title)
        self.title_label.setStyleSheet("color: #00ffff; font-size: 10px; font-weight: bold;")
        self.title_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.title_label.hide()
        
        # Контейнер для контента
        self.content_container = QFrame()
        self.content_layout = QVBoxLayout(self.content_container)
        self.content_layout.setContentsMargins(10, 5, 10, 10)
        self.content_layout.setSpacing(8)
        
        # Header с кнопкой
        header = QHBoxLayout()
        header.setContentsMargins(6, 6, 6, 0)
        header.addWidget(self.title_label)
        header.addStretch()
        header.addWidget(self.toggle_btn)
        
        self.main_layout.addLayout(header)
        self.main_layout.addWidget(self.content_container)
    
    def set_content(self, widget):
        """Устанавливает содержимое панели"""
        self.content_widget = widget
        self.content_layout.addWidget(widget)
    
    def toggle_collapse(self):
        """Сворачивает/разворачивает панель с анимацией"""
        from PyQt6.QtCore import QPropertyAnimation, QEasingCurve
        
        self.is_collapsed = not self.is_collapsed
        
        if self.is_collapsed:
            self.expanded_width = self.width()
            self.content_container.hide()
            self.title_label.show()
            self.toggle_btn.setText("▶")
            
            # Анимация сворачивания
            self._animation = QPropertyAnimation(self, b"minimumWidth")
            self._animation.setDuration(200)
            self._animation.setStartValue(self.expanded_width)
            self._animation.setEndValue(44)
            self._animation.setEasingCurve(QEasingCurve.Type.OutCubic)
            self._animation.finished.connect(lambda: self.setFixedWidth(44))
            self._animation.start()
        else:
            self.setMinimumWidth(44)
            self.setMaximumWidth(16777215)
            
            # Анимация разворачивания
            self._animation = QPropertyAnimation(self, b"minimumWidth")
            self._animation.setDuration(200)
            self._animation.setStartValue(44)
            self._animation.setEndValue(self.expanded_width)
            self._animation.setEasingCurve(QEasingCurve.Type.OutCubic)
            self._animation.finished.connect(self._on_expand_finished)
            self._animation.start()
            
            self.content_container.show()
            self.title_label.hide()
            self.toggle_btn.setText("◀")
    
    def _on_expand_finished(self):
        self.setMinimumWidth(self.expanded_width - 40)
        self.setMaximumWidth(self.expanded_width + 80)
    
    def get_content_layout(self):
        return self.content_layout
