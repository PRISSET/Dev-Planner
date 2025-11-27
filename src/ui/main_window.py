from PyQt6.QtWidgets import (QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, 
                             QLabel, QListWidgetItem, QInputDialog, QMessageBox, 
                             QFileDialog, QSplitter, QScrollArea, QSizePolicy, QPushButton)
from PyQt6.QtCore import Qt, QTimer
from PyQt6.QtGui import QFont
from src.core.config import STATUSES
from src.core.storage import load_all_projects, save_all_projects
from src.ui.widgets import TransparentPanel, ModernButton, ProjectListWidget, CollapsiblePanel
from src.ui.canvas import NodeCanvas
from src.ui.ai_chat import AIChatPanel

APP_VERSION = "beta v1.0.1"

try:
    from src.core.collab import CollabClient, CollabThread
    COLLAB_AVAILABLE = True
except ImportError:
    COLLAB_AVAILABLE = False


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Dev Planner")
        self.setMinimumSize(800, 500)
        self.resize(1400, 900)
        
        self.projects = load_all_projects()
        self.current_project = None
        self.autosave_timer = QTimer()
        self.autosave_timer.setSingleShot(True)
        self.autosave_timer.timeout.connect(self.autosave)
        
        self.collab_client = None
        self.collab_code = None
        if COLLAB_AVAILABLE:
            self._setup_collab()
        
        self.setStyleSheet("QMainWindow { background-color: #0a0a0a; }")
        
        central_widget = QWidget()
        central_widget.setObjectName("central")
        central_widget.setStyleSheet("#central { background-color: #0a0a0a; }")
        self.setCentralWidget(central_widget)
        
        main_layout = QVBoxLayout(central_widget)
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)
        
        self._setup_title_bar(main_layout)
        self._setup_toolbar(main_layout)
        self._setup_content(main_layout)
        
        self.refresh_project_list()
        if self.project_list.count() > 0:
            self.project_list.setCurrentRow(0)
            self.on_project_selected(self.project_list.item(0))
    
    def _setup_title_bar(self, main_layout):
        title_bar = QWidget()
        title_bar.setFixedHeight(36)
        title_bar.setStyleSheet("background-color: #141414; border-bottom: 1px solid #333333;")
        title_layout = QHBoxLayout(title_bar)
        title_layout.setContentsMargins(12, 0, 12, 0)
        
        app_icon = QLabel()
        app_icon.setFixedSize(10, 10)
        app_icon.setStyleSheet("background-color: #00ff9d; border-radius: 5px;")

        app_title = QLabel("Dev Planner")
        app_title.setFont(QFont(".AppleSystemUIFont", 11, QFont.Weight.Bold))
        app_title.setStyleSheet("color: #ffffff;")
        
        hint_label = QLabel("Двойной клик — создать  |  ПКМ — меню  |  Тачпад: скролл/зум")
        hint_label.setStyleSheet("color: #555555; font-size: 9px;")
        
        title_layout.addWidget(app_icon)
        title_layout.addWidget(app_title)
        title_layout.addStretch()
        title_layout.addWidget(hint_label)
        
        main_layout.addWidget(title_bar)
    
    def _setup_toolbar(self, main_layout):
        toolbar = QWidget()
        toolbar.setFixedHeight(50)
        toolbar.setStyleSheet("background-color: #1a1a1a; border-bottom: 1px solid #333333;")
        toolbar_layout = QHBoxLayout(toolbar)
        toolbar_layout.setContentsMargins(12, 6, 12, 6)
        toolbar_layout.setSpacing(8)
        
        zoom_out_btn = ModernButton("−", "#00ffff")
        zoom_out_btn.setFixedSize(36, 36)
        zoom_out_btn.clicked.connect(lambda: self.canvas.zoom_out())
        
        self.zoom_label_btn = ModernButton("100%", "#888888")
        self.zoom_label_btn.setFixedSize(60, 36)
        self.zoom_label_btn.clicked.connect(lambda: self.canvas.zoom_reset())
        
        zoom_in_btn = ModernButton("+", "#00ffff")
        zoom_in_btn.setFixedSize(36, 36)
        zoom_in_btn.clicked.connect(lambda: self.canvas.zoom_in())
        
        toolbar_layout.addWidget(zoom_out_btn)
        toolbar_layout.addWidget(self.zoom_label_btn)
        toolbar_layout.addWidget(zoom_in_btn)
        
        sep1 = QLabel("|")
        sep1.setStyleSheet("color: #333333; font-size: 16px;")
        toolbar_layout.addWidget(sep1)
        
        export_btn = ModernButton("Экспорт ТЗ", "#bd00ff")
        export_btn.setFixedHeight(36)
        export_btn.clicked.connect(self.export_tz)
        toolbar_layout.addWidget(export_btn)
        
        clear_btn = ModernButton("Очистить", "#ff0055")
        clear_btn.setFixedHeight(36)
        clear_btn.clicked.connect(self.clear_all)
        toolbar_layout.addWidget(clear_btn)
        
        sep_collab = QLabel("|")
        sep_collab.setStyleSheet("color: #333333; font-size: 16px;")
        toolbar_layout.addWidget(sep_collab)
        
        self.collab_btn = ModernButton("Пригласить", "#00ff9d")
        self.collab_btn.setFixedHeight(36)
        self.collab_btn.clicked.connect(self._show_collab_menu)
        toolbar_layout.addWidget(self.collab_btn)
        
        self.collab_status = QLabel("")
        self.collab_status.setStyleSheet("color: #00ff9d; font-size: 10px;")
        toolbar_layout.addWidget(self.collab_status)
        
        toolbar_layout.addStretch()
        
        for key, value in STATUSES.items():
            indicator = QLabel()
            indicator.setFixedSize(8, 8)
            indicator.setStyleSheet(f"background-color: {value['color']}; border-radius: 4px;")
            name_label = QLabel(value['name'])
            name_label.setStyleSheet("color: #666666; font-size: 9px;")
            toolbar_layout.addWidget(indicator)
            toolbar_layout.addWidget(name_label)
            toolbar_layout.addSpacing(6)
        
        sep2 = QLabel("|")
        sep2.setStyleSheet("color: #333333; font-size: 16px;")
        toolbar_layout.addWidget(sep2)
        
        self.stats_labels = {}
        for key, value in STATUSES.items():
            count = QLabel("0")
            count.setStyleSheet(f"color: {value['color']}; font-size: 10px; font-weight: bold;")
            count.setFixedWidth(20)
            toolbar_layout.addWidget(count)
            self.stats_labels[key] = count
        
        main_layout.addWidget(toolbar)
    
    def _setup_content(self, main_layout):
        splitter = QSplitter(Qt.Orientation.Horizontal)
        splitter.setStyleSheet("""
            QSplitter::handle {
                background-color: #333333;
                width: 2px;
            }
            QSplitter::handle:hover {
                background-color: #00ffff;
            }
        """)
        
        self._setup_projects_panel(splitter)
        
        self.canvas = NodeCanvas()
        self.canvas.main_window = self
        self.canvas.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        splitter.addWidget(self.canvas)
        
        self._setup_ai_panel(splitter)
        
        splitter.setSizes([180, 700, 380])
        splitter.setStretchFactor(0, 0)
        splitter.setStretchFactor(1, 1)
        splitter.setStretchFactor(2, 0)
        
        main_layout.addWidget(splitter, 1)
        
        self._setup_version_label()
    
    def _setup_projects_panel(self, splitter):
        self.projects_panel = TransparentPanel()
        self.projects_panel.setMinimumWidth(150)
        self.projects_panel.setMaximumWidth(300)
        projects_layout = QVBoxLayout(self.projects_panel)
        projects_layout.setSpacing(8)
        projects_layout.setContentsMargins(10, 10, 10, 10)
        
        projects_label = QLabel("ПРОЕКТЫ")
        projects_label.setStyleSheet("color: #666666; font-weight: bold; letter-spacing: 2px; font-size: 9px;")
        projects_layout.addWidget(projects_label)
        
        new_project_btn = ModernButton("+ Новый проект", "#00ff9d")
        new_project_btn.clicked.connect(self.create_new_project)
        projects_layout.addWidget(new_project_btn)
        
        self.project_list = ProjectListWidget(self)
        self.project_list.itemClicked.connect(self.on_project_selected)
        projects_layout.addWidget(self.project_list, 1)
        
        splitter.addWidget(self.projects_panel)

    def _setup_version_label(self):
        self.version_label = QLabel(APP_VERSION)
        self.version_label.setStyleSheet("""
            color: #444444;
            font-size: 10px;
            padding: 4px 8px;
            background: transparent;
        """)
        self.version_label.setParent(self.centralWidget())
        self.version_label.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents)
        self.version_label.show()
        self._update_version_position()
    
    def resizeEvent(self, event):
        super().resizeEvent(event)
        self._update_version_position()
    
    def _update_version_position(self):
        if hasattr(self, 'version_label'):
            self.version_label.adjustSize()
            self.version_label.move(8, self.centralWidget().height() - self.version_label.height() - 4)

    def _setup_ai_panel(self, splitter):
        self.ai_panel = TransparentPanel()
        self.ai_panel.setMinimumWidth(280)
        self.ai_panel.setMaximumWidth(500)
        
        ai_layout = QVBoxLayout(self.ai_panel)
        ai_layout.setContentsMargins(0, 0, 0, 0)
        ai_layout.setSpacing(0)
        
        self.ai_chat = AIChatPanel()
        self.ai_chat.task_created.connect(self._on_ai_task_created)
        self.ai_chat.tasks_connect.connect(self._on_ai_connect_tasks)
        self.ai_chat.task_update_status.connect(self._on_ai_update_status)
        self.ai_chat.task_update_desc.connect(self._on_ai_update_desc)
        self.ai_chat.task_rename.connect(self._on_ai_rename_task)
        self.ai_chat.task_delete.connect(self._on_ai_delete_task)
        self.ai_chat.tasks_delete_many.connect(self._on_ai_delete_many)
        self.ai_chat.clear_all_tasks.connect(self._on_ai_clear_all)
        self.ai_chat.request_tasks.connect(self._on_ai_request_tasks)
        self.ai_chat.arrange_tasks.connect(self._on_ai_arrange)
        self.ai_chat.disconnect_tasks.connect(self._on_ai_disconnect)
        
        ai_layout.addWidget(self.ai_chat)
        splitter.addWidget(self.ai_panel)

    def refresh_project_list(self):
        self.project_list.clear()
        for name in self.projects.keys():
            item = QListWidgetItem(name)
            item.setData(Qt.ItemDataRole.UserRole, name)
            self.project_list.addItem(item)
    
    def create_new_project(self):
        name, ok = QInputDialog.getText(self, "Новый проект", "Название проекта:")
        if ok and name:
            if name in self.projects:
                QMessageBox.warning(self, "Ошибка", "Проект с таким именем уже существует")
                return
            
            self.projects[name] = {'nodes': [], 'connections': [], 'scale': 1.0, 'offset_x': 0, 'offset_y': 0}
            save_all_projects(self.projects)
            self.refresh_project_list()
            
            for i in range(self.project_list.count()):
                item = self.project_list.item(i)
                if item.data(Qt.ItemDataRole.UserRole) == name:
                    self.project_list.setCurrentItem(item)
                    self.on_project_selected(item)
                    break
    
    def on_project_selected(self, item):
        if self.current_project:
            self.save_current_project()
        
        name = item.data(Qt.ItemDataRole.UserRole)
        self.current_project = name
        
        if name in self.projects:
            self.canvas.load_project_data(self.projects[name])
        
       
        self.ai_chat.set_project(name)
        self.ai_chat.task_counter = len(self.canvas.nodes)
        
        self.update_stats()
    
    def save_current_project(self):
        if self.current_project:
            self.projects[self.current_project] = self.canvas.get_project_data()
            save_all_projects(self.projects)
    
    def rename_project(self, item):
        old_name = item.data(Qt.ItemDataRole.UserRole)
        new_name, ok = QInputDialog.getText(self, "Переименовать", "Новое название:", text=old_name)
        if ok and new_name and new_name != old_name:
            if new_name in self.projects:
                QMessageBox.warning(self, "Ошибка", "Проект с таким именем уже существует")
                return
            
            self.projects[new_name] = self.projects.pop(old_name)
            if self.current_project == old_name:
                self.current_project = new_name
            save_all_projects(self.projects)
            self.refresh_project_list()

    def delete_project(self, item):
        name = item.data(Qt.ItemDataRole.UserRole)
        reply = QMessageBox.question(self, "Удалить проект", 
                                     f"Удалить проект '{name}'?",
                                     QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No)
        if reply == QMessageBox.StandardButton.Yes:
            del self.projects[name]
            if self.current_project == name:
                self.current_project = None
                self.canvas.clear_all()
            save_all_projects(self.projects)
            self.refresh_project_list()
            
            if self.project_list.count() > 0:
                self.project_list.setCurrentRow(0)
                self.on_project_selected(self.project_list.item(0))
    
    def schedule_autosave(self):
        self.autosave_timer.start(1000)
    
    def autosave(self):
        self.save_current_project()
        self.update_stats()
    
    def update_stats(self):
        stats = self.canvas.get_stats()
        for key, label in self.stats_labels.items():
            label.setText(str(stats[key]))
    
    def update_zoom_label(self):
        """Обновить отображение процента зума"""
        percent = int(self.canvas.scale * 100)
        self.zoom_label_btn.setText(f"{percent}%")
    
    def _on_ai_task_created(self, title, description, status="none", x=0, y=0):
        node = self.canvas.add_node(x, y)
        node.title_edit.setText(title)
        node.desc_edit.setPlainText(description)
        if status in STATUSES:
            node.set_status(status)
        self.update_stats()
    
    def _on_ai_rename_task(self, task_idx, title):
        nodes = self.canvas.nodes
        if 0 <= task_idx < len(nodes):
            nodes[task_idx].title_edit.setText(title)
    
    def _on_ai_delete_task(self, task_idx):
        nodes = self.canvas.nodes
        if 0 <= task_idx < len(nodes):
            self.canvas.remove_node(nodes[task_idx])
            self.update_stats()
    
    def _on_ai_delete_many(self, tasks):
        nodes = self.canvas.nodes
        if tasks and isinstance(tasks[0], str):
            # Удаление по статусу
            status = tasks[0]
            to_delete = [n for n in nodes if n.status == status]
            for node in to_delete:
                self.canvas.remove_node(node)
        else:
            
            for idx in sorted(tasks, reverse=True):
                if 0 <= idx < len(nodes):
                    self.canvas.remove_node(nodes[idx])
        self.update_stats()
    
    def _on_ai_clear_all(self):
        self.canvas.clear_all()
        self.update_stats()
    
    def _on_ai_disconnect(self, from_idx, to_idx):
        nodes = self.canvas.nodes
        if 0 <= from_idx < len(nodes) and 0 <= to_idx < len(nodes):
            conn = (nodes[from_idx], nodes[to_idx])
            conn_rev = (nodes[to_idx], nodes[from_idx])
            if conn in self.canvas.connections:
                self.canvas.connections.remove(conn)
            elif conn_rev in self.canvas.connections:
                self.canvas.connections.remove(conn_rev)
            self.canvas.connection_overlay.update()
    
    def _on_ai_arrange(self, arrange_type):
        import random
        import math
        nodes = self.canvas.nodes
        if not nodes:
            return
        
        n = len(nodes)
        card_w, card_h = 220, 140
        gap_x, gap_y = 40, 30
        
        if arrange_type == 'grid':
            cols = max(1, int(math.ceil(math.sqrt(n))))
            for i, node in enumerate(nodes):
                col = i % cols
                row = i // cols
                node.node_x = 50 + col * (card_w + gap_x)
                node.node_y = 50 + row * (card_h + gap_y)
                self.canvas.update_node_position(node)
        
        elif arrange_type == 'horizontal':
            total_w = n * card_w + (n - 1) * gap_x
            start_x = max(50, (800 - total_w) // 2)
            for i, node in enumerate(nodes):
                node.node_x = start_x + i * (card_w + gap_x)
                node.node_y = 250
                self.canvas.update_node_position(node)
        
        elif arrange_type == 'vertical':
            for i, node in enumerate(nodes):
                node.node_x = 350
                node.node_y = 50 + i * (card_h + gap_y)
                self.canvas.update_node_position(node)
        
        elif arrange_type == 'random':
            positions = []
            for node in nodes:
                for _ in range(50):  # Попытки найти место
                    x = random.randint(50, 700)
                    y = random.randint(50, 500)
                    ok = True
                    for px, py in positions:
                        if abs(x - px) < card_w + 20 and abs(y - py) < card_h + 20:
                            ok = False
                            break
                    if ok:
                        break
                positions.append((x, y))
                node.node_x = x
                node.node_y = y
                self.canvas.update_node_position(node)
        
        elif arrange_type == 'circle':
            cx, cy = 450, 350
            radius = max(150, n * 30)
            for i, node in enumerate(nodes):
                angle = 2 * math.pi * i / n - math.pi / 2
                node.node_x = cx + radius * math.cos(angle) - card_w / 2
                node.node_y = cy + radius * math.sin(angle) - card_h / 2
                self.canvas.update_node_position(node)
        
        elif arrange_type == 'tree':
            has_incoming = set()
            for n1, n2 in self.canvas.connections:
                has_incoming.add(n2)
            
            roots = [node for node in nodes if node not in has_incoming]
            if not roots:
                roots = [nodes[0]] if nodes else []
            
            visited = set()
            levels = {}
            
            def assign_level(node, level):
                if node in visited:
                    return
                visited.add(node)
                if level not in levels:
                    levels[level] = []
                levels[level].append(node)
                for n1, n2 in self.canvas.connections:
                    if n1 == node:
                        assign_level(n2, level + 1)
            
            for root in roots:
                assign_level(root, 0)
            
            for node in nodes:
                if node not in visited:
                    max_level = max(levels.keys()) + 1 if levels else 0
                    if max_level not in levels:
                        levels[max_level] = []
                    levels[max_level].append(node)
            
            for level, level_nodes in levels.items():
                count = len(level_nodes)
                total_w = count * card_w + (count - 1) * gap_x
                start_x = max(50, (900 - total_w) // 2)
                for i, node in enumerate(level_nodes):
                    node.node_x = start_x + i * (card_w + gap_x)
                    node.node_y = 50 + level * (card_h + gap_y + 20)
                    self.canvas.update_node_position(node)
        
        elif arrange_type == 'cascade':
            for i, node in enumerate(nodes):
                node.node_x = 50 + i * 60
                node.node_y = 50 + i * 50
                self.canvas.update_node_position(node)
        
        self.canvas.update()
        self.canvas.connection_overlay.update()
    
    def _on_ai_connect_tasks(self, from_idx, to_idx):
        nodes = self.canvas.nodes
        if 0 <= from_idx < len(nodes) and 0 <= to_idx < len(nodes):
            from_node = nodes[from_idx]
            to_node = nodes[to_idx]
            if (from_node, to_node) not in self.canvas.connections:
                if (to_node, from_node) not in self.canvas.connections:
                    self.canvas.connections.append((from_node, to_node))
                    self.canvas.connection_overlay.update()
    
    def _on_ai_update_status(self, task_idx, status):
        nodes = self.canvas.nodes
        if 0 <= task_idx < len(nodes) and status in STATUSES:
            nodes[task_idx].set_status(status)
            self.update_stats()
    
    def _on_ai_update_desc(self, task_idx, description):
        nodes = self.canvas.nodes
        if 0 <= task_idx < len(nodes):
            nodes[task_idx].desc_edit.setPlainText(description)
    
    def _on_ai_request_tasks(self):
        nodes = self.canvas.nodes
        if not nodes:
            self.ai_chat.set_tasks_info("Задач пока нет")
            return
        
        info = ""
        for i, node in enumerate(nodes, 1):
            data = node.get_data()
            status_name = STATUSES[data['status']]['name']
            info += f"{i}. {data['title']} [{status_name}]\n"
            if data['description']:
                info += f"   {data['description'][:50]}...\n"
        
        self.ai_chat.set_tasks_info(info)
    
    def clear_all(self):
        if self.current_project:
            reply = QMessageBox.question(self, "Очистить", 
                                         "Очистить все задачи в текущем проекте?",
                                         QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No)
            if reply == QMessageBox.StandardButton.Yes:
                self.canvas.clear_all()
                self.save_current_project()
                self.update_stats()
    
    def export_tz(self):
        if not self.canvas.nodes:
            QMessageBox.warning(self, "Ошибка", "Нет задач для экспорта")
            return
            
        text = f"# Техническое Задание: {self.current_project or 'Без названия'}\n\n"
        for i, node in enumerate(self.canvas.nodes, 1):
            data = node.get_data()
            status_name = STATUSES[data['status']]['name']
            title = data['title'] if data['title'] else "Без названия"
            text += f"## {i}. {title} [{status_name}]\n{data['description']}\n\n"
            
        file_path, _ = QFileDialog.getSaveFileName(self, "Сохранить ТЗ", "", "Markdown Files (*.md);;Text Files (*.txt)")
        if file_path:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(text)
            QMessageBox.information(self, "Успех", "ТЗ успешно сохранено!")
    
    def closeEvent(self, event):
        self.save_current_project()
        if self.collab_client:
            self.collab_client.disconnect_from_server()
        event.accept()
    
    def _setup_collab(self):
        self.collab_client = CollabClient()
        self.collab_client.connected.connect(self._on_collab_connected)
        self.collab_client.disconnected.connect(self._on_collab_disconnected)
        self.collab_client.room_created.connect(self._on_room_created)
        self.collab_client.room_joined.connect(self._on_room_joined)
        self.collab_client.join_failed.connect(self._on_join_failed)
        self.collab_client.members_updated.connect(self._on_members_updated)
        self.collab_client.project_updated.connect(self._on_project_updated)
        self.collab_client.task_action_received.connect(self._on_task_action_received)
        self.collab_client.cursor_updated.connect(self._on_cursor_updated)
        self.collab_client.user_left.connect(self._on_user_left)
        self.collab_client.room_closed.connect(self._on_room_closed)
        self.collab_client.error_occurred.connect(self._on_collab_error)
    
    def _show_collab_menu(self):
        from PyQt6.QtWidgets import QMenu
        menu = QMenu(self)
        menu.setStyleSheet("""
            QMenu { background-color: #1a1a1a; border: 1px solid #333333; border-radius: 5px; padding: 5px; }
            QMenu::item { color: #ffffff; padding: 8px 20px; border-radius: 3px; }
            QMenu::item:selected { background-color: #333333; }
        """)
        
        if self.collab_client and self.collab_client.in_room:
            if self.collab_code:
                code_action = menu.addAction(f"Код: {self.collab_code}")
                code_action.setEnabled(False)
            menu.addSeparator()
            leave_action = menu.addAction("Покинуть комнату")
            leave_action.triggered.connect(self._leave_collab_room)
            if self.collab_client.is_host:
                menu.addSeparator()
                close_action = menu.addAction("Закрыть комнату")
                close_action.triggered.connect(self._close_collab_room)
        else:
            create_action = menu.addAction("Создать комнату")
            create_action.triggered.connect(self._create_collab_room)
            join_action = menu.addAction("Присоединиться")
            join_action.triggered.connect(self._join_collab_room)
        
        menu.exec(self.collab_btn.mapToGlobal(self.collab_btn.rect().bottomLeft()))
    
    def _create_collab_room(self):
        if not COLLAB_AVAILABLE:
            QMessageBox.warning(self, "Ошибка", "Модуль коллаборации недоступен")
            return
        
        name, ok = QInputDialog.getText(self, "Создать комнату", "Ваше имя:")
        if ok and name:
            project_data = self.canvas.get_project_data()
            self.collab_thread = CollabThread(self.collab_client, 'create_room', 
                                              name=name, project_data=project_data)
            self.collab_thread.start()
    
    def _join_collab_room(self):
        if not COLLAB_AVAILABLE:
            QMessageBox.warning(self, "Ошибка", "Модуль коллаборации недоступен")
            return
        
        code, ok = QInputDialog.getText(self, "Присоединиться", "Введите код комнаты:")
        if ok and code:
            name, ok2 = QInputDialog.getText(self, "Присоединиться", "Ваше имя:")
            if ok2 and name:
                self.collab_thread = CollabThread(self.collab_client, 'join_room',
                                                  code=code, name=name)
                self.collab_thread.start()
    
    def _leave_collab_room(self):
        if self.collab_client:
            self.collab_thread = CollabThread(self.collab_client, 'leave_room')
            self.collab_thread.start()
            self.collab_code = None
            self.collab_status.setText("")
            self.collab_btn.setText("Пригласить")
            self.canvas.clear_remote_cursors()
    
    def _on_collab_connected(self):
        pass
    
    def _on_collab_disconnected(self):
        self.collab_code = None
        self.collab_status.setText("")
        self.collab_btn.setText("Пригласить")
    
    def _on_room_created(self, room_id, code):
        self.collab_code = code
        self.collab_status.setText(f"● Комната: {code}")
        self.collab_btn.setText("Комната")
        QMessageBox.information(self, "Комната создана", 
                               f"Код для приглашения:\n\n{code}\n\nПоделитесь этим кодом с другими участниками")
    
    def _on_room_joined(self, room_id, project_data, members):
        self.collab_code = None
        self.collab_status.setText(f"● Подключено ({len(members)})")
        self.collab_btn.setText("Комната")
        if project_data:
            self.canvas.load_project_data(project_data)
            self.update_stats()
    
    def _on_join_failed(self, message):
        QMessageBox.warning(self, "Ошибка", f"Не удалось присоединиться: {message}")
    
    def _on_members_updated(self, members):
        self.collab_status.setText(f"● Подключено ({len(members)})")
    
    def _on_project_updated(self, project_data):
        if project_data:
            self.canvas.load_project_data(project_data)
            self.update_stats()
    
    def _on_cursor_updated(self, user_id, x, y, name):
        self.canvas.update_remote_cursor(user_id, x, y, name)
    
    def _on_user_left(self, user_id):
        self.canvas.remove_remote_cursor(user_id)
    
    def _on_task_action_received(self, action, payload, from_id):
        if action == 'create_task':
            node = self.canvas.add_node_silent(payload.get('x', 100), payload.get('y', 100))
            node.title_edit.setText(payload.get('title', 'Задача'))
            node.desc_edit.setPlainText(payload.get('description', ''))
            if payload.get('status') in STATUSES:
                node.set_status(payload.get('status'))
        elif action == 'delete_task':
            idx = payload.get('index', -1)
            if 0 <= idx < len(self.canvas.nodes):
                self.canvas.remove_node_silent(self.canvas.nodes[idx])
        elif action == 'update_task':
            idx = payload.get('index', -1)
            if 0 <= idx < len(self.canvas.nodes):
                node = self.canvas.nodes[idx]
                if 'title' in payload:
                    node.title_edit.setText(payload['title'])
                if 'description' in payload:
                    node.desc_edit.setPlainText(payload['description'])
                if 'status' in payload and payload['status'] in STATUSES:
                    node.set_status(payload['status'])
                if 'x' in payload and 'y' in payload:
                    node.node_x = payload['x']
                    node.node_y = payload['y']
                    self.canvas.update_node_position(node)
        elif action == 'connect_tasks':
            from_idx = payload.get('from', -1)
            to_idx = payload.get('to', -1)
            if 0 <= from_idx < len(self.canvas.nodes) and 0 <= to_idx < len(self.canvas.nodes):
                from_node = self.canvas.nodes[from_idx]
                to_node = self.canvas.nodes[to_idx]es[to_idx]
                if (from_node, to_node) not in self.canvas.connections:
                    self.canvas.connections.append((from_node, to_node))
                    self.canvas.connection_overlay.update()
        elif action == 'full_sync':
            if payload.get('projectData'):
                self.canvas.load_project_data(payload['projectData'])
        self.update_stats()
    
    def _on_room_closed(self, message):
        self.collab_code = None
        self.collab_status.setText("")
        self.collab_btn.setText("Пригласить")
        self.canvas.clear_remote_cursors()
        QMessageBox.information(self, "Комната закрыта", message)
    
    def _close_collab_room(self):
        if self.collab_client and self.collab_client.is_host:
            reply = QMessageBox.question(self, "Закрыть комнату", 
                                         "Закрыть комнату для всех участников?",
                                         QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No)
            if reply == QMessageBox.StandardButton.Yes:
                self.collab_thread = CollabThread(self.collab_client, 'close_room')
                self.collab_thread.start()
                self.collab_code = None
                self.collab_status.setText("")
                self.collab_btn.setText("Пригласить")
    
    def _on_collab_error(self, message):
        QMessageBox.warning(self, "Ошибка подключения", message)
