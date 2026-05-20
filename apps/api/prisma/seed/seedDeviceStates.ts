import { PrismaClient } from '@generated/prisma/client';

const prisma = new PrismaClient();

export async function seedDeviceStates() {
    console.log('🌱 Seeding device states...');

    const devices = await prisma.device.findMany();

    for (const device of devices) {
        let state = 'UNKNOWN';

        if (device.code.startsWith('TL')) state = 'RED';
        if (device.code.startsWith('RB')) state = 'OPEN';
        if (device.code.startsWith('PS')) state = 'FREE';

        await prisma.deviceState.upsert({
            where: { deviceId: device.id },
            update: {},
            create: {
                deviceId: device.id,
                state,
                statusMessage: 'Initial state',
                data: {},
            },
        });
    }

    console.log('✅ Device states seeded');
}