/*
  Warnings:

  - A unique constraint covering the columns `[spotCode]` on the table `ParkingSpot` will be added. If there are existing duplicate values, this will fail.
  - Added the required column `spotCode` to the `ParkingSpot` table without a default value. This is not possible if the table is not empty.

*/
-- AlterTable
ALTER TABLE "ParkingSpot" ADD COLUMN     "createdAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
ADD COLUMN     "spotCode" TEXT NOT NULL;

-- CreateIndex
CREATE UNIQUE INDEX "ParkingSpot_spotCode_key" ON "ParkingSpot"("spotCode");
