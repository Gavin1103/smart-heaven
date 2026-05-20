import {Module} from '@nestjs/common';
import {DeviceStateController} from "./device-state.controller";
import {DeviceStateService} from "./device-state.service";
import {DeviceStateRepository} from "./device-state.repository";
import {DeviceModule} from "../device/device.module";

@Module({
    imports: [DeviceModule],
    controllers: [DeviceStateController],
    providers: [DeviceStateService, DeviceStateRepository],
})
export class DeviceStateModule {
}
