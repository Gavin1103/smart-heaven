import { ApiProperty, ApiPropertyOptional } from '@nestjs/swagger';

export class ApiErrorResponseDto {
  @ApiProperty({ example: '2026-03-13T12:00:00.000Z' })
  timestamp: string;

  @ApiProperty()
  status: number;

  @ApiProperty({ example: false })
  success: false;

  @ApiProperty({ example: 'message' })
  message: string;

  @ApiPropertyOptional({
    type: [String],
    example: ['error message'],
  })
  errors?: string[];
}
