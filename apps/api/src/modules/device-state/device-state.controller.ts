import {Body, Controller, Get, HttpCode, HttpStatus, Param, ParseIntPipe, Put, UseGuards} from '@nestjs/common';
import {ApiBearerAuth, ApiBody, ApiHeader, ApiOperation, ApiTags} from '@nestjs/swagger';
import {DeviceStateService} from './device-state.service';
import {UpsertDeviceStateDto} from './dto/upsert-device-state.dto';
import {JwtAuthGuard} from '../auth/guards/jwt-auth.guard';
import {JwtOrApiKeyGuard} from '../auth/guards/jwt-or-api-key.guard';

@ApiTags('device-state')
@Controller('device-state')
@UseGuards(JwtAuthGuard)
@ApiBearerAuth()
export class DeviceStateController {
    constructor(private readonly deviceStateService: DeviceStateService) {
    }

    @Get('/:deviceId')
    @ApiOperation({summary: 'Get device state by device ID'})
    getByDeviceId(@Param('deviceId', ParseIntPipe) deviceId: number) {
        return this.deviceStateService.getByDeviceId(deviceId);
    }

    @Put('/:deviceId')
    @HttpCode(HttpStatus.OK)
    @UseGuards(JwtOrApiKeyGuard)
    @ApiHeader({name: 'X-Api-Key', description: 'Device API key (alternative to Bearer JWT)', required: false})
    @ApiOperation({summary: 'Create or update device state'})
    @ApiBody({type: UpsertDeviceStateDto})
    upsertDeviceState(
        @Param('deviceId', ParseIntPipe) deviceId: number,
        @Body() dto: Omit<UpsertDeviceStateDto, 'deviceId'>,
    ) {
        return this.deviceStateService.upsertDeviceState({...dto, deviceId});
    }
}
