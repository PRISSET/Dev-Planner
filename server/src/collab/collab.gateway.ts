import {
  WebSocketGateway,
  WebSocketServer,
  SubscribeMessage,
  OnGatewayConnection,
  OnGatewayDisconnect,
  ConnectedSocket,
  MessageBody,
} from '@nestjs/websockets';
import { Server, Socket } from 'socket.io';
import { CollabService } from './collab.service';

@WebSocketGateway({
  cors: {
    origin: '*',
  },
})
export class CollabGateway implements OnGatewayConnection, OnGatewayDisconnect {
  @WebSocketServer()
  server: Server;

  private socketToRoom: Map<string, string> = new Map();

  constructor(private readonly collabService: CollabService) {}

  handleConnection(client: Socket) {
    console.log(`Client connected: ${client.id}`);
  }

  handleDisconnect(client: Socket) {
    const roomId = this.socketToRoom.get(client.id);
    if (roomId) {
      this.collabService.leaveRoom(roomId, client.id);
      this.socketToRoom.delete(client.id);
      
      const members = this.collabService.getMembersList(roomId);
      this.server.to(roomId).emit('members_updated', { members });
      this.server.to(roomId).emit('user_left', { id: client.id });
    }
  }

  @SubscribeMessage('create_room')
  handleCreateRoom(
    @ConnectedSocket() client: Socket,
    @MessageBody() data: { name: string; projectData: any },
  ) {
    const { roomId, code } = this.collabService.createRoom(
      client.id,
      data.name,
      data.projectData,
    );

    client.join(roomId);
    this.socketToRoom.set(client.id, roomId);

    return {
      success: true,
      roomId,
      code,
      message: `Room created with code: ${code}`,
    };
  }

  @SubscribeMessage('join_room')
  handleJoinRoom(
    @ConnectedSocket() client: Socket,
    @MessageBody() data: { code: string; name: string },
  ) {
    const room = this.collabService.joinRoom(data.code, client.id, data.name);

    if (!room) {
      return { success: false, message: 'Room not found' };
    }

    client.join(room.id);
    this.socketToRoom.set(client.id, room.id);

    const members = this.collabService.getMembersList(room.id);
    this.server.to(room.id).emit('members_updated', { members });
    this.server.to(room.id).emit('user_joined', { id: client.id, name: data.name });

    return {
      success: true,
      roomId: room.id,
      projectData: room.projectData,
      members,
      message: 'Joined successfully',
    };
  }

  @SubscribeMessage('sync_project')
  handleSyncProject(
    @ConnectedSocket() client: Socket,
    @MessageBody() data: { projectData: any },
  ) {
    const roomId = this.socketToRoom.get(client.id);
    if (!roomId) return { success: false };

    this.collabService.updateProjectData(roomId, data.projectData);
    client.to(roomId).emit('project_updated', { projectData: data.projectData });

    return { success: true };
  }

  @SubscribeMessage('task_action')
  handleTaskAction(
    @ConnectedSocket() client: Socket,
    @MessageBody() data: { action: string; payload: any },
  ) {
    const roomId = this.socketToRoom.get(client.id);
    if (!roomId) return { success: false };

    client.to(roomId).emit('task_action', {
      action: data.action,
      payload: data.payload,
      from: client.id,
    });

    return { success: true };
  }

  @SubscribeMessage('cursor_move')
  handleCursorMove(
    @ConnectedSocket() client: Socket,
    @MessageBody() data: { x: number; y: number; name: string },
  ) {
    const roomId = this.socketToRoom.get(client.id);
    if (!roomId) return;

    client.to(roomId).emit('cursor_update', {
      id: client.id,
      x: data.x,
      y: data.y,
      name: data.name,
    });
  }

  @SubscribeMessage('leave_room')
  handleLeaveRoom(@ConnectedSocket() client: Socket) {
    const roomId = this.socketToRoom.get(client.id);
    if (!roomId) return { success: false };

    this.collabService.leaveRoom(roomId, client.id);
    client.leave(roomId);
    this.socketToRoom.delete(client.id);

    const members = this.collabService.getMembersList(roomId);
    this.server.to(roomId).emit('members_updated', { members });
    this.server.to(roomId).emit('user_left', { id: client.id });

    return { success: true };
  }

  @SubscribeMessage('close_room')
  handleCloseRoom(@ConnectedSocket() client: Socket) {
    const roomId = this.socketToRoom.get(client.id);
    if (!roomId) return { success: false, message: 'Not in a room' };

    const isHost = this.collabService.isRoomHost(roomId, client.id);
    if (!isHost) return { success: false, message: 'Only host can close room' };

    this.server.to(roomId).emit('room_closed', { message: 'Room was closed by host' });
    
    const closed = this.collabService.closeRoom(roomId, client.id);
    if (closed) {
      this.server.in(roomId).socketsLeave(roomId);
      for (const [socketId, rId] of this.socketToRoom.entries()) {
        if (rId === roomId) {
          this.socketToRoom.delete(socketId);
        }
      }
    }

    return { success: true };
  }

  @SubscribeMessage('rejoin_room')
  handleRejoinRoom(
    @ConnectedSocket() client: Socket,
    @MessageBody() data: { code: string; name: string },
  ) {
    const room = this.collabService.getRoomByCode(data.code);
    if (!room) {
      return { success: false, message: 'Room not found or closed' };
    }

    const existingRoom = this.socketToRoom.get(client.id);
    if (existingRoom) {
      client.leave(existingRoom);
    }

    room.members.set(client.id, { id: client.id, name: data.name });
    client.join(room.id);
    this.socketToRoom.set(client.id, room.id);

    const members = this.collabService.getMembersList(room.id);
    this.server.to(room.id).emit('members_updated', { members });
    this.server.to(room.id).emit('user_joined', { id: client.id, name: data.name });

    return {
      success: true,
      roomId: room.id,
      projectData: room.projectData,
      members,
      isHost: room.hostId === client.id,
    };
  }
}
