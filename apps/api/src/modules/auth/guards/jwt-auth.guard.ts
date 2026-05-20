import { Injectable } from '@nestjs/common';
import { AuthGuard } from '@nestjs/passport';

/**
 * Guard responsible for JWT-based authentication.
 *
 * This guard uses the registered `jwt` Passport strategy to:
 * - extract the bearer token from the Authorization header
 * - validate the token signature and expiration
 * - attach the validated payload to `req.user`
 *
 * Requests without a valid JWT will be rejected with `401 Unauthorized`.
 */
@Injectable()
export class JwtAuthGuard extends AuthGuard('jwt') {}
