import {
  ArgumentsHost,
  Catch,
  ExceptionFilter,
  HttpException,
  HttpStatus,
} from '@nestjs/common';
import { Response } from 'express';
import {ApiErrorResponseDto} from "../dto/api-error-response.dto";

/**
 * Global exception filter that catches all exceptions and returns a consistent
 * ApiErrorResponse envelope for every error response.
 *
 * Handles two cases:
 * - HttpException: maps the exception status and message to the error response.
 * - Unknown errors: returns a generic 500 Internal Server Error response.
 *
 * Every outgoing error response will have the following structure:
 * {
 *   timestamp: string,   // ISO 8601 date string of when the error occurred
 *   status: number,      // HTTP status code
 *   success: false,      // always false for error responses
 *   message: string,     // human-readable error message
 *   errors?: string[]    // list of validation errors, only present on 400 responses
 * }
 */
@Catch()
export class HttpExceptionFilter implements ExceptionFilter {
  catch(exception: unknown, host: ArgumentsHost): void {
    const ctx = host.switchToHttp();
    const response = ctx.getResponse<Response>();

    let status = HttpStatus.INTERNAL_SERVER_ERROR;
    let errorResponse: ApiErrorResponseDto = {
      timestamp: new Date().toISOString(),
      status,
      success: false,
      message: 'Internal server error',
    };

    if (exception instanceof HttpException) {
      status = exception.getStatus();
      const exceptionResult = exception.getResponse();

      if (typeof exceptionResult === 'string') {
        /** HttpException was thrown with a plain string message. */
        errorResponse = {
          timestamp: new Date().toISOString(),
          status,
          success: false,
          message: exceptionResult,
        };
      } else if (
        typeof exceptionResult === 'object' &&
        exceptionResult !== null
      ) {
        const result = exceptionResult as {
          message?: string | string[];
          error?: string;
        };

        if (Array.isArray(result.message)) {
          /**
           * Validation pipe throws an array of messages (e.g. class-validator errors).
           * Each entry in the array describes a specific field validation failure.
           */
          errorResponse = {
            timestamp: new Date().toISOString(),
            status,
            success: false,
            message: 'Validation failed',
            errors: result.message,
          };
        } else {
          /** HttpException was thrown with an object containing a single message or error. */
          errorResponse = {
            timestamp: new Date().toISOString(),
            status,
            success: false,
            message: result.message || result.error || 'Request failed',
          };
        }
      }
    } else {
      // Log unexpected errors for debugging
      console.error('Unexpected error:', exception);
    }

    response.status(status).json(errorResponse);
  }
}
