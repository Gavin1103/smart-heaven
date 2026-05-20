/*
  Warnings:

  - You are about to drop the column `spotCode` on the `ParkingSpot` table. All the data in the column will be lost.
  - Added the required column `updatedAt` to the `User` table without a default value. This is not possible if the table is not empty.

*/
-- DropIndex
DROP INDEX "ParkingSpot_spotCode_key";

-- DropIndex
DROP INDEX "User_email_key";

-- AlterTable
ALTER TABLE "BusSchedule" ADD COLUMN     "createdAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP;

-- AlterTable
ALTER TABLE "ParkingSpot" DROP COLUMN "spotCode";

-- AlterTable
ALTER TABLE "User" ADD COLUMN     "updatedAt" TIMESTAMP(3) NOT NULL;
