import {ApiProperty} from "@nestjs/swagger";
import {IsEnum, IsString} from "class-validator";

export class HardwareTypeDto {
    @ApiProperty({ example: 'RAILWAY_BARRIER' })
    @IsString()
    key: string;

    @ApiProperty({ example: 'Railway Barrier' })
    @IsString()
    name: string;

    @ApiProperty()
    updatedAt: Date;

    @ApiProperty()
    createdAt: Date;

}