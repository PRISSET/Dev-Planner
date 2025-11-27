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

- `create_room`
  - `{ name: string, projectData: object }`
  - Returns: `{ success: boolean, roomId: string, code: string }`

- `join_room`
  - `{ code: string, name: string }`
  - Returns: `{ success: boolean, roomId: string, projectData: object, members: array }`

- `leave_room`

- `sync_project`
  - `{ projectData: object }`

- `task_action`
  - `{ action: string, payload: object }`

- `cursor_move`
  - `{ x: number, y: number, name: string }`

#### Server -> Client

- `members_updated`
- `user_joined`
- `user_left`
- `project_updated`
- `task_action`
- `cursor_update`

## Деплой

Сервер слушает порт 3000 на всех интерфейсах (0.0.0.0).

```bash
npm run build
npm run start:prod
```
