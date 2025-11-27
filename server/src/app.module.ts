import { Module } from '@nestjs/common';
import { CollabGateway } from './collab/collab.gateway';
import { CollabService } from './collab/collab.service';

@Module({
  providers: [CollabGateway, CollabService],
})
export class AppModule {}
