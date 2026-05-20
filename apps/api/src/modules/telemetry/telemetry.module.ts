import {TelemetryController} from "./telemetry.controller";
import {TelemetryService} from "./telemetry.service";
import {TelemetryRepository} from "./telemetry.repository";
import {Module} from "@nestjs/common";
import {DeviceModule} from "../device/device.module";

@Module({
    imports: [DeviceModule],
    controllers: [TelemetryController],
    providers: [TelemetryService, TelemetryRepository,],
})
export class TelemetryModule {}
