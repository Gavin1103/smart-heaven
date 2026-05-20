import {Injectable, NotFoundException} from '@nestjs/common';
import {LogRepository} from './log.repository';
import {CreateLogDto} from './dto/create-log.dto';
import {DeviceRepository} from '../device/device.repository';

@Injectable()
export class LogService {
    constructor(
        private readonly logRepository: LogRepository,
        private readonly deviceRepository: DeviceRepository,
    ) {
    }

    async createLog(dto: CreateLogDto) {
        const device = await this.deviceRepository.findById(dto.deviceId);

        if (!device) {
            throw new NotFoundException('Device not found');
        }

        return this.logRepository.create(dto);
    }

    async findMany(
        page = 1,
        limit = 25,
        deviceId?: number,
    ) {
        const [logs, total] = await Promise.all([
            this.logRepository.findMany(page, limit, deviceId),
            this.logRepository.count(deviceId),
        ]);

        return {
            data: logs,
            pagination: {
                total,
                page,
                limit,
                totalPages: Math.ceil(total / limit),
            },
        };
    }
}