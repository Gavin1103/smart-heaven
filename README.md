# Smart Cities Group Project - 2026

## Overview

This monorepo contains the backend, frontend, embedded firmware, and documentation for the Smart City simulation project.

## Project Structure

```
city-smart-heaven/
├── apps/
│   ├── api/              # Backend – NestJS
│   ├── web/              # Frontend – Angular
│   └── embedded/
│       ├── smart_city_v2/    # Active: unified embedded firmware (PlatformIO)
│       └── legacy/           # Archived: older standalone sketches & prototypes
│
├── packages/
│   └── shared-types/     # Shared TypeScript types (api & web)
│
├── docs/                 # MkDocs documentation source
├── mdocotion/            # MkDocs theme/plugin (mdocotion)
└── docker-compose.mdocotion.yaml
```

## Apps

### Backend (`apps/api`)
- **Framework:** NestJS
- **Database:** PostgreSQL via Prisma ORM

### Frontend (`apps/web`)
- **Framework:** Angular

### Embedded (`apps/embedded`)

#### `smart_city_v2` (active)
The new unified firmware project built with PlatformIO. All embedded features are being integrated here going forward.

- `src/` – main application entry point
- `lib/` – hardware abstraction libraries (e.g. `BridgeSystem`, `I2CManager`)
- `include/` – shared headers
- `platformio.ini` – PlatformIO build config

#### `legacy` (archived)
Older standalone implementations kept for reference. Do not add new features here.

- `bridge/` – Rust-based bridge prototype
- `city-hub-prototype-c3/` – C3 hub prototype (Rust)
- `ino-files/` – Arduino `.ino` sketches (traffic light, railway barrier, bus stop)
- `Parkingsensor/`, `Parkingsensor_sign/` – parking sensor prototypes

---

## Documentation

Docs live in `docs/` and are served via MkDocs using the custom `mdocotion` theme/plugin.

Start the docs server:

```bash
docker compose -f docker-compose.mdocotion.yaml up
```

---

## Backend Development Setup

The backend and frontend run via Docker Compose.

### Environment Variables

Copy the following into a `.env` file at the project root:

```env
POSTGRES_USER=postgres
POSTGRES_PASSWORD=postgres
POSTGRES_DB=smart-heaven
DATABASE_URL=postgresql://postgres:postgres@db:5432/smart-heaven?schema=public

API_PORT=3000
WEB_PORT=4200
DB_PORT=5433
```

> Inside Docker the database host is `db`. From your local machine it is available on `localhost:${DB_PORT}`.

### Start All Services

```bash
docker compose up
```

| Service                   | URL                        |
|:--------------------------|:---------------------------|
| Frontend                  | http://localhost:4200      |
| Backend API               | http://localhost:3000      |
| Swagger API docs          | http://localhost:3000/docs |
| PostgreSQL                | localhost:5433             |

To resolve imports in your editor, also run:

```bash
npm install
```

> The application itself runs entirely through Docker — this is only for editor type resolution.

---

## Database Commands

Run all Prisma commands from the monorepo root after starting the API.

| Command                  | Description                              |
|:-------------------------|:-----------------------------------------|
| `npm run db:migrate`     | Generate and apply a migration           |
| `npm run db:push`        | Push schema changes without a migration  |
| `npm run db:reset`       | Reset database and reapply all migrations|
| `npm run db:seed`        | Seed with fake data                      |

Example with migration name:

```bash
npm run db:migrate -- --name init
```

### Database Access (TablePlus / any client)

| Field    | Value        |
|:---------|:-------------|
| Host     | localhost    |
| Port     | 5433         |
| User     | postgres     |
| Password | postgres     |
| Database | smart-heaven |

---

## Useful Docker Commands

```bash
docker ps                 # view running containers
docker compose down       # stop all services
docker compose up --build # rebuild and start
```

---

## Development Workflow

1. Start services: `docker compose up`
2. Run migrations: `npm run db:migrate` (or `db:push` for schema-only)
3. Open the app:
   - Frontend: http://localhost:4200
   - Backend API: http://localhost:3000

**Notes:**
- Hot reload is enabled for both NestJS and Angular
- All services run inside Docker containers
- The database is persisted via a Docker volume (`pgdata`)
