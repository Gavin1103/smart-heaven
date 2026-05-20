import {ConflictException, Injectable, NotFoundException} from "@nestjs/common";
import {HardwareTypeRepository} from "./hardware-type.repository";
import {CreateHardwareTypeDto} from "./dto/create-hardware-type";
import {UpdateHardwareTypeDto} from "./dto/update-hardware-type.dto";

@Injectable()
export class HardwareTypeService {
    constructor(private readonly hardwareTypeRepository: HardwareTypeRepository) {
    }

    async findAll() {
        return this.hardwareTypeRepository.findAll();
    }

    async findByKey(key: string) {
        const hardwareType = await this.hardwareTypeRepository.findByKey(key);
        if (!hardwareType) throw new NotFoundException(`Hardware type with key: ${key} not found`);
        return hardwareType;
    }

    async createHardwareType(dto: CreateHardwareTypeDto) {
        const doesKeyExist = await this.hardwareTypeRepository.doesKeyExist(dto.key);
        if (doesKeyExist) throw new ConflictException(`Key: ${dto.key} already exists`);
        return this.hardwareTypeRepository.createHardwareType(dto);
    }

    async updateHardwareType(key: string, dto: UpdateHardwareTypeDto) {
        const hardwareType = await this.hardwareTypeRepository.findByKey(key);
        if (!hardwareType) throw new NotFoundException(`Hardware type with key: ${key} not found`);
        return this.hardwareTypeRepository.updateHardwareType(key, dto);
    }

    async deleteHardwareType(key: string): Promise<void> {
        const hardwareType = await this.hardwareTypeRepository.findByKey(key);
        if (!hardwareType) throw new NotFoundException(`Hardware type with key: ${key} not found`);
        await this.hardwareTypeRepository.deleteHardwareType(key);
    }
}