import {Injectable, NotFoundException} from '@nestjs/common';
import {DeviceRepository} from '../device/device.repository';
import {CreateTelemetryDto} from "./dto/create-telemetry.dto";
import {TelemetryRepository} from "./telemetry.repository";

@Injectable()
export class TelemetryService {
    constructor(
        private readonly deviceRepository: DeviceRepository,
        private readonly telemetryRepository: TelemetryRepository,
    ) {
    }

    async receiveTelemetry(dto: CreateTelemetryDto): Promise<void> {
        const device = await this.deviceRepository.findDeviceByCode(dto.deviceCode);

        if (!device) {
            throw new NotFoundException(`Device ${dto.deviceCode} not found`);
        }

        await this.telemetryRepository.createTelemetry(dto, device)
    }
}