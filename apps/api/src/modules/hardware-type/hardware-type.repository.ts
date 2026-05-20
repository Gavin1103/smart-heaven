import {Injectable} from "@nestjs/common";
import {HardwareType, PrismaClient} from '@generated/prisma/client';
import {CreateHardwareTypeDto} from "./dto/create-hardware-type";
import {UpdateHardwareTypeDto} from "./dto/update-hardware-type.dto";

@Injectable()
export class HardwareTypeRepository {
    private readonly prisma = new PrismaClient();

    public async findAll(): Promise<HardwareType[]> {
        return this.prisma.hardwareType.findMany();
    }

    public async createHardwareType(dto: CreateHardwareTypeDto) {
        return this.prisma.hardwareType.create({ data: dto });
    }

    public async findByKey(key: string): Promise<HardwareType | null> {
        return this.prisma.hardwareType.findUnique({ where: { key } });
    }

    public async doesKeyExist(key: string): Promise<boolean> {
        return (await this.prisma.hardwareType.findUnique({ where: { key } })) !== null;
    }

    public async updateHardwareType(key: string, dto: UpdateHardwareTypeDto): Promise<HardwareType> {
        return this.prisma.hardwareType.update({ where: { key }, data: dto });
    }

    public async deleteHardwareType(key: string): Promise<HardwareType> {
        return this.prisma.hardwareType.delete({ where: { key } });
    }
}