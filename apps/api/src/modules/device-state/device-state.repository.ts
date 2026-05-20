import {Injectable} from '@nestjs/common';
import {PrismaClient} from "@generated/prisma/client";
import {UpsertDeviceStateDto} from "./dto/upsert-device-state.dto";

@Injectable()
export class DeviceStateRepository {
    private readonly prisma = new PrismaClient();

    findByDeviceId(deviceId: number) {
        return this.prisma.deviceState.findUnique({
            where: {deviceId},
        });
    }

    async upsertWithLog(dto: UpsertDeviceStateDto) {
        return this.prisma.$transaction(async (tx) => {
            const deviceState = await tx.deviceState.upsert({
                where: { deviceId: dto.deviceId },
                update: {
                    state: dto.state,
                    statusMessage: dto.statusMessage,
                    data: dto.data,
                },
                create: {
                    deviceId: dto.deviceId,
                    state: dto.state,
                    statusMessage: dto.statusMessage,
                    data: dto.data,
                },
            });

            await tx.log.create({
                data: {
                    deviceId: dto.deviceId,
                    level: 'INFO',
                    eventType: 'DEVICE_STATE_CHANGED',
                    message: `Device state changed to ${dto.state}`,
                    payload: {
                        state: dto.state,
                        statusMessage: dto.statusMessage,
                        data: dto.data,
                    },
                },
            });

            return deviceState;
        });
    }
}