import {Module} from '@nestjs/common';
import {UserModule} from './modules/user/user.module';
import {HealthModule} from './modules/health/health.module';
import {AuthModule} from './modules/auth/auth.module';
import {ConfigModule} from "@nestjs/config";
import {HardwareTypeModule} from "./modules/hardware-type/hardware-type.module";
import {DeviceModule} from "./modules/device/device.module";
import {LogModule} from "./modules/log/log.module";
import {DeviceStateModule} from "./modules/device-state/device-state.module";
import {TelemetryModule} from "./modules/telemetry/telemetry.module";

@Module({
    imports: [
        AuthModule,
        TelemetryModule,
        LogModule,
        HardwareTypeModule,
        DeviceStateModule,
        DeviceModule,
        UserModule,
        HealthModule,
        ConfigModule.forRoot({
            isGlobal: true,
        }),],
})
export class AppModule {
}
