import { IsEmail, IsString } from 'class-validator';
import { ApiProperty } from '@nestjs/swagger';

export class LoginRequestDto {
  @ApiProperty({default: "admin@admin.com"})
  @IsEmail()
  email: string;

  @ApiProperty({default: "admin"})
  @IsString()
  password: string;
}
