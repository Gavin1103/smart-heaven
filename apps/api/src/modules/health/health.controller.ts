import { Controller, Get } from '@nestjs/common';
import { ApiOperation, ApiResponse, ApiTags } from '@nestjs/swagger';

@ApiTags('HealthController')
@Controller('health')
export class HealthController {
  @Get()
  @ApiOperation({ summary: 'Check if the API is running' })
  @ApiResponse({
    status: 200,
    description: 'API is running successfully',
    schema: {
      example: {
        ok: true,
        message: 'API works',
      },
    },
  })
  ping() {
    return { ok: true, message: 'API works' };
  }
}
