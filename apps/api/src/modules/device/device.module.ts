import { Module } from '@nestjs/common';
import {DeviceController} from "./device.controller";
import {DeviceService} from "./device.service";
import {DeviceRepository} from "./device.repository";
import {HardwareTypeModule} from "../hardware-type/hardware-type.module";

@Module({
    imports: [HardwareTypeModule],
    controllers: [DeviceController],
    providers: [
        DeviceService,
        DeviceRepository,
    ],
    exports: [DeviceRepository],
})
export class DeviceModule {}
