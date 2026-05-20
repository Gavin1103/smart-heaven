import { Module } from '@nestjs/common';
import { LogController } from './log.controller';
import { LogService } from './log.service';
import { LogRepository } from './log.repository';
import { DeviceModule } from '../device/device.module';

@Module({
    imports: [DeviceModule],
    controllers: [LogController],
    providers: [LogService, LogRepository],
})
export class LogModule {}