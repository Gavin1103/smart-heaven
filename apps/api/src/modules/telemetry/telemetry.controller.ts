import {Body, Controller, HttpCode, HttpStatus, Post, UseGuards} from '@nestjs/common';
import {ApiBearerAuth, ApiBody, ApiHeader, ApiOperation, ApiTags} from '@nestjs/swagger';
import {TelemetryService} from './telemetry.service';
import {CreateTelemetryDto} from "./dto/create-telemetry.dto";
import {ApiErrorResponsesDecorator} from "../../common/decorators/api-error-response.decorator";
import {ApiSuccessResponseDecorator} from "../../common/decorators/api-success-response.decorator";
import {ApiResponseDto} from "../../common/dto/api-response.dto";
import {JwtOrApiKeyGuard} from "../auth/guards/jwt-or-api-key.guard";

@ApiTags('Telemetry')
@Controller('telemetry')
@UseGuards(JwtOrApiKeyGuard)
@ApiBearerAuth()
@ApiHeader({name: 'X-Api-Key', description: 'Device API key (alternative to Bearer JWT)', required: false})
export class TelemetryController {
    constructor(private readonly telemetryService: TelemetryService) {
    }

    @Post()
    @HttpCode(HttpStatus.NO_CONTENT)
    @ApiOperation({summary: 'Receive telemetry from embedded devices'})
    @ApiBody({type: CreateTelemetryDto})
    @ApiSuccessResponseDecorator(ApiResponseDto, 204, "Successfully created telemetry")
    @ApiErrorResponsesDecorator(401, 404, 400)
    async receiveTelemetry(@Body() dto: CreateTelemetryDto) {
        await this.telemetryService.receiveTelemetry(dto);
    }
}
