import { PrismaClient, LogLevel } from '@generated/prisma/client';

const prisma = new PrismaClient();

export async function seedLogs() {
    console.log('🌱 Seeding logs...');

    const devices = await prisma.device.findMany();

    for (const device of devices) {
        await prisma.log.createMany({
            data: [
                {
                    deviceId: device.id,
                    level: LogLevel.INFO,
                    eventType: 'DEVICE_STARTED',
                    message: `${device.code} started successfully`,
                    payload: {},
                },
                {
                    deviceId: device.id,
                    level: LogLevel.INFO,
                    eventType: 'STATUS_UPDATE',
                    message: `${device.code} sent status update`,
                    payload: { uptime: Math.floor(Math.random() * 1000) },
                },
            ],
        });

        // random error (nice realism)
        if (Math.random() > 0.7) {
            await prisma.log.create({
                data: {
                    deviceId: device.id,
                    level: LogLevel.ERROR,
                    eventType: 'RANDOM_ERROR',
                    message: `${device.code} encountered an error`,
                    payload: { code: 500 },
                },
            });
        }
    }

    console.log('✅ Logs seeded');
}