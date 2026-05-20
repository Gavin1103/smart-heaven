import {ApiProperty} from '@nestjs/swagger';
import {DeviceStatus} from '@generated/prisma/enums';
import {HardwareTypeDto} from '../../hardware-type/dto/hardware-type.dto';

export class DeviceDto {
    @ApiProperty({example: 1})
    id: number;

    @ApiProperty({example: 'Bridge - Tile 1'})
    name: string;

    @ApiProperty({example: 'B-T1'})
    code: string;

    @ApiProperty({
        example: 'bridge on tile 1',
        nullable: true,
    })
    description: string | null;

    @ApiProperty({
        example: 'Bridge tile 1',
        nullable: true,
    })
    locationName: string | null;

    @ApiProperty({
        enum: DeviceStatus,
        example: DeviceStatus.ONLINE,
    })
    status: DeviceStatus;

    @ApiProperty({
        example: '2026-05-10T13:00:00.000Z',
        nullable: true,
    })
    lastSeenAt: Date | null;

    @ApiProperty({
        example: '2026-05-10T13:00:00.000Z',
        nullable: true,
    })
    installedAt: Date | null;

    @ApiProperty({
        example: '2026-05-10T13:00:00.000Z',
    })
    createdAt: Date;

    @ApiProperty({
        example: '2026-05-10T13:00:00.000Z',
    })
    updatedAt: Date;

    @ApiProperty({
        type: () => HardwareTypeDto,
    })
    hardwareType: HardwareTypeDto;
}