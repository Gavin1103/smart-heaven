import { Injectable } from '@nestjs/common';
import { AuthGuard } from '@nestjs/passport';

/**
 * Guard for device API key authentication.
 * Reads the X-Api-Key header and validates it against the device's stored hash.
 */
@Injectable()
export class ApiKeyGuard extends AuthGuard('api-key') {}
