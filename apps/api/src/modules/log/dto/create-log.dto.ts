import {IsEnum, IsInt, IsNotEmpty, IsOptional, IsString} from 'class-validator';
import {ApiProperty, ApiPropertyOptional} from '@nestjs/swagger';
import {LogLevel} from "@generated/prisma/enums";
import {Prisma} from "@generated/prisma/client";

export class CreateLogDto {
    @ApiProperty()
    @IsInt()
    deviceId: number;

    @ApiProperty({enum: LogLevel, default: LogLevel.INFO})
    @IsEnum(LogLevel)
    level: LogLevel;

    @ApiProperty()
    @IsString()
    @IsNotEmpty()
    eventType: string;

    @ApiProperty()
    @IsString()
    @IsNotEmpty()
    message: string;

    @ApiPropertyOptional()
    @IsOptional()
    payload?: Prisma.InputJsonValue;}