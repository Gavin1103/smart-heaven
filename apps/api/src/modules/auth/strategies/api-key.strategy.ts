import { Injectable, UnauthorizedException } from '@nestjs/common';
import { PassportStrategy } from '@nestjs/passport';
import { Strategy } from 'passport-custom';
import { Request } from 'express';
import { PrismaClient } from '@generated/prisma/client';
import * as bcrypt from 'bcrypt';

export interface DevicePrincipal {
    deviceId: number;
    deviceCode: string;
    type: 'device';
}

@Injectable()
export class ApiKeyStrategy extends PassportStrategy(Strategy, 'api-key') {
    private readonly prisma = new PrismaClient();

    async validate(req: Request): Promise<DevicePrincipal> {
        const apiKey = req.headers['x-api-key'] as string | undefined;

        if (!apiKey) {
            throw new UnauthorizedException('Missing X-Api-Key header');
        }

        const dotIndex = apiKey.indexOf('.');
        if (dotIndex === -1) {
            throw new UnauthorizedException('Invalid API key format');
        }

        const deviceCode = apiKey.substring(0, dotIndex);

        const device = await this.prisma.device.findUnique({
            where: { code: deviceCode },
        });

        if (!device || !device.apiKeyHash) {
            throw new UnauthorizedException('Invalid API key');
        }

        const isValid = await bcrypt.compare(apiKey, device.apiKeyHash);
        if (!isValid) {
            throw new UnauthorizedException('Invalid API key');
        }

        return { deviceId: device.id, deviceCode: device.code, type: 'device' };
    }
}
