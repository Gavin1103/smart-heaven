import {ApiProperty} from "@nestjs/swagger";
import {IsEnum, IsString} from "class-validator";

export class CreateHardwareTypeDto {
    @ApiProperty({ example: 'RAILWAY_BARRIER' })
    @IsString()
    key: string;

    @ApiProperty({ example: 'Railway Barrier' })
    @IsString()
    name: string;
}