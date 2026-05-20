import { ApiProperty, ApiPropertyOptional } from '@nestjs/swagger';
import { IsEnum, IsNotEmpty, IsObject, IsOptional, IsString } from 'class-validator';
import { LogLevel, Prisma } from '@generated/prisma/client';

export class CreateTelemetryDto {
    @ApiProperty({ example: 'bridge-01' })
    @IsString()
    @IsNotEmpty()
    deviceCode: string;

    @ApiProperty({ example: 'BRIDGE_OPENED' })
    @IsString()
    @IsNotEmpty()
    eventType: string;

    @ApiProperty({ example: 'Open' })
    @IsString()
    @IsNotEmpty()
    state: string;

    @ApiProperty({ enum: LogLevel, example: LogLevel.INFO })
    @IsEnum(LogLevel)
    level: LogLevel;

    @ApiProperty({ example: 'Bridge opened successfully' })
    @IsString()
    @IsNotEmpty()
    message: string;

    @ApiPropertyOptional({
        example: {
            fromState: 'Opening',
            toState: 'Open',
            sensors: {
                boatDetected: true,
                stepperPos: 6400,
            },
        },
    })
    @IsOptional()
    @IsObject()
    data?: Prisma.InputJsonValue;
}