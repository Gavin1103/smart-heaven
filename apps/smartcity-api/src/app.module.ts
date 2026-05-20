import { Module } from '@nestjs/common';
import { AppController } from './app.controller';
import { AppService } from './app.service';
import { ParkingModule } from './modules/parking/parking.module';
import { PrismaModule } from 'prisma/prisma.module';

@Module({
  imports: [ParkingModule, PrismaModule],
  controllers: [AppController],
  providers: [AppService],
   
})
export class AppModule {}


