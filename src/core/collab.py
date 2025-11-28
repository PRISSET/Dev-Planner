import socketio
from PyQt6.QtCore import QObject, pyqtSignal, QThread

SERVER_URL = "http://144.31.221.20:3005"


class CollabWorker(QObject):
    connected = pyqtSignal()
    disconnected = pyqtSignal()
    room_created = pyqtSignal(str, str)
    room_joined = pyqtSignal(str, object, list)
    join_failed = pyqtSignal(str)
    members_updated = pyqtSignal(list)
    user_joined = pyqtSignal(str, str)
    user_left = pyqtSignal(str)
    project_updated = pyqtSignal(object)
    task_action_received = pyqtSignal(str, object, str)
    cursor_updated = pyqtSignal(str, float, float, str)
    room_closed = pyqtSignal(str)
    error_occurred = pyqtSignal(str)
    
    do_connect = pyqtSignal()
    do_disconnect = pyqtSignal()
    do_create_room = pyqtSignal(str, object)
    do_join_room = pyqtSignal(str, str)
    do_leave_room = pyqtSignal()
    do_close_room = pyqtSignal()
    do_sync_project = pyqtSignal(object)
    do_task_action = pyqtSignal(str, object)
    do_cursor_move = pyqtSignal(float, float, str)

    def __init__(self):
        super().__init__()
        self.sio = socketio.Client(reconnection=False)
        self.room_id = None
        self.user_name = "User"
        self.is_host = False
        self._setup_handlers()
        self._setup_slots()

    def _setup_slots(self):
        self.do_connect.connect(self._connect)
        self.do_disconnect.connect(self._disconnect)
        self.do_create_room.connect(self._create_room)
        self.do_join_room.connect(self._join_room)
        self.do_leave_room.connect(self._leave_room)
        self.do_close_room.connect(self._close_room)
        self.do_sync_project.connect(self._sync_project)
        self.do_task_action.connect(self._task_action)
        self.do_cursor_move.connect(self._cursor_move)

    def _setup_handlers(self):
        @self.sio.event
        def connect():
            self.connected.emit()

        @self.sio.event
        def disconnect():
            self.disconnected.emit()
            self.room_id = None

        @self.sio.on('members_updated')
        def on_members_updated(data):
            self.members_updated.emit(data.get('members', []))

        @self.sio.on('user_joined')
        def on_user_joined(data):
            self.user_joined.emit(data.get('id', ''), data.get('name', ''))

        @self.sio.on('user_left')
        def on_user_left(data):
            self.user_left.emit(data.get('id', ''))

        @self.sio.on('project_updated')
        def on_project_updated(data):
            self.project_updated.emit(data.get('projectData', {}))

        @self.sio.on('task_action')
        def on_task_action(data):
            self.task_action_received.emit(
                data.get('action', ''),
                data.get('payload', {}),
                data.get('from', '')
            )

        @self.sio.on('cursor_update')
        def on_cursor_update(data):
            self.cursor_updated.emit(
                data.get('id', ''),
                float(data.get('x', 0)),
                float(data.get('y', 0)),
                data.get('name', '')
            )

        @self.sio.on('room_closed')
        def on_room_closed(data):
            self.room_id = None
            self.is_host = False
            self.room_closed.emit(data.get('message', 'Room closed'))

    def _connect(self):
        try:
            if not self.sio.connected:
                self.sio.connect(SERVER_URL)
        except Exception as e:
            self.error_occurred.emit(f"Connection failed: {str(e)}")

    def _disconnect(self):
        try:
            if self.sio.connected:
                if self.room_id:
                    self.sio.emit('leave_room')
                self.sio.disconnect()
        except Exception:
            pass

    def _create_room(self, name, project_data):
        try:
            if not self.sio.connected:
                self.sio.connect(SERVER_URL)
            
            self.user_name = name
            self.is_host = True
            
            response = self.sio.call('create_room', {
                'name': name,
                'projectData': project_data
            }, timeout=10)
            
            if response and response.get('success'):
                self.room_id = response.get('roomId')
                self.room_created.emit(self.room_id, response.get('code', ''))
            else:
                self.error_occurred.emit(response.get('message', 'Failed to create room') if response else 'No response')
        except Exception as e:
            self.error_occurred.emit(str(e))

    def _join_room(self, code, name):
        try:
            if not self.sio.connected:
                self.sio.connect(SERVER_URL)
            
            self.user_name = name
            self.is_host = False
            
            response = self.sio.call('join_room', {
                'code': code,
                'name': name
            }, timeout=10)
            
            if response and response.get('success'):
                self.room_id = response.get('roomId')
                self.room_joined.emit(
                    self.room_id,
                    response.get('projectData', {}),
                    response.get('members', [])
                )
            else:
                self.join_failed.emit(response.get('message', 'Failed to join room') if response else 'No response')
        except Exception as e:
            self.join_failed.emit(str(e))

    def _leave_room(self):
        try:
            if self.sio.connected and self.room_id:
                self.sio.emit('leave_room')
                self.room_id = None
        except Exception:
            pass

    def _close_room(self):
        try:
            if self.sio.connected and self.room_id and self.is_host:
                self.sio.call('close_room', timeout=5)
                self.room_id = None
                self.is_host = False
        except Exception:
            pass

    def _sync_project(self, project_data):
        try:
            if self.sio.connected and self.room_id:
                self.sio.emit('sync_project', {'projectData': project_data})
        except Exception:
            pass

    def _task_action(self, action, payload):
        try:
            if self.sio.connected and self.room_id:
                self.sio.emit('task_action', {'action': action, 'payload': payload})
        except Exception:
            pass

    def _cursor_move(self, x, y, name):
        try:
            if self.sio.connected and self.room_id:
                self.sio.emit('cursor_move', {'x': x, 'y': y, 'name': name})
        except Exception:
            pass


