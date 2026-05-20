import {Injectable, NotFoundException} from '@nestjs/common';
import {DeviceStateRepository} from './device-state.repository';
import {DeviceRepository} from '../device/device.repository';
import {UpsertDeviceStateDto} from "./dto/upsert-device-state.dto";

@Injectable()
export class DeviceStateService {
    constructor(
        private readonly deviceStateRepository: DeviceStateRepository,
        private readonly deviceRepository: DeviceRepository,
    ) {
    }

    async getByDeviceId(deviceId: number) {
        const deviceState = await this.deviceStateRepository.findByDeviceId(deviceId);

        if (!deviceState) {
            throw new NotFoundException('Device state not found');
        }

        return deviceState;
    }

    async upsertDeviceState(dto: UpsertDeviceStateDto) {
        const device = await this.deviceRepository.findById(dto.deviceId);

        if (!device) {
            throw new NotFoundException('Device not found');
        }

        return this.deviceStateRepository.upsertWithLog(dto);
    }
}