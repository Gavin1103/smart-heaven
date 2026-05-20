import { applyDecorators, Type } from '@nestjs/common';
import { ApiExtraModels, ApiResponse, getSchemaPath } from '@nestjs/swagger';
import { ApiResponseDto } from '../dto/api-response.dto';

export const ApiSuccessResponseDecorator = <T extends Type<unknown>>(
    model?: T,
    status: number = 200,
    description: string = 'Success',
) => {
    if (!model || status === 204) {
        return applyDecorators(
            ApiResponse({
                status,
                description,
            }),
        );
    }

    return applyDecorators(
        ApiExtraModels(ApiResponseDto, model),
        ApiResponse({
            status,
            description,
            schema: {
                allOf: [
                    { $ref: getSchemaPath(ApiResponseDto) },
                    {
                        properties: {
                            data: { $ref: getSchemaPath(model) },
                        },
                        required: ['data'],
                    },
                ],
            },
        }),
    );
};