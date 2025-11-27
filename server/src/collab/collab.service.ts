import { Injectable } from '@nestjs/common';
import { v4 as uuidv4 } from 'uuid';

interface Room {
  id: string;
  code: string;
  hostId: string;
  members: Map<string, { id: string; name: string }>;
  projectData: any;
  createdAt: Date;
}

@Injectable()
export class CollabService {
  private rooms: Map<string, Room> = new Map();
  private codeToRoom: Map<string, string> = new Map();

  generateCode(): string {
    const chars = 'ABCDEFGHJKLMNPQRSTUVWXYZ23456789';
    let code = '';
    for (let i = 0; i < 6; i++) {
      code += chars.charAt(Math.floor(Math.random() * chars.length));
    }
    return code;
  }

  createRoom(hostId: string, hostName: string, projectData: any): { roomId: string; code: string } {
    const roomId = uuidv4();
    let code = this.generateCode();
    
    while (this.codeToRoom.has(code)) {
      code = this.generateCode();
    }

    const room: Room = {
      id: roomId,
      code,
      hostId,
      members: new Map([[hostId, { id: hostId, name: hostName }]]),
      projectData,
      createdAt: new Date(),
    };

    this.rooms.set(roomId, room);
    this.codeToRoom.set(code, roomId);

    return { roomId, code };
  }

  joinRoom(code: string, memberId: string, memberName: string): Room | null {
    const roomId = this.codeToRoom.get(code.toUpperCase());
    if (!roomId) return null;

    const room = this.rooms.get(roomId);
    if (!room) return null;

    room.members.set(memberId, { id: memberId, name: memberName });
    return room;
  }

  leaveRoom(roomId: string, memberId: string): boolean {
    const room = this.rooms.get(roomId);
    if (!room) return false;

    room.members.delete(memberId);
    return true;
  }

  closeRoom(roomId: string, memberId: string): boolean {
    const room = this.rooms.get(roomId);
    if (!room) return false;
    
    if (memberId !== room.hostId) return false;
    
    this.rooms.delete(roomId);
    this.codeToRoom.delete(room.code);
    return true;
  }

  isRoomHost(roomId: string, memberId: string): boolean {
    const room = this.rooms.get(roomId);
    if (!room) return false;
    return room.hostId === memberId;
  }

  getRoom(roomId: string): Room | null {
    return this.rooms.get(roomId) || null;
  }

  getRoomByCode(code: string): Room | null {
    const roomId = this.codeToRoom.get(code.toUpperCase());
    if (!roomId) return null;
    return this.rooms.get(roomId) || null;
  }

  updateProjectData(roomId: string, projectData: any): boolean {
    const room = this.rooms.get(roomId);
    if (!room) return false;
    room.projectData = projectData;
    return true;
  }

  getMembersList(roomId: string): { id: string; name: string }[] {
    const room = this.rooms.get(roomId);
    if (!room) return [];
    return Array.from(room.members.values());
  }
}
