import { NestFactory } from '@nestjs/core';
import { AppModule } from './app.module';

async function bootstrap() {
  const app = await NestFactory.create(AppModule, {
    cors: {
      origin: '*',
      credentials: true,
    },
  });
  
  await app.listen(3005, '0.0.0.0');
  console.log('Dev Planner Server running on port 3005');
}
bootstrap();
