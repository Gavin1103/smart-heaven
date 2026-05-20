import {
  CanActivate,
  ExecutionContext,
  Injectable,
  ForbiddenException,
} from '@nestjs/common';
import { Reflector } from '@nestjs/core';
import { ROLES_KEY } from '../decorators/roles.decorator';
import { Request } from 'express';
import { Role } from '@generated/prisma/enums';

/**
 * Extended Express request type containing the authenticated user
 * that was attached by the JWT strategy.
 */
type AuthenticatedRequest = Request & {
  user: {
    email: string;
    role: Role;
  };
};

/**
 * Guard responsible for role-based authorization.
 *
 * This guard reads the roles defined via the `@Roles(...)` decorator
 * and checks whether the authenticated user has one of the required roles.
 *
 * Authorization flow:
 * - JWT guard authenticates the user and attaches `req.user`
 * - RolesGuard reads the required roles from metadata
 * - RolesGuard compares `req.user.role` against the allowed roles
 * - access is granted only if a matching role is found
 */
@Injectable()
export class RolesGuard implements CanActivate {
  constructor(private readonly reflector: Reflector) {}

  /**
   * Determines whether the current request is allowed to proceed
   * based on the roles assigned to the authenticated user.
   *
   * @param context The current execution context
   * @returns `true` when access is allowed
   * @throws ForbiddenException when the user is missing or lacks the required role
   */
  canActivate(context: ExecutionContext): boolean {
    /**
     * Retrieve the required roles from the route handler or controller class.
     * Method-level metadata takes precedence over class-level metadata.
     */
    const requiredRoles = this.reflector.getAllAndOverride<Role[]>(ROLES_KEY, [
      context.getHandler(),
      context.getClass(),
    ]);

    /**
     * If no roles are defined for the route, allow access.
     */
    if (!requiredRoles || requiredRoles.length === 0) {
      return true;
    }

    /**
     * Retrieve the authenticated user from the request.
     * This user is typically attached by the JWT strategy.
     */
    const request = context.switchToHttp().getRequest<AuthenticatedRequest>();
    const user = request.user;

    /**
     * Deny access if no authenticated user is present.
     */
    if (!user) {
      throw new ForbiddenException('User not found in request');
    }

    /**
     * Check whether the authenticated user has at least one
     * of the roles required by the route.
     */
    const hasRequiredRole = requiredRoles.includes(user.role);

    /**
     * Deny access when the user does not have the required role.
     */
    if (!hasRequiredRole) {
      throw new ForbiddenException(
        'You do not have permission to access this resource',
      );
    }

    return true;
  }
}