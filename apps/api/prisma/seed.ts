import {PrismaClient} from '@generated/prisma/client';
import {seedLogs} from "./seed/seedLogs";
import {seedHardwareTypes} from "./seed/seedHardwareTypes";
import {seedDevices} from "./seed/seedDevices";
import {seedDeviceStates} from "./seed/seedDeviceStates";
import 'dotenv/config'
import {seedUsers} from "./seed/seedUsers";

const prisma = new PrismaClient();

async function main() {
    await seedUsers();
    await seedHardwareTypes();
    await seedDevices();
    // await seedDeviceStates();
    // await seedLogs();
}

main()
    .catch((e) => {
        console.error(e);
        process.exit(1);
    })
    .finally(async () => {
        await prisma.$disconnect();
    });
