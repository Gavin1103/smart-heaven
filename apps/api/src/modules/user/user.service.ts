import {
    ConflictException,
    Injectable,
    NotFoundException,
} from '@nestjs/common';
import {CreateUserRequestDto} from './dto/create-user-request.dto';
import {UpdateUserDto} from './dto/update-user.dto';
import {UserRepository} from './user.repository';
import {User} from '@generated/prisma/client';
import {UserResponseDto} from "./dto/user-response.dto";
import * as bcrypt from 'bcrypt';

@Injectable()
export class UserService {
    constructor(private readonly userRepository: UserRepository) {
    }

    async findAll(): Promise<UserResponseDto[]> {
        const users = await this.userRepository.findAll();
        return users.map(this.toUserResponse);
    }

    async findById(id: number): Promise<UserResponseDto> {
        const user = await this.userRepository.findById(id);
        if (!user) throw new NotFoundException('User not found');
        return this.toUserResponse(user);
    }

    async me(userEmail: string): Promise<UserResponseDto> {
        const user = await this.userRepository.findByEmail(userEmail);
        if (!user) throw new NotFoundException('User not found');
        return this.toUserResponse(user);
    }

    async createUser(createUserDto: CreateUserRequestDto): Promise<UserResponseDto> {
        const existingUser: User | null = await this.userRepository.findByEmail(
            createUserDto.email,
        );
        if (existingUser) {
            throw new ConflictException('Email already in use');
        }
        const hashedPassword = await bcrypt.hash(createUserDto.password, 10);
        const createdUser: User = await this.userRepository.createUser({
            email: createUserDto.email,
            firstName: createUserDto.firstName,
            lastName: createUserDto.lastName,
            password: hashedPassword,
        });
        return this.toUserResponse(createdUser);
    }

    async updateUser(id: number, dto: UpdateUserDto): Promise<UserResponseDto> {
        const user = await this.userRepository.findById(id);
        if (!user) throw new NotFoundException('User not found');

        const data: UpdateUserDto = { ...dto };
        if (dto.password) {
            data.password = await bcrypt.hash(dto.password, 10);
        }

        const updated = await this.userRepository.updateUser(id, data);
        return this.toUserResponse(updated);
    }

    async deleteUser(id: number): Promise<void> {
        const user = await this.userRepository.findById(id);
        if (!user) throw new NotFoundException('User not found');
        await this.userRepository.deleteUser(id);
    }

    private toUserResponse(user: User): UserResponseDto {
        return {
            id: user.id,
            email: user.email,
            firstName: user.firstName,
            lastName: user.lastName,
        } satisfies UserResponseDto;
    }
}
