import { Body, Controller, Post } from '@nestjs/common';
import {
  ApiBody,
  ApiExtraModels,
  ApiOperation,
  ApiTags,
} from '@nestjs/swagger';
import { AuthService } from './auth.service';
import { ApiSuccessResponseDecorator } from '../../common/decorators/api-success-response.decorator';
import { ApiErrorResponsesDecorator } from '../../common/decorators/api-error-response.decorator';
import { ApiControllerResponse } from '../../common/interfaces/api-controller-response.interface';
import { LoginRequestDto } from './dto/login-request.dto';
import { JwtResponseDto } from './dto/jwt-response.dto';

@ApiTags('AuthController')
@Controller('auth')
export class AuthController {
  constructor(private readonly authService: AuthService) {}

  @Post('/login')
  @ApiOperation({ summary: 'Login with email and password' })
  @ApiBody({ type: LoginRequestDto })
  @ApiSuccessResponseDecorator(JwtResponseDto, 200, 'Successfully logged in')
  @ApiErrorResponsesDecorator(401, 409, 400)
  async login(
    @Body() loginDto: LoginRequestDto,
  ): Promise<ApiControllerResponse<JwtResponseDto>> {
    const token = await this.authService.login(loginDto);
    return {
      message: 'Login successful',
      data: token,
    };
  }
}
