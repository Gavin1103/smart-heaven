import {PrismaClient, Role} from "@generated/prisma/client";

const prisma = new PrismaClient();
import * as bcrypt from 'bcrypt';

export async function seedUsers() {
    const admin = await prisma.user.upsert({
        where: { email: 'admin@admin.com' },
        update: {},
        create: {
            email: 'admin@admin.com',
            password: await bcrypt.hash('admin', 10),
            firstName: 'admin',
            lastName: 'admin',
            role: Role.ADMIN,
        },
    });

    const user = await prisma.user.upsert({
        where: { email: 'user@user.com' },
        update: {},
        create: {
            email: 'user@user.com',
            password: await bcrypt.hash('user', 10),
            firstName: 'user',
            lastName: 'user',
            role: Role.USER,
        },
    });

    console.log('Created user with role ADMIN:', admin.email);
    console.log('Created user with role USER:', user.email);
}