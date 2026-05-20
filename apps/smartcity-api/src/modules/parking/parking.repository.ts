import { Injectable } from '@nestjs/common'
import { PrismaService } from 'prisma/prisma.service'
import { UpdateParkingDto } from './dto/update-parking.dto'

@Injectable()
export class ParkingRepository {
  constructor(private prisma: PrismaService) {}

  async upsertParking(data: UpdateParkingDto) {
    return this.prisma.parkingSpot.upsert({
      where: {
        spotCode: data.spotCode,
      },
      update: {
        occupied: data.occupied,
        location: data.location,
      },
      create: data,
    })
  }

  async findAll() {
    return this.prisma.parkingSpot.findMany()
  }
}