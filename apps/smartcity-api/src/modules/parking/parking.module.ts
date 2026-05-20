import { Module } from '@nestjs/common'
import { ParkingController } from './parking.controller'
import { ParkingService } from './parking.service'
import { ParkingRepository } from './parking.repository'
import { PrismaModule } from 'prisma/prisma.module'

@Module({
  imports: [PrismaModule],
  controllers: [ParkingController],
  providers: [ParkingService, ParkingRepository],
})
export class ParkingModule {}