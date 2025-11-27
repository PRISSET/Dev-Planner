import os

STATUSES = {
    'none': {'color': '#666666', 'name': 'Без статуса'},
    'done': {'color': '#00ff9d', 'name': 'Готово'},
    'progress': {'color': '#ffcc00', 'name': 'В процессе'},
    'todo': {'color': '#ff0055', 'name': 'Не сделано'},
    'cancelled': {'color': '#555555', 'name': 'Отменено'}
}

DATA_DIR = os.path.expanduser("~/.devchain_planner")
PROJECTS_FILE = os.path.join(DATA_DIR, "projects.json")
