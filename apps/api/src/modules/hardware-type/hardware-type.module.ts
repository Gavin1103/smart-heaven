import { Module } from '@nestjs/common';
import {HardwareTypeController} from "./hardware-type.controller";
import {HardwareTypeService} from "./hardware-type.service";
import {HardwareTypeRepository} from "./hardware-type.repository";

@Module({
    controllers: [HardwareTypeController],
    providers: [HardwareTypeService, HardwareTypeRepository],
    exports: [HardwareTypeRepository],
})
export class HardwareTypeModule {}
