import {IsEnum, IsInt, IsNotEmpty, IsOptional, IsString} from 'class-validator';
import {ApiProperty, ApiPropertyOptional} from '@nestjs/swagger';
import {LogLevel} from "@generated/prisma/enums";
import {NullableJsonNullValueInput} from "@generated/prisma/internal/prismaNamespace";

export class LogDto {
    @ApiProperty()
    deviceId: number;

    @ApiProperty({enum: LogLevel, default: LogLevel.INFO})
    @IsEnum(LogLevel)
    level: LogLevel;

    @ApiProperty()
    eventType: string;

    @ApiProperty()
    message: string;

    @ApiPropertyOptional()
    payload: NullableJsonNullValueInput;

    @ApiProperty()
    createdAt: Date;
}