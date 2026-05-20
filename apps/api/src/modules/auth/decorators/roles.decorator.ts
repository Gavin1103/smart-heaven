import { SetMetadata } from '@nestjs/common';
import { Role } from '@generated/prisma/enums';

/**
 * Metadata key used to store role requirements on routes and controllers.
 */
export const ROLES_KEY = 'roles';

/**
 * Decorator used to restrict access to specific user roles.
 *
 * This decorator attaches the allowed roles as metadata,
 * which can then be read by the `RolesGuard`.
 *
 * Example:
 * ```ts
 * @UseGuards(JwtAuthGuard, RolesGuard)
 * @Roles(Role.ADMIN)
 * @Get('admin-only')
 * getAdminResource() {
 *   return 'Only admins can access this route';
 * }
 * ```
 *
 * @param roles The roles allowed to access the route
 * @returns A metadata decorator containing the allowed roles
 */
export const Roles = (...roles: Role[]) => SetMetadata(ROLES_KEY, roles);
