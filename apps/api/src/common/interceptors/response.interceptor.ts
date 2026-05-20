import {
  CallHandler,
  ExecutionContext,
  Injectable,
  NestInterceptor,
} from '@nestjs/common';
import { Response } from 'express';
import { Observable } from 'rxjs';
import { map } from 'rxjs/operators';
import { ApiControllerResponse } from '../interfaces/api-controller-response.interface';
import {ApiResponseDto} from "../dto/api-response.dto";

/**
 * Interceptor that wraps all successful HTTP responses in a consistent ApiResponse envelope.
 *
 * Every outgoing response will have the following structure:
 * {
 *   timestamp: string,  // ISO 8601 date string of when the response was sent
 *   status: number,     // HTTP status code
 *   success: true,      // always true for successful responses
 *   message: string,    // descriptive message from the controller or default fallback
 *   data: T             // the actual response payload
 * }
 *
 * If the controller returns an `ApiControllerResponse<T>` (an object with `data` and optionally
 * `message`), the interceptor extracts those fields. Otherwise, the raw return value is used as `data`.
 */
@Injectable()
export class ResponseInterceptor<T> implements NestInterceptor<
  T,
  ApiResponseDto<T>
> {
  intercept(
    context: ExecutionContext,
    next: CallHandler,
  ): Observable<ApiResponseDto<T>> {
    const http = context.switchToHttp();
    const res = http.getResponse<Response>();

    return next.handle().pipe(
      map((response: T | ApiControllerResponse<T>) => {
        const timestamp = new Date().toISOString();
        const status = res.statusCode;

        /**
         * If the controller returned an ApiControllerResponse<T>, unwrap it.
         * This allows controllers to pass a custom message alongside the data.
         */
        if (response && typeof response === 'object' && 'data' in response) {
          const typedResponse = response;

          return {
            timestamp,
            status,
            success: true,
            message: typedResponse.message ?? 'Request successful',
            data: typedResponse.data,
          };
        }

        /** Fallback: wrap the raw response value directly as data. */
        return {
          timestamp,
          status,
          success: true,
          message: 'Request successful',
          data: response,
        };
      }),
    );
  }
}
