#  Smart City Backend – Setup Guide

Deze handleiding beschrijft hoe je de **Smart City backend** lokaal opzet met **Docker, PostgreSQL, Prisma en NestJS**.
Hiermee werkt iedereen in het team met dezelfde database en API.

---

#  Vereisten

Installeer eerst:

* Docker Desktop
* Node.js (v18 of hoger)
* npm
* Git

Controleer installatie:

```bash
docker --version
node -v
npm -v
git --version
```

---

#  Project openen

Ga naar backend map:

```bash
cd apps/smartcity-api
```

---

#  Environment variables instellen

Maak bestand:

```
apps/smartcity-api/.env
```

Plaats:

```env
DATABASE_URL="postgresql://user:password@db:5432/smartcity?schema=public"
```

Deze URL wordt gebruikt door Prisma om met Docker PostgreSQL te verbinden.

---

#  Docker containers starten

Start database + backend:

```bash
docker compose down -v
docker compose up --build
```

Wat gebeurt hier:

* oude containers verwijderen
* database resetten
* Prisma opnieuw genereren
* NestJS backend starten
* PostgreSQL container starten

---

#  Controleren of server werkt

Zoek in terminal output:

```
Nest application successfully started
```

Open daarna in browser:

```
http://localhost:3000
```

Of:

```
http://localhost:3000/parking
```

---

#  Database structuur (Prisma)

Database schema staat in:

```
apps/smartcity-api/prisma/schema.prisma
```

Voorbeeld ParkingSpot tabel:

```prisma
model ParkingSpot {
  id        Int      @id @default(autoincrement())
  spotCode  String   @unique
  location  String
  occupied  Boolean
  createdAt DateTime @default(now())
  updatedAt DateTime @updatedAt
}
```

---

#  Prisma client genereren

Na schema wijzigingen:

```bash
npx prisma generate
```

Database migratie uitvoeren:

```bash
npx prisma migrate dev --name init
```

---

#  Parking endpoint testen

Nieuwe parkeerplaats toevoegen:

```bash
curl -X POST http://localhost:3000/parking/update \
-H "Content-Type: application/json" \
-d "{\"spotCode\":\"A12\",\"location\":\"Station Noord\",\"occupied\":true}"
```

Alle parkeerplaatsen ophalen:

```
http://localhost:3000/parking
```

---

#  ESP32 data naar backend sturen

ESP32 kan JSON sturen naar:

```
POST /parking/update
```

Voorbeeld payload:

```json
{
  "spotCode": "B04",
  "location": "Campus Zuid",
  "occupied": false
}
```

---

#  Database bekijken via Prisma Studio

Start database GUI:

```bash
npx prisma studio
```

Open:

```
http://localhost:5555
```

Hier kun je tabellen bekijken en aanpassen.

---

#  PostgreSQL container info

Database draait in container:

```
smartcity-db
```

Poort:

```
localhost:5433
```

Login gegevens:

```
user: user
password: password
database: smartcity
```

---

#  API structuur

Belangrijke bestanden:

```
src/
 ├── prisma/
 │    └── prisma.service.ts
 │
 ├── modules/
 │    └── parking/
 │         ├── parking.controller.ts
 │         ├── parking.service.ts
 │         ├── parking.repository.ts
 │         └── parking.module.ts
```

---

#  API endpoints overzicht

Parking status opslaan:

```
POST /parking/update
```

Alle parking data ophalen:

```
GET /parking
```

Health check:

```
GET /
```

---

#  Workflow voor teamleden

Nieuwe developer setup:

```bash

cd apps/smartcity-api
docker compose up --build
```

Daarna openen:

```
http://localhost:3000/parking
```

De backend en database draaien nu lokaal correct.