class CollabClient(QObject):
    connected = pyqtSignal()
    disconnected = pyqtSignal()
    room_created = pyqtSignal(str, str)
    room_joined = pyqtSignal(str, object, list)
    join_failed = pyqtSignal(str)
    members_updated = pyqtSignal(list)
    user_joined = pyqtSignal(str, str)
    user_left = pyqtSignal(str)
    project_updated = pyqtSignal(object)
    task_action_received = pyqtSignal(str, object, str)
    cursor_updated = pyqtSignal(str, float, float, str)
    room_closed = pyqtSignal(str)
    error_occurred = pyqtSignal(str)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.thread = QThread()
        self.worker = CollabWorker()
        self.worker.moveToThread(self.thread)
        
        self.worker.connected.connect(self.connected)
        self.worker.disconnected.connect(self.disconnected)
        self.worker.room_created.connect(self.room_created)
        self.worker.room_joined.connect(self.room_joined)
        self.worker.join_failed.connect(self.join_failed)
        self.worker.members_updated.connect(self.members_updated)
        self.worker.user_joined.connect(self.user_joined)
        self.worker.user_left.connect(self.user_left)
        self.worker.project_updated.connect(self.project_updated)
        self.worker.task_action_received.connect(self.task_action_received)
        self.worker.cursor_updated.connect(self.cursor_updated)
        self.worker.room_closed.connect(self.room_closed)
        self.worker.error_occurred.connect(self.error_occurred)
        
        self.thread.start()

    def connect_to_server(self):
        self.worker.do_connect.emit()

    def disconnect_from_server(self):
        self.worker.do_disconnect.emit()

    def create_room(self, name: str, project_data):
        self.worker.do_create_room.emit(name, project_data)

    def join_room(self, code: str, name: str):
        self.worker.do_join_room.emit(code, name)

    def leave_room(self):
        self.worker.do_leave_room.emit()

    def close_room(self):
        self.worker.do_close_room.emit()

    def sync_project(self, project_data):
        self.worker.do_sync_project.emit(project_data)

    def send_task_action(self, action: str, payload):
        self.worker.do_task_action.emit(action, payload)

    def send_cursor_position(self, x: float, y: float):
        self.worker.do_cursor_move.emit(x, y, self.worker.user_name)

    @property
    def is_connected(self):
        return self.worker.sio.connected if self.worker else False

    @property
    def in_room(self):
        return self.worker.room_id is not None if self.worker else False

    @property
    def is_host(self):
        return self.worker.is_host if self.worker else False

    def cleanup(self):
        self.worker.do_disconnect.emit()
        self.thread.quit()
        self.thread.wait(2000)
