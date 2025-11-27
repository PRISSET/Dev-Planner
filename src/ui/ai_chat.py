import json
import os
import requests
from PyQt6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QTextEdit, 
                             QLineEdit, QPushButton, QLabel, QScrollArea, QFrame)
from PyQt6.QtCore import Qt, QThread, pyqtSignal, QTimer
from PyQt6.QtGui import QFont

OPENROUTER_API_URL = "https://openrouter.ai/api/v1/chat/completions"
MODEL = "openai/gpt-4o-mini"
DEFAULT_API_KEY = "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"
CONTEXT_DIR = os.path.expanduser("~/.devchain_planner/contexts")

SYSTEM_PROMPT = """Ты — AI-исполнитель Dev Planner. Твоя задача — НЕМЕДЛЕННО выполнять команды пользователя.

ГЛАВНОЕ ПРАВИЛО: ВСЕГДА отвечай ТОЛЬКО JSON-командой! Никакого текста кроме JSON!

ЦВЕТА ЗАДАЧ (ВАЖНО!):
- "жёлтый/желтый/yellow" = статус "progress"
- "красный/red" = статус "todo"  
- "зелёный/зеленый/green" = статус "done"
- "серый/grey" = статус "none"
- "отменён" = статус "cancelled"

СОЗДАНИЕ:
{"action": "create_task", "title": "Короткое название", "description": "Краткое описание", "status": "todo"}
{"action": "create_tasks_chain", "tasks": [{"title": "...", "description": "...", "status": "progress"}], "connect": true}

СТАТУСЫ:
{"action": "set_status", "task": 1, "status": "progress"}
{"action": "set_many_status", "tasks": [1, 2, 3], "status": "progress"}

СОЕДИНЕНИЯ:
{"action": "connect", "from": 1, "to": 2}
{"action": "connect_many", "connections": [[1,2], [2,3]]}
{"action": "disconnect", "from": 1, "to": 2}

РЕДАКТИРОВАНИЕ:
{"action": "rename", "task": 1, "title": "..."}
{"action": "set_description", "task": 1, "description": "..."}

УДАЛЕНИЕ:
{"action": "delete", "task": 1}
{"action": "delete_many", "tasks": [1, 2]}
{"action": "clear_all"}

РАСПОЛОЖЕНИЕ:
{"action": "arrange_grid"}
{"action": "arrange_tree"}

ИНФОРМАЦИЯ:
{"action": "get_tasks"}

НЕСКОЛЬКО ДЕЙСТВИЙ:
{"actions": [{"action": "...", ...}, {"action": "...", ...}]}

КРИТИЧЕСКИЕ ПРАВИЛА:
1. ОТВЕЧАЙ ТОЛЬКО JSON! Никакого текста до или после!
2. "Сделай жёлтым/желтым" = set_status с "progress"
3. "Сделай зелёным" = set_status с "done"
4. "Сделай красным" = set_status с "todo"
5. При создании нескольких задач — РАСПРЕДЕЛИ информацию РАВНОМЕРНО между ними!
6. Каждая задача должна иметь КОРОТКИЙ title (3-5 слов) и КРАТКОЕ description (1-2 предложения)
7. НЕ пиши всю информацию в одну задачу — раздели на несколько!
8. Если просят N задач — создай РОВНО N задач с равными частями информации
9. Если несколько действий нужно — используй {"actions": [...]}"""


def ensure_context_dir():
    if not os.path.exists(CONTEXT_DIR):
        os.makedirs(CONTEXT_DIR)

def load_context(project_name):
    ensure_context_dir()
    path = os.path.join(CONTEXT_DIR, f"{project_name}.json")
    if os.path.exists(path):
        try:
            with open(path, 'r', encoding='utf-8') as f:
                return json.load(f)
        except:
            pass
    return []

def save_context(project_name, messages):
    ensure_context_dir()
    path = os.path.join(CONTEXT_DIR, f"{project_name}.json")
    to_save = messages[-100:] if len(messages) > 100 else messages
    with open(path, 'w', encoding='utf-8') as f:
        json.dump(to_save, f, ensure_ascii=False, indent=2)


