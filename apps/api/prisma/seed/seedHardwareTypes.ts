import {PrismaClient} from "@generated/prisma/client";

const prisma = new PrismaClient();

export async function seedHardwareTypes() {
    console.log('🌱 Seeding hardware types...');

    const types = [
        {key: "RAILWAY_BARRIER", name: 'Railway Barrier'},
        {key: "TRAFFIC_LIGHT", name: 'Traffic Light'},
        {key: "BRIDGE", name: 'Bridge'},
    ];

    for (const type of types) {
        await prisma.hardwareType.upsert({
            where: {key: type.key},
            update: {},
            create: type,
        });
    }

    console.log('✅ Hardware types seeded');
}