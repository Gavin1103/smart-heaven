import { ApiPropertyOptional } from '@nestjs/swagger';
import { IsOptional, IsString } from 'class-validator';

export class UpdateDeviceDto {
    @ApiPropertyOptional({ example: 'Bridge - Tile 1' })
    @IsOptional()
    @IsString()
    name?: string;

    @ApiPropertyOptional({ example: 'bridge on tile 1', nullable: true })
    @IsOptional()
    @IsString()
    description?: string | null;

    @ApiPropertyOptional({ example: 'Bridge tile 1', nullable: true })
    @IsOptional()
    @IsString()
    locationName?: string | null;
}
