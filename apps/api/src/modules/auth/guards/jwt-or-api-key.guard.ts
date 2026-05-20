import { Injectable } from '@nestjs/common';
import { AuthGuard } from '@nestjs/passport';

/**
 * Guard that accepts either a valid JWT (user) or a valid device API key.
 * Used on endpoints that must be accessible by both users and embedded devices.
 */
@Injectable()
export class JwtOrApiKeyGuard extends AuthGuard(['jwt', 'api-key']) {}
