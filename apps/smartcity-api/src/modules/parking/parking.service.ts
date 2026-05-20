import { Injectable } from '@nestjs/common'
import { ParkingRepository } from './parking.repository'
import { UpdateParkingDto } from './dto/update-parking.dto'

@Injectable()
export class ParkingService {
  constructor(private readonly parkingRepository: ParkingRepository) {}

  async updateParking(dto: UpdateParkingDto) {
    return this.parkingRepository.upsertParking(dto)
  }

  async getAllParking() {
    return this.parkingRepository.findAll()
  }
}