class AIWorker(QThread):
    response_ready = pyqtSignal(str)
    error_occurred = pyqtSignal(str)
    
    def __init__(self, api_key, messages):
        super().__init__()
        self.api_key = api_key
        self.messages = messages
    
    def run(self):
        try:
            headers = {
                "Authorization": f"Bearer {self.api_key}",
                "Content-Type": "application/json",
                "HTTP-Referer": "https://dev-planner.local",
                "X-Title": "Dev Planner"
            }
            data = {
                "model": MODEL,
                "messages": self.messages,
                "max_tokens": 2000
            }
            response = requests.post(OPENROUTER_API_URL, headers=headers, json=data, timeout=60)
            if response.status_code == 200:
                result = response.json()
                content = result['choices'][0]['message']['content']
                self.response_ready.emit(content)
            else:
                self.error_occurred.emit(f"Ошибка API: {response.status_code}")
        except Exception as e:
            self.error_occurred.emit(f"Ошибка: {str(e)}")


class ChatMessage(QFrame):
    def __init__(self, text, is_user=True, parent=None):
        super().__init__(parent)
        
        if is_user:
            bg_color = "rgba(0, 255, 255, 0.15)"
            border_color = "rgba(0, 255, 255, 0.3)"
            label_color = "#00ffff"
            label_text = "Вы"
        else:
            bg_color = "rgba(0, 255, 157, 0.1)"
            border_color = "rgba(0, 255, 157, 0.2)"
            label_color = "#00ff9d"
            label_text = "AI"
        
        self.setStyleSheet(f"""
            ChatMessage {{
                background-color: {bg_color};
                border: 1px solid {border_color};
                border-radius: 12px;
            }}
            ChatMessage QLabel {{
                background: transparent;
                border: none;
            }}
        """)
        self.setObjectName("chatMsg")
        
        layout = QVBoxLayout(self)
        layout.setContentsMargins(12, 10, 12, 10)
        layout.setSpacing(6)
        
        label = QLabel(label_text)
        label.setStyleSheet(f"background: transparent; border: none; color: {label_color}; font-size: 10px; font-weight: bold;")
        
        message = QLabel(text)
        message.setWordWrap(True)
        message.setStyleSheet("background: transparent; border: none; color: #ffffff; font-size: 13px;")
        message.setTextInteractionFlags(Qt.TextInteractionFlag.TextSelectableByMouse)
        message.setMinimumWidth(50)
        message.setMaximumWidth(350)
        
        layout.addWidget(label)
        layout.addWidget(message)


