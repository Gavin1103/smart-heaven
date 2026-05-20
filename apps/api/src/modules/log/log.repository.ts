import {Injectable} from '@nestjs/common';
import {CreateLogDto} from './dto/create-log.dto';
import {PrismaClient} from "@generated/prisma/client";

@Injectable()
export class LogRepository {

    private readonly prisma = new PrismaClient();

    create(dto: CreateLogDto) {
        return this.prisma.log.create({
            data: {
                deviceId: dto.deviceId,
                level: dto.level,
                eventType: dto.eventType,
                message: dto.message,
                payload: dto.payload
            },
        });
    }

    findMany(
        page: number,
        limit: number,
        deviceId?: number,
    ) {
        const skip = (page - 1) * limit;

        return this.prisma.log.findMany({
            where: {
                ...(deviceId && {deviceId}),
            },
            orderBy: {
                createdAt: 'desc',
            },
            skip,
            take: limit,
        });
    }

    count(deviceId?: number) {
        return this.prisma.log.count({
            where: {
                ...(deviceId && {deviceId}),
            },
        });
    }
}