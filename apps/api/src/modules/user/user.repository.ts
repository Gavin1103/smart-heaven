import { Injectable } from '@nestjs/common';
import { PrismaClient, Role, User } from '@generated/prisma/client';
import { CreateUserRequestDto } from './dto/create-user-request.dto';
import { UpdateUserDto } from './dto/update-user.dto';

@Injectable()
export class UserRepository {
  prisma = new PrismaClient();

  findAll(): Promise<User[]> {
    return this.prisma.user.findMany();
  }

  findById(id: number): Promise<User | null> {
    return this.prisma.user.findUnique({ where: { id } });
  }

  findByEmail(email: string): Promise<User | null> {
    return this.prisma.user.findUnique({
      where: { email },
    });
  }

  createUser(data: CreateUserRequestDto): Promise<User> {
    return this.prisma.user.create({
      data: {
        email: data.email,
        password: data.password,
        firstName: data.firstName,
        lastName: data.lastName,
        role: Role.USER,
      },
    });
  }

  updateUser(id: number, data: UpdateUserDto): Promise<User> {
    return this.prisma.user.update({ where: { id }, data });
  }

  deleteUser(id: number): Promise<User> {
    return this.prisma.user.delete({ where: { id } });
  }
}
