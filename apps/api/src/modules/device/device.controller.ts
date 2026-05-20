import {ApiBearerAuth, ApiBody, ApiOperation, ApiTags} from "@nestjs/swagger";
import {Body, Controller, Delete, Get, HttpCode, HttpStatus, Param, ParseIntPipe, Patch, Post, UseGuards} from "@nestjs/common";
import {DeviceService} from "./device.service";
import {CreateDeviceDto} from "./dto/create-device.dto";
import {UpdateDeviceDto} from "./dto/update-device.dto";
import {DeviceDto} from "./dto/device.dto";
import {ApiSuccessResponseDecorator} from "../../common/decorators/api-success-response.decorator";
import {ApiErrorResponsesDecorator} from "../../common/decorators/api-error-response.decorator";
import {JwtAuthGuard} from "../auth/guards/jwt-auth.guard";
import {RolesGuard} from "../auth/guards/roles.guard";
import {Roles} from "../auth/decorators/roles.decorator";
import {Role} from "@generated/prisma/enums";

@ApiTags('devices')
@Controller('devices')
@UseGuards(JwtAuthGuard)
@ApiBearerAuth()
export class DeviceController {

    constructor(private readonly deviceService: DeviceService) {
    }

    @Post()
    @UseGuards(RolesGuard)
    @Roles(Role.ADMIN)
    @ApiOperation({summary: 'Create a device'})
    @ApiBody({type: CreateDeviceDto})
    @ApiSuccessResponseDecorator(DeviceDto, 201, 'Successfully created device')
    @ApiErrorResponsesDecorator(401, 403, 404, 409, 400)
    async createDevice(@Body() dto: CreateDeviceDto) {
        const data = await this.deviceService.createDevice(dto);
        return {message: 'Successfully created device', data};
    }

    @Get()
    @ApiOperation({summary: 'Get all devices'})
    @ApiSuccessResponseDecorator(DeviceDto, 200, 'Devices fetched successfully')
    @ApiErrorResponsesDecorator(401)
    async findAll() {
        const data = await this.deviceService.findAll();
        return {message: 'Devices fetched successfully', data};
    }

    @Get(':id')
    @ApiOperation({summary: 'Get a device by ID'})
    @ApiSuccessResponseDecorator(DeviceDto, 200, 'Device fetched successfully')
    @ApiErrorResponsesDecorator(401, 404)
    async findById(@Param('id', ParseIntPipe) id: number) {
        const data = await this.deviceService.findById(id);
        return {message: 'Device fetched successfully', data};
    }

    @Patch(':id')
    @UseGuards(RolesGuard)
    @Roles(Role.ADMIN)
    @ApiOperation({summary: 'Update a device'})
    @ApiBody({type: UpdateDeviceDto})
    @ApiSuccessResponseDecorator(DeviceDto, 200, 'Device updated successfully')
    @ApiErrorResponsesDecorator(401, 403, 404, 400)
    async updateDevice(
        @Param('id', ParseIntPipe) id: number,
        @Body() dto: UpdateDeviceDto,
    ) {
        const data = await this.deviceService.updateDevice(id, dto);
        return {message: 'Device updated successfully', data};
    }

    @Delete(':id')
    @HttpCode(HttpStatus.NO_CONTENT)
    @UseGuards(RolesGuard)
    @Roles(Role.ADMIN)
    @ApiOperation({summary: 'Delete a device'})
    @ApiErrorResponsesDecorator(401, 403, 404)
    async deleteDevice(@Param('id', ParseIntPipe) id: number): Promise<void> {
        await this.deviceService.deleteDevice(id);
    }

    @Post(':id/api-key')
    @UseGuards(RolesGuard)
    @Roles(Role.ADMIN)
    @ApiOperation({summary: 'Generate or rotate the API key for a device. The key is only returned once.'})
    @ApiErrorResponsesDecorator(401, 403, 404)
    async generateApiKey(@Param('id', ParseIntPipe) id: number) {
        const result = await this.deviceService.generateApiKey(id);
        return {message: 'API key generated. Store it securely — it cannot be retrieved again.', data: result};
    }
}
