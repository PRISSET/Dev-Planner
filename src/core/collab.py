import json
import socketio
from PyQt6.QtCore import QObject, pyqtSignal, QThread

SERVER_URL = "http://144.31.221.20:3005"

class CollabClient(QObject):
    connected = pyqtSignal()
    disconnected = pyqtSignal()
    room_created = pyqtSignal(str, str)  # roomId, code
    room_joined = pyqtSignal(str, dict, list)  # roomId, projectData, members
    join_failed = pyqtSignal(str)  # message
    members_updated = pyqtSignal(list)  # members
    user_joined = pyqtSignal(str, str)  # id, name
    user_left = pyqtSignal(str)  # id
    project_updated = pyqtSignal(dict)  # projectData
    task_action_received = pyqtSignal(str, dict, str)  # action, payload, from
    cursor_updated = pyqtSignal(str, float, float, str)  # id, x, y, name
    room_closed = pyqtSignal(str)  # message
    error_occurred = pyqtSignal(str)  # message

    def __init__(self, parent=None):
        super().__init__(parent)
        self.sio = socketio.Client()
        self.room_id = None
        self.room_code = None
        self.user_name = "User"
        self.is_host = False
        self._setup_handlers()

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
                data.get('x', 0),
                data.get('y', 0),
                data.get('name', '')
            )

        @self.sio.on('room_closed')
        def on_room_closed(data):
            self.room_id = None
            self.room_code = None
            self.is_host = False
            self.room_closed.emit(data.get('message', 'Room closed'))

    def connect_to_server(self):
        try:
            if not self.sio.connected:
                self.sio.connect(SERVER_URL)
            return True
        except Exception as e:
            self.error_occurred.emit(f"Connection failed: {str(e)}")
            return False

    def disconnect_from_server(self):
        if self.sio.connected:
            if self.room_id:
                self.sio.emit('leave_room')
            self.sio.disconnect()

    def create_room(self, name: str, project_data: dict):
        if not self.sio.connected:
            if not self.connect_to_server():
                return
        
        self.user_name = name
        self.is_host = True
        
        response = self.sio.call('create_room', {
            'name': name,
            'projectData': project_data
        })
        
        if response.get('success'):
            self.room_id = response.get('roomId')
            self.room_created.emit(self.room_id, response.get('code', ''))
        else:
            self.error_occurred.emit(response.get('message', 'Failed to create room'))

    def join_room(self, code: str, name: str):
        if not self.sio.connected:
            if not self.connect_to_server():
                return
        
        self.user_name = name
        self.is_host = False
        
        response = self.sio.call('join_room', {
            'code': code,
            'name': name
        })
        
        if response.get('success'):
            self.room_id = response.get('roomId')
            self.room_joined.emit(
                self.room_id,
                response.get('projectData', {}),
                response.get('members', [])
            )
        else:
            self.join_failed.emit(response.get('message', 'Failed to join room'))

    def leave_room(self):
        if self.sio.connected and self.room_id:
            self.sio.emit('leave_room')
            self.room_id = None
            self.room_code = None

    def close_room(self):
        if self.sio.connected and self.room_id and self.is_host:
            response = self.sio.call('close_room')
            if response.get('success'):
                self.room_id = None
                self.room_code = None
                self.is_host = False
                return True
        return False

    def sync_project(self, project_data: dict):
        if self.sio.connected and self.room_id:
            self.sio.emit('sync_project', {'projectData': project_data})

    def send_task_action(self, action: str, payload: dict):
        if self.sio.connected and self.room_id:
            self.sio.emit('task_action', {'action': action, 'payload': payload})

    def send_cursor_position(self, x: float, y: float):
        if self.sio.connected and self.room_id:
            self.sio.emit('cursor_move', {'x': x, 'y': y, 'name': self.user_name})

    @property
    def is_connected(self):
        return self.sio.connected

    @property
    def in_room(self):
        return self.room_id is not None


class CollabThread(QThread):
    def __init__(self, client: CollabClient, action: str, **kwargs):
        super().__init__()
        self.client = client
        self.action = action
        self.kwargs = kwargs

    def run(self):
        if self.action == 'connect':
            self.client.connect_to_server()
        elif self.action == 'create_room':
            self.client.create_room(self.kwargs.get('name'), self.kwargs.get('project_data'))
        elif self.action == 'join_room':
            self.client.join_room(self.kwargs.get('code'), self.kwargs.get('name'))
        elif self.action == 'leave_room':
            self.client.leave_room()
        elif self.action == 'close_room':
            self.client.close_room()
        elif self.action == 'disconnect':
            self.client.disconnect_from_server()
