import { ApiProperty, ApiPropertyOptional } from '@nestjs/swagger';
import { IsInt, IsNotEmpty, IsOptional, IsString, IsObject } from 'class-validator';
import { Prisma } from '@generated/prisma/client';

export class UpsertDeviceStateDto {
    @ApiProperty()
    @IsInt()
    deviceId: number;

    @ApiProperty()
    @IsString()
    @IsNotEmpty()
    state: string;

    @ApiPropertyOptional()
    @IsOptional()
    @IsString()
    statusMessage?: string;

    @ApiPropertyOptional()
    @IsOptional()
    @IsObject()
    data?: Prisma.InputJsonValue;
}