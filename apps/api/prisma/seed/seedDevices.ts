import {PrismaClient, DeviceStatus} from '@generated/prisma/client';

const prisma = new PrismaClient();

export async function seedDevices() {
    console.log('🌱 Seeding devices...');

    const trafficLightType = await prisma.hardwareType.findUnique({
        where: {key: "TRAFFIC_LIGHT"},
    });

    const railwayType = await prisma.hardwareType.findUnique({
        where: {key: "RAILWAY_BARRIER"},
    });

    const bridgeType = await prisma.hardwareType.findUnique({
        where: {key: "BRIDGE"},
    });

    if (!trafficLightType || !railwayType || !bridgeType) {
        throw new Error('Hardware types must be seeded first');
    }

    const devices = [
        // 🚦 Traffic lights
        {name: 'Traffic Light A', code: 'TL-01', hardwareTypeId: trafficLightType.id},
        {name: 'Traffic Light B', code: 'TL-02', hardwareTypeId: trafficLightType.id},
        {name: 'Traffic Light C', code: 'TL-03', hardwareTypeId: trafficLightType.id},

        // 🚧 Railway barrier
        {name: 'Railway Barrier A', code: 'RB-01', hardwareTypeId: railwayType.id},

        {name: 'Bridge A', code: 'B-01', hardwareTypeId: bridgeType.id},
        {name: 'Bridge B', code: 'B-02', hardwareTypeId: bridgeType.id},
    ];

    for (const device of devices) {
        await prisma.device.upsert({
            where: {code: device.code},
            update: {},
            create: {
                ...device,
                status: DeviceStatus.ONLINE,
            },
        });
    }

    console.log('✅ Devices seeded');
}