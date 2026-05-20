import { IsBoolean, IsString } from 'class-validator'

export class UpdateParkingDto {
  @IsString()
  spotCode: string

  @IsString()
  location: string

  @IsBoolean()
  occupied: boolean
}