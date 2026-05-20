import {PrismaClient} from "@generated/prisma/client";
import {CreateDeviceDto} from "./dto/create-device.dto";
import {UpdateDeviceDto} from "./dto/update-device.dto";

export class DeviceRepository {
    private readonly prisma = new PrismaClient();

    public async findAll() {
        return this.prisma.device.findMany({ include: { hardwareType: true } });
    }

    public async createDevice(dto: CreateDeviceDto, hardwareTypeId: number) {
        return this.prisma.device.create({
            data: {
                name: dto.name,
                code: dto.code,
                description: dto.description,
                locationName: dto.locationName,
                hardwareType: {
                    connect: {
                        id: hardwareTypeId,
                    },
                },
            },
            include: { hardwareType: true },
        });
    }

    public async findDeviceByCode(code: string) {
        return this.prisma.device.findUnique({
            where: { code },
            include: { hardwareType: true },
        });
    }

    public async findById(id: number) {
        return this.prisma.device.findUnique({
            where: { id },
            include: { hardwareType: true },
        });
    }

    public async doesCodeAlreadyExist(code: string) {
        return (await this.prisma.device.findUnique({ where: { code } })) !== null;
    }

    public async updateDevice(id: number, dto: UpdateDeviceDto) {
        return this.prisma.device.update({
            where: { id },
            data: dto,
            include: { hardwareType: true },
        });
    }

    public async deleteDevice(id: number) {
        return this.prisma.device.delete({ where: { id } });
    }

    public async setApiKeyHash(id: number, apiKeyHash: string) {
        return this.prisma.device.update({
            where: { id },
            data: { apiKeyHash },
        });
    }
}