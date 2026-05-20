# Smart City Backend – Database Implementation Documentation (Prisma + TypeScript)

**Made by:** Denzel Purperhart  
**Date:** April 2026  
**Subject:** Database implementation document for the Smart City ParkingSpot backend
<!-- TOC -->

* [Smart City Backend – Database Implementation Documentation (Prisma + TypeScript)](#smart-city-backend--database-implementation-documentation-prisma--typescript)
  * [1. Introduction](#1-introduction)
  * [2. Purpose of the Implementation](#2-purpose-of-the-implementation)
  * [3. Technologies Used](#3-technologies-used)
  * [4. Database Design with Prisma](#4-database-design-with-prisma)
  * [5. Prisma Schema Structure](#5-prisma-schema-structure)
  * [6. Database Connection Configuration](#6-database-connection-configuration)
  * [7. Running Prisma Migrations](#7-running-prisma-migrations)
  * [8. Backend Integration with Prisma Client (NestJS)](#8-backend-integration-with-prisma-client-nestjs)
  * [9. Backend Module Structure – Parking Module Implementation](#9-backend-module-structure--parking-module-implementation)
  * [10. Problems During Implementation and Conclusion](#10-problems-during-implementation-and-conclusion)

# 1. Introduction

This document describes the implementation of the backend database infrastructure for the Smart City ParkingSpot system using PostgreSQL and Prisma ORM within a TypeScript-based NestJS backend environment.

The database supports storage and retrieval of infrastructure data related to:

- parking occupancy measurements
- public transport arrival schedules
- device monitoring information
- location-based infrastructure metadata

Prisma ORM is used as the database abstraction layer to ensure type-safe queries and structured schema migrations.

---

# 2. Purpose of the Implementation

The purpose of this implementation was to create a structured and maintainable relational database architecture that integrates with the Smart City backend API.

This included defining Prisma models, configuring PostgreSQL connections, generating migrations, and exposing infrastructure datasets through REST endpoints implemented inside the NestJS backend.

The resulting architecture supports scalable smart city infrastructure monitoring.

---

# 3. Technologies Used

The following technologies were used:

| Technology | Purpose |
|-----------|---------|
| PostgreSQL | relational database |
| Prisma ORM | schema modelling and queries |
| Prisma Migrate | database version control |
| TypeScript | backend development language |
| NestJS | backend API framework |
| Docker | containerized database environment |
| Angular frontend | infrastructure dashboard interface |

---

# 4. Database Design with Prisma

Each model represents a relational PostgreSQL table. Prisma automatically translates these models into SQL migrations and keeps the schema synchronized with the database.

The current schema includes:

- Location
- Device
- ParkingSpot
- ParkingLog
- BusSchedule
- BusLog

This modular structure supports extensibility for future smart city datasets.

---

# 5. Prisma Schema Structure

Example Prisma model:

``` ts
model ParkingSpot {
  id        Int      @id @default(autoincrement())
  occupied  Boolean
  createdAt DateTime @default(now())
  updatedAt DateTime @updatedAt

  locationId Int
  deviceId   Int
} 
```
After running migrations, this model becomes a PostgreSQL table automatically.

Prisma ensures schema consistency across development environments.

# 6. Database Connection Configuration

Database connectivity is configured using the .env file:

```
DATABASE_URL="postgresql://user:password@localhost:5433/smartcity"
```

This connection string is used by Prisma Client inside the NestJS backend.

Using environment variables improves portability and security.

# 7. Running Prisma Migrations

Database tables are created using Prisma migration commands:
```
npx prisma migrate dev --name init
```

This command:

- generates migration files
- applies schema changes
- updates Prisma Client automatically

After execution, PostgreSQL matches the schema definition exactly.

# 8 Backend Integration with Prisma Client (NestJS)

The backend communicates with the database using Prisma Client inside NestJS services.

Example Prisma service:
```
import { PrismaClient } from '@prisma/client';

export const prisma = new PrismaClient();
``` 
Example query:
```
const parkingSpots = await prisma.parkingSpot.findMany();
```
This replaces raw SQL queries and ensures type-safe database interaction inside the backend architecture.

# 9. Backend Module Structure – Parking Module Implementation

Within the Smart City backend architecture, the parking functionality is implemented as a dedicated NestJS module. This module is responsible for handling parking occupancy data received from sensors and exposing this information through REST API endpoints.

The module follows a layered architecture consisting of:

- controller layer
- service layer
- repository layer
- DTO definitions

This structure improves maintainability, scalability, and separation of responsibilities within the backend system.

---

# 9. Backend Module Implementation – Parking Module (NestJS + Prisma)

This chapter explains how the Parking module is implemented inside the Smart City backend architecture. The module is responsible for receiving parking occupancy updates from ESP32 devices and exposing parking availability information through REST API endpoints.

The backend follows a layered architecture consisting of a controller layer, service layer, repository layer, and DTO validation layer. This structure improves maintainability, scalability, and separation of responsibilities between components.

---

## 9.1 Module Architecture Overview

The Parking module is implemented using the NestJS modular architecture pattern:


Each layer has a specific responsibility within the system.

---

## 9.2 parking.controller.ts

The controller handles incoming HTTP requests from the dashboard frontend or embedded devices. It defines API routes and forwards requests to the service layer.

Example controller implementation:

```ts
import { Controller, Get, Patch, Param, Body } from '@nestjs/common';
import { ParkingService } from './parking.service';
import { UpdateParkingDto } from './dto/update-parking.dto';

@Controller('parking-spots')
export class ParkingController {
  constructor(private readonly parkingService: ParkingService) {}

  @Get()
  findAll() {
    return this.parkingService.getAllParkingSpots();
  }

  @Patch(':id')
  updateStatus(
    @Param('id') id: number,
    @Body() updateParkingDto: UpdateParkingDto,
  ) {
    return this.parkingService.updateParkingSpotStatus(
      id,
      updateParkingDto,
    );
  }
}

The controller ensures structured communication between frontend requests and backend logic.
```
# 9.3 parking.service.ts

The service layer contains the business logic of the Parking module. It processes incoming data and coordinates database operations through the repository layer.

```ts
import { Injectable } from '@nestjs/common';
import { ParkingRepository } from './parking.repository';
import { UpdateParkingDto } from './dto/update-parking.dto';

@Injectable()
export class ParkingService {
  constructor(
    private readonly parkingRepository: ParkingRepository,
  ) {}

  getAllParkingSpots() {
    return this.parkingRepository.findAll();
  }

  updateParkingSpotStatus(
    id: number,
    updateParkingDto: UpdateParkingDto,
  ) {
    return this.parkingRepository.updateStatus(
      id,
      updateParkingDto.occupied,
    );
  }
}
```
# 9.4 parking.repository.ts

The repository layer communicates directly with the database through Prisma Client. This abstraction improves code readability and separates database queries from business logic.

```ts
import { Injectable } from '@nestjs/common';
import { PrismaService } from '../prisma/prisma.service';

@Injectable()
export class ParkingRepository {
  constructor(private prisma: PrismaService) {}

  findAll() {
    return this.prisma.parkingSpot.findMany();
  }

  updateStatus(id: number, occupied: boolean) {
    return this.prisma.parkingSpot.update({
      where: { id },
      data: { occupied },
    });
  }
}
```
# 9.5 DTO Validation – update-parking.dto.ts

DTO files define the structure of incoming request data and ensure type safety within the backend system.

Example DTO:
```ts 
import { IsBoolean } from 'class-validator';

export class UpdateParkingDto {
  @IsBoolean()
  occupied: boolean;
}
```

# 9.6 parking.module.ts

The module file registers all Parking components inside the NestJS dependency injection system.

```ts
import { Module } from '@nestjs/common';
import { ParkingController } from './parking.controller';
import { ParkingService } from './parking.service';
import { ParkingRepository } from './parking.repository';
import { PrismaService } from '../prisma/prisma.service';

@Module({
  controllers: [ParkingController],
  providers: [
    ParkingService,
    ParkingRepository,
    PrismaService,
  ],
})
export class ParkingModule {}
```
This modular structure keeps the Parking functionality isolated from other infrastructure modules such as BusSchedule or Device monitoring.

# 10. Problems During Implementation and Conclusion

During the implementation of the Smart City ParkingSpot backend database several technical issues occurred related to Prisma configuration, PostgreSQL connectivity, and Docker container networking. Resolving these issues improved stability and ensured that the backend system functioned correctly within the NestJS TypeScript environment.

One of the first problems occurred when Prisma could not connect to the PostgreSQL database because the `DATABASE_URL` environment variable was missing or incorrectly configured. This caused runtime errors indicating that the database connection string could not be read. The issue was resolved by correctly defining the database connection inside the `.env` 


Another issue appeared when the backend attempted to connect using SSL while the locally running PostgreSQL Docker container did not support SSL connections. This resulted in the error message stating that the server does not support SSL connections. The solution was to disable SSL for the local development database configuration and connect directly through the exposed Docker port.

A third issue occurred when the backend attempted to connect to the database using the container hostname instead of `localhost`. Because Docker hostnames are only accessible inside the container network, the connection failed. Updating the connection string to use `localhost:5433` resolved the issue and allowed Prisma to connect successfully from the local development environment.

An additional problem occurred during migration execution when SQL statements were accidentally inserted into the Prisma schema file. Prisma schema files only support Prisma model syntax and cannot contain raw SQL commands such as `CREATE TABLE`. After removing the SQL statements and rewriting the schema using Prisma models, migrations executed successfully.

A final issue occurred when migrations were executed while the PostgreSQL container was not running. This caused connection errors indicating that the database server could not be reached. Restarting the Docker container resolved the problem:


After resolving these configuration issues, the backend successfully connected to PostgreSQL using Prisma Client within the NestJS TypeScript environment. The implemented schema now supports storage and retrieval of parking occupancy data and public transport arrival schedules through REST API endpoints that are used by the dashboard interface.

Because the database structure is defined using Prisma migrations and integrated into a modular NestJS backend architecture, the system remains scalable and maintainable for future extensions such as additional sensor datasets, authentication layers, and real-time infrastructure monitoring features. The implemented backend database therefore provides a stable and extensible foundation for further development of the Smart City ParkingSpot platform.
