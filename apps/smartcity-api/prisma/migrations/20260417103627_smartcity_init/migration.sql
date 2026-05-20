-- CreateTable
CREATE TABLE "ParkingSpot" (
    "id" SERIAL NOT NULL,
    "location" TEXT NOT NULL,
    "occupied" BOOLEAN NOT NULL,
    "updatedAt" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "ParkingSpot_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "BusSchedule" (
    "id" SERIAL NOT NULL,
    "line" TEXT NOT NULL,
    "destination" TEXT NOT NULL,
    "arrivalTime" TIMESTAMP(3) NOT NULL,
    "updatedAt" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "BusSchedule_pkey" PRIMARY KEY ("id")
);
