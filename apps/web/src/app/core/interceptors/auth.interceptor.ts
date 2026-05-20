import { HttpInterceptorFn } from '@angular/common/http';

/**
 * Auth Interceptor
 *
 * Automatically attaches a Bearer token to outgoing HTTP requests.
 *
 * Behavior:
 * - Adds Authorization header if a token exists in localStorage
 * - Skips authentication for public endpoints (e.g. login)
 *
 * This interceptor is responsible ONLY for attaching tokens.
 * Error handling and token refresh should be handled separately.
 */
export const authInterceptor: HttpInterceptorFn = (req, next) => {
  const token = localStorage.getItem('token');

  /**
   * Endpoints that should NOT include an Authorization header
   */
  const publicEndpoints = [
    '/auth/login',
  ];

  const isPublic = publicEndpoints.some((url) => req.url.includes(url));

  /**
   * If request is public OR no token exists → pass through
   */
  if (!token || isPublic) {
    return next(req);
  }

  /**
   * Clone request and attach Authorization header
   */
  const authReq = req.clone({
    setHeaders: {
      Authorization: `Bearer ${token}`,
    },
  });

  return next(authReq);
};
