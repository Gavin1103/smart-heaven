import { ApiProperty } from '@nestjs/swagger';

export class JwtResponseDto {
  @ApiProperty()
  token: string;
}
