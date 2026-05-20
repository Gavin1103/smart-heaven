## API Architecture

### Overview

This API / backend is built with NestJS and follows a layered architecture to keep responsibilities separated and the codebase
maintainable.

The main layers are:

````
Controller → Service → Repository → Database
````

````
HTTP Request
↓
Controller
↓
Service (business logic)
↓
Repository (database access)
↓
Prisma ORM
↓
PostgreSQL
````

## Project Structure

````
src/
│
├── modules/
│   └── user/
│       ├── dto/
│       │   ├── create-user-request.dto.ts
│       │   └── user-response.dto.ts
│       │
│       ├── user.controller.ts
│       ├── user.service.ts
│       ├── user.repository.ts
│       └── user.module.ts
│
├── main.ts
└── app.module.ts
````

Each domain is organised into its own module.

Example:

````
User
├── Controller
├── Service
├── Module
├── Repository
└── DTOs
````

## Layer Responsibilities

### Controller

Controllers handle incoming HTTP requests and return responses.

Responsibilities:

- Define API routes
- Validate request input via DTOs
- Call the appropriate service

Example:

````typescript
@Post('/create-user')
@ApiOperation({summary: 'Create a new user'})
@ApiBody({type: CreateUserRequestDto})
@ApiCreatedResponse({
    description: 'User successfully created',
    type: UserResponseDto,
})
@ApiBadRequestResponse({
    description: 'Invalid input data',
})
createUser(@Body()
createUserDto: CreateUserRequestDto
):
Promise < UserResponseDto > {
    return this.userService.createUser(createUserDto);
}
````

Controllers should not contain business logic.

### Service

Services contain the business logic of the application.

Responsibilities:

- Application logic
- Validation rules
- Security checks
- Data transformation

Example:

````
Check if email already exists
Hash password
Call repository to store user
Return response DTO
````

### Repository

Repositories handle database access.

Responsibilities:

- Query the database
- Use Prisma ORM
- Return database models

Example:

````
create(data: CreateUserRequestDto) {
return this.prisma.user.create({
data
})
}
````

Repositories should not contain business logic.

### DTO (Data Transfer Object)

DTOs define the shape of incoming and outgoing data.

Two types are used:

### Request DTO

Used for validating incoming requests.

Example:

````
CreateUserRequestDto
````

### Response DTO

Used for returning safe data to the client.

Example:

````
UserResponseDto
````

Sensitive fields such as passwords are never returned.

## Database Layer

The backend uses Prisma ORM to communicate with PostgreSQL.

Prisma schema files are organised into multiple files:

````
prisma/
├── schema.prisma
└── models/
    └── user.prisma
````

This keeps the schema modular as the database grows.

Example model:

````prisma
model User {
  id        Int      @id @default(autoincrement())
  email     String   @unique
  password  String   
  firstName String   
  lastName  String   
  role      Role     
  createdAt DateTime @default(now())
}

````

## Dependency Injection

NestJS uses dependency injection to manage services.

Example:

````
UserController
↓
UserService
↓
UserRepository
````

This makes components:

- easier to test
- loosely coupled
- reusable

## API Documentation

The API is documented using Swagger.

Swagger is available at:

````
/docs
````

Example endpoint documentation:

````
POST /user/create-user
Create a new user
````

DTOs automatically generate request and response schemas in Swagger.

### Security Considerations

The backend follows several security practices:

- Passwords are hashed before storage
- DTO validation prevents invalid input
- Sensitive fields are excluded from responses
- Unique constraints prevent duplicate users

### Future Improvements

Planned improvements include:

- Authentication (JWT)
- Role-based authorization
- Refresh tokens
- Global error handling
- API response handling
- Logging
- Password hashing

## Technology Stack

| Technology | Purpose                 |
|------------|-------------------------|
| NestJS     | Backend framework       |
| Prisma     | ORM                     |
| PostgreSQL | Database                |
| Docker     | Development environment |
| Swagger    | API documentation       |
