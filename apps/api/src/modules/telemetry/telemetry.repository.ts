import {Injectable} from "@nestjs/common";
import {CreateTelemetryDto} from "./dto/create-telemetry.dto";
import {Device, PrismaClient} from "@generated/prisma/client";

@Injectable()
export class TelemetryRepository {
    private readonly prisma = new PrismaClient();

    async createTelemetry(dto: CreateTelemetryDto, device: Device) {
        await this.prisma.$transaction(async (tx) => {
            await tx.deviceState.upsert({
                where: {deviceId: device.id},
                update: {
                    state: dto.state,
                    statusMessage: dto.message,
                    data: dto.data,
                },
                create: {
                    deviceId: device.id,
                    state: dto.state,
                    statusMessage: dto.message,
                    data: dto.data,
                },
            });

            await tx.log.create({
                data: {
                    deviceId: device.id,
                    level: dto.level,
                    eventType: dto.eventType,
                    message: dto.message,
                    payload: dto.data,
                },
            });
        });
    }
}
