import {ApiPropertyOptional} from "@nestjs/swagger";
import {IsOptional, IsString} from "class-validator";

export class UpdateHardwareTypeDto {
    @ApiPropertyOptional({ example: 'Railway Barrier' })
    @IsOptional()
    @IsString()
    name?: string;
}
