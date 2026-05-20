import { Controller, Post, Get, Body } from '@nestjs/common'
import { ApiTags, ApiResponse } from '@nestjs/swagger'
import { ParkingService } from './parking.service'
import { UpdateParkingDto } from './dto/update-parking.dto'

@ApiTags('parking')
@Controller('parking')
export class ParkingController {

  constructor(
    private readonly parkingService: ParkingService
  ) {}

  @Post('update')
  @ApiResponse({
    status: 200,
    description: 'Parking spot updated successfully'
  })
  updateParking(@Body() dto: UpdateParkingDto) {
    return this.parkingService.updateParking(dto)
  }

  @Get()
  @ApiResponse({
    status: 200,
    description: 'List of all parking spots returned'
  })
  getAllParking() {
    return this.parkingService.getAllParking()
  }
}