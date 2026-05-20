import { applyDecorators } from '@nestjs/common';
import {
  ApiBadRequestResponse,
  ApiConflictResponse,
  ApiUnauthorizedResponse,
  ApiForbiddenResponse,
  ApiNotFoundResponse,
} from '@nestjs/swagger';
import { ApiErrorResponseDto } from '../dto/api-error-response.dto';

const errorDecorators: Record<number, MethodDecorator> = {
  400: ApiBadRequestResponse({ type: ApiErrorResponseDto }),
  401: ApiUnauthorizedResponse({ type: ApiErrorResponseDto }),
  403: ApiForbiddenResponse({ type: ApiErrorResponseDto }),
  404: ApiNotFoundResponse({ type: ApiErrorResponseDto }),
  409: ApiConflictResponse({ type: ApiErrorResponseDto }),
};

export const ApiErrorResponsesDecorator = (...codes: number[]) =>
  applyDecorators(...codes.map((code) => errorDecorators[code]));
