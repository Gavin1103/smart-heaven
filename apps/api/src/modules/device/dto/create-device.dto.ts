import { ApiProperty } from '@nestjs/swagger';
import { IsInt, IsOptional, IsString } from 'class-validator';

export class CreateDeviceDto {
    @ApiProperty({ example: 'Bridge - Tile 1' })
    @IsString()
    name: string;

    @ApiProperty({ example: 'B-T1' })
    @IsString()
    code: string;

    @ApiProperty({ nullable: true, example: 'bridge on tile 1' })
    @IsOptional()
    @IsString()
    description?: string | null;

    @ApiProperty({ example: "RAILWAY_BARRIER" })
    @IsString()
    hardwareTypeKey: string;

    @ApiProperty({ example: 'Bridge tile 1', nullable: true })
    @IsOptional()
    @IsString()
    locationName?: string | null;
}