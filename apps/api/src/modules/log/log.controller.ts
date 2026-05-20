import {Body, Controller, Get, Post, Query, UseGuards} from '@nestjs/common';
import {ApiBearerAuth, ApiBody, ApiOperation, ApiQuery, ApiTags} from "@nestjs/swagger";
import {LogService} from './log.service';
import {CreateLogDto} from './dto/create-log.dto';
import {LogDto} from "./dto/log.dto";
import {ApiSuccessResponseDecorator} from "../../common/decorators/api-success-response.decorator";
import {ApiErrorResponsesDecorator} from "../../common/decorators/api-error-response.decorator";
import {JwtAuthGuard} from "../auth/guards/jwt-auth.guard";
import {RolesGuard} from "../auth/guards/roles.guard";
import {Roles} from "../auth/decorators/roles.decorator";
import {Role} from "@generated/prisma/enums";

@ApiTags('logs')
@Controller('logs')
@UseGuards(JwtAuthGuard)
@ApiBearerAuth()
export class LogController {
    constructor(private readonly logService: LogService) {
    }

    @Post()
    @UseGuards(RolesGuard)
    @Roles(Role.ADMIN)
    @ApiOperation({summary: 'Create a log entry'})
    @ApiBody({type: CreateLogDto})
    @ApiSuccessResponseDecorator(LogDto, 201, 'Log created successfully')
    @ApiErrorResponsesDecorator(401, 403, 404, 400)
    async createLog(@Body() dto: CreateLogDto) {
        const data = await this.logService.createLog(dto);
        return {message: 'Log created successfully', data};
    }

    @Get()
    @ApiOperation({summary: 'Fetch paginated logs'})
    @ApiQuery({name: 'page', required: false, example: 1, description: 'Page number'})
    @ApiQuery({name: 'limit', required: false, example: 25, description: 'Amount of logs per page'})
    @ApiQuery({name: 'deviceId', required: false, example: 1, description: 'Filter logs by device id'})
    @ApiErrorResponsesDecorator(401)
    findMany(
        @Query('page') page?: string,
        @Query('limit') limit?: string,
        @Query('deviceId') deviceId?: string,
    ) {
        return this.logService.findMany(
            page ? parseInt(page) : 1,
            limit ? parseInt(limit) : 25,
            deviceId ? parseInt(deviceId) : undefined,
        );
    }
}
