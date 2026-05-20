# Smart City Parking API

This API manages parking spot data for the Smart City project.

## Tech stack

- NestJS
- Prisma ORM
- PostgreSQL
- Docker

## Installation

Install dependencies:

npm install

Start development server:

npm run start:dev

Server runs on:

http://localhost:3000


## Swagger documentation

Available at:

http://localhost:3000/api


## Database

Start database container:

docker compose up -d


## Endpoints

### Update parking spot

POST /parking/update

Example request:

{
  "spotCode": "A12",
  "location": "Station Noord",
  "occupied": true
}


### Get all parking spots

GET /parking


## Example response

{
  "id": 1,
  "spotCode": "A12",
  "location": "Station Noord",
  "occupied": true
}


## Run with Docker

Start API + database:

docker compose up --build

Stop containers:

docker compose down