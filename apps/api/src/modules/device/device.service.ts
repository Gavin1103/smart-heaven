import {DeviceRepository} from "./device.repository";
import {CreateDeviceDto} from "./dto/create-device.dto";
import {UpdateDeviceDto} from "./dto/update-device.dto";
import {HardwareTypeRepository} from "../hardware-type/hardware-type.repository";
import {ConflictException, Injectable, NotFoundException} from "@nestjs/common";
import {HardwareType} from "@generated/prisma/client";
import * as bcrypt from 'bcrypt';
import * as crypto from 'crypto';

@Injectable()
export class DeviceService {
    constructor(
        private readonly deviceRepository: DeviceRepository,
        private readonly hardwareTypeRepository: HardwareTypeRepository
    ) {
    }

    async findAll() {
        return this.deviceRepository.findAll();
    }

    async findById(id: number) {
        const device = await this.deviceRepository.findById(id);
        if (!device) throw new NotFoundException(`Device with id: ${id} not found`);
        return device;
    }

    async createDevice(dto: CreateDeviceDto) {
        const hardwareType: HardwareType | null = await this.hardwareTypeRepository.findByKey(dto.hardwareTypeKey);

        if (!hardwareType) {
            throw new NotFoundException(`Hardware type with key: ${dto.hardwareTypeKey} does not exist`);
        }

        const doesCodeExist = await this.deviceRepository.doesCodeAlreadyExist(dto.code);
        if (doesCodeExist) {
            throw new ConflictException(`Device with code: ${dto.code} already exists`);
        }

        return this.deviceRepository.createDevice(dto, hardwareType.id);
    }

    async updateDevice(id: number, dto: UpdateDeviceDto) {
        const device = await this.deviceRepository.findById(id);
        if (!device) throw new NotFoundException(`Device with id: ${id} not found`);
        return this.deviceRepository.updateDevice(id, dto);
    }

    async deleteDevice(id: number): Promise<void> {
        const device = await this.deviceRepository.findById(id);
        if (!device) throw new NotFoundException(`Device with id: ${id} not found`);
        await this.deviceRepository.deleteDevice(id);
    }

    async generateApiKey(id: number): Promise<{ apiKey: string }> {
        const device = await this.deviceRepository.findById(id);
        if (!device) throw new NotFoundException(`Device with id: ${id} not found`);

        const rawKey = `${device.code}.${crypto.randomBytes(32).toString('hex')}`;
        const hash = await bcrypt.hash(rawKey, 10);
        await this.deviceRepository.setApiKeyHash(id, hash);

        return { apiKey: rawKey };
    }
}