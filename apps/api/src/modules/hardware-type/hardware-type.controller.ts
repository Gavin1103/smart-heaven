import {Body, Controller, Delete, Get, HttpCode, HttpStatus, Param, Patch, Post, UseGuards} from "@nestjs/common";
import {ApiBearerAuth, ApiBody, ApiOperation, ApiTags} from "@nestjs/swagger";
import {HardwareTypeService} from "./hardware-type.service";
import {ApiSuccessResponseDecorator} from "../../common/decorators/api-success-response.decorator";
import {ApiErrorResponsesDecorator} from "../../common/decorators/api-error-response.decorator";
import {CreateHardwareTypeDto} from "./dto/create-hardware-type";
import {UpdateHardwareTypeDto} from "./dto/update-hardware-type.dto";
import {HardwareTypeDto} from "./dto/hardware-type.dto";
import {JwtAuthGuard} from "../auth/guards/jwt-auth.guard";
import {RolesGuard} from "../auth/guards/roles.guard";
import {Roles} from "../auth/decorators/roles.decorator";
import {Role} from "@generated/prisma/enums";

@ApiTags('hardware-types')
@Controller('hardware-types')
@UseGuards(JwtAuthGuard)
@ApiBearerAuth()
export class HardwareTypeController {
    constructor(private readonly hardwareTypeService: HardwareTypeService) {
    }

    @Post()
    @UseGuards(JwtAuthGuard, RolesGuard)
    @Roles(Role.ADMIN)
    @ApiBearerAuth()
    @ApiOperation({summary: 'Create a hardware type'})
    @ApiBody({type: CreateHardwareTypeDto})
    @ApiSuccessResponseDecorator(HardwareTypeDto, 201, 'Successfully created hardware type')
    @ApiErrorResponsesDecorator(401, 403, 404, 409, 400)
    async createHardwareType(@Body() dto: CreateHardwareTypeDto) {
        const data = await this.hardwareTypeService.createHardwareType(dto);
        return {message: 'Successfully created hardware type', data};
    }

    @Get()
    @ApiOperation({summary: 'Get all hardware types'})
    @ApiSuccessResponseDecorator(HardwareTypeDto, 200, 'Hardware types fetched successfully')
    async findAll() {
        const data = await this.hardwareTypeService.findAll();
        return {message: 'Hardware types fetched successfully', data};
    }

    @Get(':key')
    @ApiOperation({summary: 'Get a hardware type by key'})
    @ApiSuccessResponseDecorator(HardwareTypeDto, 200, 'Hardware type fetched successfully')
    @ApiErrorResponsesDecorator(404)
    async findByKey(@Param('key') key: string) {
        const data = await this.hardwareTypeService.findByKey(key);
        return {message: 'Hardware type fetched successfully', data};
    }

    @Patch(':key')
    @UseGuards(JwtAuthGuard, RolesGuard)
    @Roles(Role.ADMIN)
    @ApiBearerAuth()
    @ApiOperation({summary: 'Update a hardware type'})
    @ApiBody({type: UpdateHardwareTypeDto})
    @ApiSuccessResponseDecorator(HardwareTypeDto, 200, 'Hardware type updated successfully')
    @ApiErrorResponsesDecorator(401, 403, 404, 400)
    async updateHardwareType(
        @Param('key') key: string,
        @Body() dto: UpdateHardwareTypeDto,
    ) {
        const data = await this.hardwareTypeService.updateHardwareType(key, dto);
        return {message: 'Hardware type updated successfully', data};
    }

    @Delete(':key')
    @HttpCode(HttpStatus.NO_CONTENT)
    @UseGuards(JwtAuthGuard, RolesGuard)
    @Roles(Role.ADMIN)
    @ApiBearerAuth()
    @ApiOperation({summary: 'Delete a hardware type'})
    @ApiErrorResponsesDecorator(401, 403, 404)
    async deleteHardwareType(@Param('key') key: string): Promise<void> {
        await this.hardwareTypeService.deleteHardwareType(key);
    }
}
