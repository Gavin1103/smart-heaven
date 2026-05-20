import { Body, Controller, Delete, Get, HttpCode, HttpStatus, Param, ParseIntPipe, Patch, Post, Req, UseGuards } from '@nestjs/common';
import { ApiBody, ApiOperation, ApiTags, ApiBearerAuth } from '@nestjs/swagger';
import { CreateUserRequestDto } from './dto/create-user-request.dto';
import { UpdateUserDto } from './dto/update-user.dto';
import { UserResponseDto } from './dto/user-response.dto';
import { UserService } from './user.service';
import { ApiControllerResponse } from '../../common/interfaces/api-controller-response.interface';
import { ApiSuccessResponseDecorator } from '../../common/decorators/api-success-response.decorator';
import { ApiErrorResponsesDecorator } from '../../common/decorators/api-error-response.decorator';
import { JwtAuthGuard } from '../auth/guards/jwt-auth.guard';
import { RolesGuard } from '../auth/guards/roles.guard';
import { Roles } from '../auth/decorators/roles.decorator';
import { Role } from '@generated/prisma/enums';

@ApiTags('users')
@Controller('users')
export class UserController {
  constructor(private readonly userService: UserService) {}

  @Post()
  @UseGuards(JwtAuthGuard)
  @ApiBearerAuth()
  @ApiOperation({ summary: 'Create a new user' })
  @ApiBody({ type: CreateUserRequestDto })
  @ApiSuccessResponseDecorator(UserResponseDto, 201, 'User successfully created')
  @ApiErrorResponsesDecorator(401, 403, 404, 409, 400)
  async createUser(
    @Body() createUserDto: CreateUserRequestDto,
  ): Promise<ApiControllerResponse<UserResponseDto>> {
    const createdUser = await this.userService.createUser(createUserDto);
    return { message: 'User successfully created', data: createdUser };
  }

  @Get()
  @UseGuards(JwtAuthGuard, RolesGuard)
  @Roles(Role.ADMIN)
  @ApiBearerAuth()
  @ApiOperation({ summary: 'Get all users' })
  @ApiSuccessResponseDecorator(UserResponseDto, 200, 'Users fetched successfully')
  @ApiErrorResponsesDecorator(401, 403)
  async findAll(): Promise<ApiControllerResponse<UserResponseDto[]>> {
    const users = await this.userService.findAll();
    return { message: 'Users fetched successfully', data: users };
  }

  @Get('me')
  @UseGuards(JwtAuthGuard)
  @ApiBearerAuth()
  @ApiOperation({ summary: 'Fetch info of the logged in user' })
  @ApiSuccessResponseDecorator(UserResponseDto, 200, 'User fetched successfully')
  @ApiErrorResponsesDecorator(401, 404)
  async me(@Req() req: Request): Promise<ApiControllerResponse<UserResponseDto>> {
    const jwtUser = (req as Request & { user: { email: string } }).user;
    const user = await this.userService.me(jwtUser.email);
    return { message: 'User fetched successfully', data: user };
  }

  @Get(':id')
  @UseGuards(JwtAuthGuard, RolesGuard)
  @Roles(Role.ADMIN)
  @ApiBearerAuth()
  @ApiOperation({ summary: 'Get a user by ID' })
  @ApiSuccessResponseDecorator(UserResponseDto, 200, 'User fetched successfully')
  @ApiErrorResponsesDecorator(401, 403, 404)
  async findById(
    @Param('id', ParseIntPipe) id: number,
  ): Promise<ApiControllerResponse<UserResponseDto>> {
    const user = await this.userService.findById(id);
    return { message: 'User fetched successfully', data: user };
  }

  @Patch(':id')
  @UseGuards(JwtAuthGuard)
  @ApiBearerAuth()
  @ApiOperation({ summary: 'Update a user' })
  @ApiBody({ type: UpdateUserDto })
  @ApiSuccessResponseDecorator(UserResponseDto, 200, 'User updated successfully')
  @ApiErrorResponsesDecorator(401, 403, 404, 400)
  async updateUser(
    @Param('id', ParseIntPipe) id: number,
    @Body() dto: UpdateUserDto,
  ): Promise<ApiControllerResponse<UserResponseDto>> {
    const updated = await this.userService.updateUser(id, dto);
    return { message: 'User updated successfully', data: updated };
  }

  @Delete(':id')
  @HttpCode(HttpStatus.NO_CONTENT)
  @UseGuards(JwtAuthGuard, RolesGuard)
  @Roles(Role.ADMIN)
  @ApiBearerAuth()
  @ApiOperation({ summary: 'Delete a user' })
  @ApiErrorResponsesDecorator(401, 403, 404)
  async deleteUser(@Param('id', ParseIntPipe) id: number): Promise<void> {
    await this.userService.deleteUser(id);
  }

  @Get('admin-only')
  @UseGuards(JwtAuthGuard, RolesGuard)
  @Roles(Role.ADMIN)
  @ApiBearerAuth()
  @ApiOperation({ summary: 'Admin only endpoint' })
  @ApiErrorResponsesDecorator(401, 403)
  adminOnly() {
    return { message: 'You are an admin', data: true };
  }
}
