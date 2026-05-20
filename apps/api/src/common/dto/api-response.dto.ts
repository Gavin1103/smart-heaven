import { ApiProperty } from '@nestjs/swagger';

export class ApiResponseDto<T> {
  @ApiProperty({ example: '2026-03-13T12:00:00.000Z' })
  timestamp: string;

  @ApiProperty({ example: 201 })
  status: number;

  @ApiProperty({ example: true })
  success: true;

  @ApiProperty({ example: 'User successfully created' })
  message: string;

  data: T;
}