class AIChatPanel(QWidget):
    
    task_created = pyqtSignal(str, str, str, int, int)  # title, desc, status, x, y
    tasks_connect = pyqtSignal(int, int)
    task_update_status = pyqtSignal(int, str)
    task_update_desc = pyqtSignal(int, str)
    task_rename = pyqtSignal(int, str)
    task_delete = pyqtSignal(int)
    tasks_delete_many = pyqtSignal(list)
    clear_all_tasks = pyqtSignal()
    request_tasks = pyqtSignal()
    arrange_tasks = pyqtSignal(str)  # grid, tree, horizontal, vertical
    disconnect_tasks = pyqtSignal(int, int)
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self.api_key = DEFAULT_API_KEY
        self.messages = [{"role": "system", "content": SYSTEM_PROMPT}]
        self.worker = None
        self.current_project = None
        self.task_counter = 0  # Для расположения задач
        
        self._setup_ui()
    
    def set_project(self, project_name):
        if self.current_project and self.current_project != project_name:
            save_context(self.current_project, self.messages[1:])
        
        self.current_project = project_name
        self.task_counter = 0
        
        saved = load_context(project_name)
        self.messages = [{"role": "system", "content": SYSTEM_PROMPT}]
        if saved:
            self.messages.extend(saved)
        
        self._clear_chat_ui()
        
        for msg in self.messages[1:]:
            if msg['role'] == 'user':
                self._add_message_ui(msg['content'], is_user=True)
            elif msg['role'] == 'assistant':
                display_text = self._format_ai_message(msg['content'])
                self._add_message_ui(display_text, is_user=False)
    
    def _clear_chat_ui(self):
        while self.messages_layout.count():
            item = self.messages_layout.takeAt(0)
            if item.widget():
                item.widget().deleteLater()
    
    def _clear_chat(self):
        self._clear_chat_ui()
        self.messages = [{"role": "system", "content": SYSTEM_PROMPT}]
        if self.current_project:
            save_context(self.current_project, [])
    
    def _setup_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(12, 12, 12, 12)
        layout.setSpacing(10)
        
        
        header_layout = QHBoxLayout()
        header = QLabel("AI АССИСТЕНТ")
        header.setStyleSheet("color: #00ffff; font-weight: bold; letter-spacing: 2px; font-size: 11px;")
        header_layout.addWidget(header)
        header_layout.addStretch()
        
        clear_chat_btn = QPushButton("🗑")
        clear_chat_btn.setFixedSize(24, 24)
        clear_chat_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        clear_chat_btn.setToolTip("Очистить чат")
        clear_chat_btn.setStyleSheet("""
            QPushButton {
                background: rgba(255, 0, 85, 0.2);
                color: #ff0055;
                border: 1px solid rgba(255, 0, 85, 0.3);
                border-radius: 12px;
                font-size: 12px;
            }
            QPushButton:hover {
                background: rgba(255, 0, 85, 0.4);
            }
        """)
        clear_chat_btn.clicked.connect(self._clear_chat)
        header_layout.addWidget(clear_chat_btn)
        header_layout.addSpacing(8)
        
        self.status_label = QLabel("● Онлайн")
        self.status_label.setStyleSheet("color: #00ff9d; font-size: 10px;")
        header_layout.addWidget(self.status_label)
        layout.addLayout(header_layout)
        
        sep = QFrame()
        sep.setFixedHeight(1)
        sep.setStyleSheet("background-color: #333333;")
        layout.addWidget(sep)
        
        self.scroll_area = QScrollArea()
        self.scroll_area.setWidgetResizable(True)
        self.scroll_area.setStyleSheet("""
            QScrollArea { 
                background: rgba(0,0,0,0.2); 
                border: none; 
                border-radius: 8px;
            }
            QScrollBar:vertical { 
                background: #1a1a1a; 
                width: 8px; 
                border-radius: 4px;
            }
            QScrollBar::handle:vertical { 
                background: #444444; 
                border-radius: 4px;
                min-height: 30px;
            }
            QScrollBar::handle:vertical:hover { 
                background: #00ffff; 
            }
        """)
        
        self.messages_widget = QWidget()
        self.messages_widget.setStyleSheet("background: transparent;")
        self.messages_layout = QVBoxLayout(self.messages_widget)
        self.messages_layout.setAlignment(Qt.AlignmentFlag.AlignTop)
        self.messages_layout.setSpacing(8)
        self.messages_layout.setContentsMargins(8, 8, 8, 8)
        self.scroll_area.setWidget(self.messages_widget)
        layout.addWidget(self.scroll_area, 1)

        input_container = QFrame()
        input_container.setStyleSheet("""
            QFrame {
                background: rgba(0,0,0,0.3);
                border: 1px solid #333333;
                border-radius: 8px;
            }
        """)
        input_layout = QHBoxLayout(input_container)
        input_layout.setContentsMargins(8, 6, 6, 6)
        input_layout.setSpacing(8)
        
        self.input_field = QLineEdit()
        self.input_field.setPlaceholderText("Напишите сообщение...")
        self.input_field.setStyleSheet("""
            QLineEdit {
                background: transparent;
                border: none;
                color: #ffffff;
                padding: 6px;
                font-size: 13px;
            }
        """)
        self.input_field.returnPressed.connect(self._send_message)
        
        self.send_btn = QPushButton("➤")
        self.send_btn.setFixedSize(40, 40)
        self.send_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        self.send_btn.setStyleSheet("""
            QPushButton {
                background: #00ffff;
                color: #000000;
                border: none;
                border-radius: 20px;
                font-size: 16px;
                font-weight: bold;
            }
            QPushButton:hover { background: #00cccc; }
            QPushButton:disabled { background: #333333; color: #666666; }
        """)
        self.send_btn.clicked.connect(self._send_message)
        
        input_layout.addWidget(self.input_field)
        input_layout.addWidget(self.send_btn)
        layout.addWidget(input_container)
        
        hints_layout = QHBoxLayout()
        hints = ["Создай задачу", "Покажи задачи", "Расположи сеткой"]
        for hint_text in hints:
            hint_btn = QPushButton(hint_text)
            hint_btn.setCursor(Qt.CursorShape.PointingHandCursor)
            hint_btn.setStyleSheet("""
                QPushButton {
                    background: rgba(0,255,255,0.1);
                    color: #00ffff;
                    border: 1px solid rgba(0,255,255,0.3);
                    border-radius: 12px;
                    padding: 4px 8px;
                    font-size: 9px;
                }
                QPushButton:hover {
                    background: rgba(0,255,255,0.2);
                }
            """)
            hint_btn.clicked.connect(lambda checked, t=hint_text: self._quick_action(t))
            hints_layout.addWidget(hint_btn)
        hints_layout.addStretch()
        layout.addLayout(hints_layout)
    
    def _quick_action(self, text):
        actions = {
            "Создай задачу": "Создай новую задачу",
            "Покажи задачи": "Покажи все задачи",
            "Расположи сеткой": "Расположи задачи сеткой"
        }
        self.input_field.setText(actions.get(text, text))
        self._send_message()

    def _send_message(self):
        text = self.input_field.text().strip()
        if not text or not self.api_key:
            return
        
        self._add_message_ui(text, is_user=True)
        self.messages.append({"role": "user", "content": text})
        self.input_field.clear()
        
        self.input_field.setEnabled(False)
        self.send_btn.setEnabled(False)
        
        self.worker = AIWorker(self.api_key, self.messages.copy())
        self.worker.response_ready.connect(self._on_response)
        self.worker.error_occurred.connect(self._on_error)
        self.worker.start()
    
    def _add_message_ui(self, text, is_user=True):
        msg = ChatMessage(text, is_user)
        self.messages_layout.addWidget(msg)
        QTimer.singleShot(50, self._scroll_to_bottom)
    
    def _scroll_to_bottom(self):
        self.scroll_area.verticalScrollBar().setValue(
            self.scroll_area.verticalScrollBar().maximum()
        )

    def _on_response(self, content):
        self.input_field.setEnabled(True)
        self.send_btn.setEnabled(True)
        
        self.messages.append({"role": "assistant", "content": content})
        
        if self.current_project:
            save_context(self.current_project, self.messages[1:])
        
        self._process_ai_response(content)
    
    def _get_task_position(self):
        col = self.task_counter % 3
        row = self.task_counter // 3
        x = 50 + col * 250
        y = 50 + row * 180
        self.task_counter += 1
        return x, y
    
    def _process_ai_response(self, content):
        actions_done = []
        
        json_objects = self._extract_all_json(content)
        
        for data in json_objects:
            try:
                if 'actions' in data:
                    for action_data in data['actions']:
                        result = self._execute_action(action_data)
                        if result:
                            actions_done.append(result)
                else:
                    result = self._execute_action(data)
                    if result:
                        actions_done.append(result)
            except Exception:
                pass
        
        if actions_done:
            display_text = "✓ " + "\n✓ ".join(actions_done)
        else:
            display_text = self._clean_json_from_text(content)
        
        self._add_message_ui(display_text, is_user=False)
    
    def _clean_json_from_text(self, text):
        import re
        cleaned = re.sub(r'\{[^{}]*\}', '', text)
        while '{' in cleaned and '}' in cleaned:
            cleaned = re.sub(r'\{[^{}]*\}', '', cleaned)
        cleaned = re.sub(r'\n\s*\n', '\n', cleaned).strip()
        return cleaned if cleaned else "Готово"
    
    def _format_ai_message(self, content):
        json_objects = self._extract_all_json(content)
        
        if not json_objects:
            return content
        
        descriptions = []
        for data in json_objects:
            if 'actions' in data:
                for action_data in data['actions']:
                    desc = self._describe_action(action_data)
                    if desc:
                        descriptions.append(desc)
            else:
                desc = self._describe_action(data)
                if desc:
                    descriptions.append(desc)
        
        if descriptions:
            return "✓ " + "\n✓ ".join(descriptions)
        
        return self._clean_json_from_text(content)
    
    def _describe_action(self, data):
        action = data.get('action')
        if not action:
            return None
        
        if action == 'create_task':
            return f"Создана: {data.get('title', 'Задача')}"
        elif action == 'create_tasks_chain':
            return f"Создано {len(data.get('tasks', []))} задач"
        elif action == 'connect':
            return f"Соединено: {data.get('from')} → {data.get('to')}"
        elif action == 'set_status':
            return f"Статус задачи {data.get('task')} → {data.get('status')}"
        elif action == 'set_many_status':
            return f"Статус задач {data.get('tasks')} → {data.get('status')}"
        elif action == 'set_description':
            return f"Описание: задача {data.get('task')}"
        elif action == 'rename':
            return f"Переименовано: задача {data.get('task')}"
        elif action == 'delete':
            return f"Удалена задача {data.get('task')}"
        elif action == 'clear_all':
            return "Все задачи удалены"
        elif action == 'get_tasks':
            return "Список задач"
        elif action.startswith('arrange_'):
            return f"Расположение: {action.replace('arrange_', '')}"
        
        return None
    
    def _extract_all_json(self, text):
        results = []
        i = 0
        while i < len(text):
            if text[i] == '{':
                # Ищем закрывающую скобку
                depth = 0
                start = i
                for j in range(i, len(text)):
                    if text[j] == '{':
                        depth += 1
                    elif text[j] == '}':
                        depth -= 1
                        if depth == 0:
                            try:
                                json_str = text[start:j+1]
                                data = json.loads(json_str)
                                results.append(data)
                            except json.JSONDecodeError:
                                pass
                            i = j
                            break
            i += 1
        return results
    
    def _execute_action(self, data):
        action = data.get('action')
        if not action:
            return None
                
        if action == 'create_task':
            x, y = self._get_task_position()
            self.task_created.emit(
                data.get('title', 'Задача'),
                data.get('description', ''),
                data.get('status', 'todo'),
                x, y
            )
            return f"Создана: {data.get('title')}"
        
        elif action == 'create_tasks_chain':
            tasks = data.get('tasks', [])
            connect = data.get('connect', True)
            start_idx = self.task_counter
            for i, task in enumerate(tasks):
                x, y = self._get_task_position()
                self.task_created.emit(
                    task.get('title', 'Задача'),
                    task.get('description', ''),
                    task.get('status', 'todo'),
                    x, y
                )
            if connect and len(tasks) > 1:
                for i in range(len(tasks) - 1):
                    self.tasks_connect.emit(start_idx + i, start_idx + i + 1)
            return f"Создано {len(tasks)} задач" + (" (цепочка)" if connect else "")
        
        elif action == 'connect':
            self.tasks_connect.emit(data.get('from', 1) - 1, data.get('to', 2) - 1)
            return f"Соединено: {data.get('from')} → {data.get('to')}"
        
        elif action == 'connect_many':
            for conn in data.get('connections', []):
                if len(conn) == 2:
                    self.tasks_connect.emit(conn[0] - 1, conn[1] - 1)
            return f"Создано {len(data.get('connections', []))} соединений"
        
        elif action == 'disconnect':
            self.disconnect_tasks.emit(data.get('from', 1) - 1, data.get('to', 2) - 1)
            return f"Разъединено: {data.get('from')} — {data.get('to')}"
        
        elif action == 'set_status':
            self.task_update_status.emit(data.get('task', 1) - 1, data.get('status', 'none'))
            return f"Статус задачи {data.get('task')} → {data.get('status')}"
        
        elif action == 'set_many_status':
            status = data.get('status', 'todo')
            for t in data.get('tasks', []):
                self.task_update_status.emit(t - 1, status)
            return f"Статус задач {data.get('tasks')} → {status}"
        
        elif action == 'mark_done':
            for t in data.get('tasks', []):
                self.task_update_status.emit(t - 1, 'done')
            return f"Готово: {data.get('tasks')}"
        
        elif action == 'mark_progress':
            for t in data.get('tasks', []):
                self.task_update_status.emit(t - 1, 'progress')
            return f"В процессе: {data.get('tasks')}"

        elif action == 'rename':
            self.task_rename.emit(data.get('task', 1) - 1, data.get('title', ''))
            return f"Переименовано: задача {data.get('task')}"
        
        elif action == 'set_description':
            self.task_update_desc.emit(data.get('task', 1) - 1, data.get('description', ''))
            return f"Описание: задача {data.get('task')}"
        
        elif action == 'delete':
            self.task_delete.emit(data.get('task', 1) - 1)
            return f"Удалена задача {data.get('task')}"
        
        elif action == 'delete_many':
            tasks = [t - 1 for t in data.get('tasks', [])]
            self.tasks_delete_many.emit(tasks)
            return f"Удалено: {len(tasks)} задач"
        
        elif action == 'clear_all':
            self.clear_all_tasks.emit()
            self.task_counter = 0
            return "Все задачи удалены"
        
        elif action == 'get_tasks':
            self.request_tasks.emit()
            return "Список задач"
        
        elif action.startswith('arrange_'):
            arrange_type = action.replace('arrange_', '')
            self.arrange_tasks.emit(arrange_type)
            names = {
                'grid': 'сетка', 'horizontal': 'горизонтально', 
                'vertical': 'вертикально', 'random': 'случайно',
                'circle': 'по кругу', 'tree': 'дерево', 'cascade': 'каскад'
            }
            return f"Расположение: {names.get(arrange_type, arrange_type)}"
        
        elif action == 'template':
            template_type = data.get('type', 'web_app')
            self._create_template(template_type)
            return f"Шаблон: {template_type}"
        
        return None
    
    def _create_template(self, template_type):
        templates = {
            'web_app': [
                {"title": "Анализ требований", "description": "Сбор и анализ требований к веб-приложению"},
                {"title": "Дизайн UI/UX", "description": "Создание макетов и прототипов интерфейса"},
                {"title": "Настройка окружения", "description": "Настройка среды разработки и инструментов"},
                {"title": "Разработка фронтенда", "description": "Вёрстка и программирование клиентской части"},
                {"title": "Разработка бэкенда", "description": "Создание серверной логики и API"},
                {"title": "База данных", "description": "Проектирование и реализация БД"},
                {"title": "Тестирование", "description": "Юнит-тесты, интеграционные тесты"},
                {"title": "Деплой", "description": "Развёртывание на сервере"},
            ],
            'mobile_app': [
                {"title": "Исследование рынка", "description": "Анализ конкурентов и целевой аудитории"},
                {"title": "Дизайн приложения", "description": "UI/UX дизайн для мобильных устройств"},
                {"title": "Архитектура", "description": "Проектирование архитектуры приложения"},
                {"title": "Разработка", "description": "Программирование основного функционала"},
                {"title": "Интеграция API", "description": "Подключение внешних сервисов"},
                {"title": "Тестирование", "description": "QA тестирование на устройствах"},
                {"title": "Публикация", "description": "Публикация в App Store / Google Play"},
            ],
            'api': [
                {"title": "Спецификация API", "description": "Документация эндпоинтов и моделей"},
                {"title": "Аутентификация", "description": "Реализация JWT/OAuth"},
                {"title": "CRUD операции", "description": "Базовые операции с данными"},
                {"title": "Валидация", "description": "Валидация входных данных"},
                {"title": "Документация", "description": "Swagger/OpenAPI документация"},
                {"title": "Тесты", "description": "Автоматические тесты API"},
            ],
            'startup': [
                {"title": "Идея и валидация", "description": "Проверка гипотезы продукта"},
                {"title": "MVP", "description": "Минимально жизнеспособный продукт"},
                {"title": "Первые пользователи", "description": "Привлечение бета-тестеров"},
                {"title": "Обратная связь", "description": "Сбор и анализ фидбека"},
                {"title": "Итерация", "description": "Улучшение продукта"},
                {"title": "Масштабирование", "description": "Рост и развитие"},
            ],
            'landing': [
                {"title": "Контент", "description": "Тексты и изображения"},
                {"title": "Дизайн", "description": "Макет лендинга"},
                {"title": "Вёрстка", "description": "HTML/CSS разработка"},
                {"title": "Формы", "description": "Формы захвата лидов"},
                {"title": "Аналитика", "description": "Подключение метрик"},
                {"title": "Запуск", "description": "Публикация и тестирование"},
            ],
        }
        
        tasks = templates.get(template_type, templates['web_app'])
        for i, task in enumerate(tasks):
            x = 50 + (i % 3) * 250
            y = 50 + (i // 3) * 180
            self.task_created.emit(task['title'], task['description'], 'todo', x, y)
            self.task_counter += 1
        
        for i in range(len(tasks) - 1):
            self.tasks_connect.emit(i, i + 1)
    
    def set_tasks_info(self, info):
        self._add_message_ui(f"Текущие задачи:\n{info}", is_user=False)
    
    def _on_error(self, error):
        self.input_field.setEnabled(True)
        self.send_btn.setEnabled(True)
        self._add_message_ui(f"Ошибка: {error}", is_user=False)
