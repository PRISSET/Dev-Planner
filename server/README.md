# Dev Planner Server

WebSocket сервер для коллаборации в Dev Planner.

## Установка

```bash
cd server
npm install
```

## Запуск

### Development
```bash
npm run start:dev
```

### Production
```bash
npm run build
npm run start:prod
```

## API

### WebSocket Events

#### Client -> Server

- `create_room` - создать комнату
  - `{ name: string, projectData: object }`
  - Returns: `{ success: boolean, roomId: string, code: string }`

- `join_room` - присоединиться к комнате
  - `{ code: string, name: string }`
  - Returns: `{ success: boolean, roomId: string, projectData: object, members: array }`

- `leave_room` - покинуть комнату

- `sync_project` - синхронизировать проект
  - `{ projectData: object }`

- `task_action` - действие с задачей
  - `{ action: string, payload: object }`

- `cursor_move` - движение курсора
  - `{ x: number, y: number, name: string }`

#### Server -> Client

- `members_updated` - обновление списка участников
- `user_joined` - пользователь присоединился
- `user_left` - пользователь вышел
- `project_updated` - проект обновлён
- `task_action` - действие с задачей от другого пользователя
- `cursor_update` - обновление курсора другого пользователя

## Деплой

Сервер слушает порт 3000 на всех интерфейсах (0.0.0.0).

```bash
# На сервере 144.31.221.20
npm run build
npm run start:prod
```
