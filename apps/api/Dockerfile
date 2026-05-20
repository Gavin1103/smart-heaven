FROM node:20-alpine

WORKDIR /app

COPY package*.json ./

RUN npm install

COPY . .

ARG DATABASE_URL="postgresql://postgres:postgres@localhost:5435/smart-heaven"
ENV DATABASE_URL=$DATABASE_URL

RUN npx prisma generate
RUN npm run build

CMD ["sh", "-c", "npx prisma migrate deploy && npm run start:prod"]