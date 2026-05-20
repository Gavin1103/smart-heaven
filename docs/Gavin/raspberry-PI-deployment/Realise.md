# Raspberry PI Deployment - Realise

* **Written by:** Gavin Tjin, software engineer
* **Date:** 20-05-2026
* **Classifications:** Public
* **Version:** 1.0
* **Target audience:** Backend and embedded engineers
* **Keywords:** Raspberry Pi, Railway, Docker, NestJS, Deployment, Smart City
* **Client:** Gerald Stap (mayor)
* **Company:** Smart Heaven

# Table of contents

1. Introduction
2. Initial Raspberry Pi deployment

    * 2.1 Introduction
    * 2.2 Raspberry Pi environment setup
    * 2.3 Docker and Node.js installation
    * 2.4 API deployment attempt
    * 2.5 Performance limitations
    * 2.6 Partial conclusion
3. Alternative cloud deployment

    * 3.1 Introduction
    * 3.2 Choosing Railway
    * 3.3 Docker configuration
    * 3.4 Deployment process
    * 3.5 Verification and testing
    * 3.6 Partial conclusion
4. Reflection on the deployment process

    * 4.1 Introduction
    * 4.2 Differences between edge and cloud deployment
    * 4.3 Lessons learned
    * 4.4 Partial conclusion
5. Conclusion
6. Recommendations
7. References

# 1. Introduction

This document describes the realisation process of deploying the Smart Heaven backend API.
The original goal of this learning objective was to deploy the NestJS API and database locally on a Raspberry Pi using
Docker.

The Smart Heaven project contains multiple Smart City devices such as railway barriers, traffic lights, and parking
sensors.
These devices continuously send telemetry data to the backend API, meaning the API must remain stable and accessible.

Based on the previous analysis and advice documents, deploying the API locally on a Raspberry Pi was considered a
suitable
edge computing solution. The intention was to reduce network dependency and process Smart City data locally.

During implementation, several hardware limitations were encountered. The Raspberry Pi 3 B used for this project did not
have enough processing power and memory to reliably run both the NestJS API and the PostgreSQL database simultaneously.
Because of these limitations, the deployment strategy was changed from edge deployment to cloud deployment.

As an alternative solution, the API was successfully deployed on Railway using Docker.

The following main question is central within this realisation document:

**How was the Smart Heaven backend API deployed, and what challenges and solutions were encountered during the
deployment process?**

To answer this question, the following subquestions are defined:

* How was the Raspberry Pi environment prepared for deployment?
* Which limitations prevented the Raspberry Pi deployment from succeeding?
* Why was Railway chosen as an alternative deployment platform?
* How was the API deployed to Railway using Docker?
* What lessons were learned during the deployment process?

# 2. Initial Raspberry Pi deployment

## 2.1 Introduction

This chapter describes the original attempt to deploy the Smart Heaven backend API on a Raspberry Pi.

## 2.2 Raspberry Pi environment setup

The deployment process started with preparing the Raspberry Pi environment.
A Raspberry Pi 3 Model B was used during this project.

The Raspberry Pi OS was installed on a microSD card using the Raspberry Pi Imager.
SSH was enabled so the device could be accessed remotely through the terminal.

After the operating system was installed, the Raspberry Pi was updated using the following commands:

```bash
sudo apt update
sudo apt upgrade
```

This ensured that the latest system packages and security updates were installed before starting the deployment process.

## 2.3 Docker and Node.js installation

After configuring the Raspberry Pi environment, Docker and Node.js were installed.
Docker was required to containerise the NestJS API and PostgreSQL database.

Docker was installed using the official Docker installation script.
After installation, Docker commands were tested successfully.

Node.js and npm were also installed to allow local testing of the NestJS application outside of Docker.

The API project was cloned from the GitLab repository onto the Raspberry Pi.
Dependencies were installed using npm.

## 2.4 API deployment attempt

The original deployment goal was to run both the NestJS API and the PostgreSQL database inside Docker containers.

A Dockerfile was created for the NestJS API.
The API container exposed port 3000 and used environment variables for database configuration.

A Docker Compose configuration was prepared to run:

* The NestJS API container
* The PostgreSQL database container

The API was able to start successfully in some situations.
However, performance problems quickly became visible when both services were running simultaneously.

## 2.5 Performance limitations

The Raspberry Pi 3 Model B introduced several hardware limitations during deployment.

The most significant issue was limited RAM and CPU performance.
Running both PostgreSQL and the NestJS API at the same time caused the Raspberry Pi to become extremely slow.
In some situations, the API startup process took a very long time or became unresponsive.

The limited hardware resources also affected Docker performance.
Container builds were slow, package installation required significant time, and system responsiveness decreased heavily
during runtime.

Although the API could technically run on the Raspberry Pi, the device was not powerful enough to provide a stable and
reliable deployment environment for both services together.

Because the Smart Heaven project requires stable backend communication for Smart City devices, this deployment approach
was considered unreliable.

## 2.6 Partial conclusion

The Raspberry Pi environment was successfully prepared by installing Raspberry Pi OS, Docker, Node.js, and the Smart
Heaven API project.
The deployment process demonstrated that Docker containers and NestJS could technically run on the Raspberry Pi.

However, the Raspberry Pi 3 Model B did not provide enough hardware resources to reliably run both the NestJS API and
PostgreSQL database simultaneously.
The limited CPU performance and memory caused major performance and stability issues.

As a result, the Raspberry Pi deployment was not considered suitable for the final implementation of the Smart Heaven
backend.

# 3. Alternative cloud deployment

## 3.1 Introduction

Because the Raspberry Pi deployment was not stable enough, an alternative deployment solution was required.
This chapter explains how the API was deployed to Railway using Docker.

## 3.2 Choosing Railway

Railway was selected as the deployment platform because it provides a simple cloud hosting environment for Docker-based
applications.

Another important reason for choosing Railway was that it already supports PostgreSQL hosting.
This made it possible to host both the backend API and database in a stable cloud environment without requiring
additional server management.

Railway also supports automatic deployments from Git repositories, environment variable management, and Dockerfile-based
deployments.

These features made Railway a practical alternative to the Raspberry Pi deployment.

## 3.3 Docker configuration

To prepare the API for Railway deployment, the Docker configuration was updated.

The Dockerfile was configured to:

* Use a Node.js base image
* Install dependencies
* Build the NestJS application
* Start the production server

Environment variables such as the database connection string and JWT secret were configured inside Railway.

The PostgreSQL database was created using Railway's managed database service.

## 3.4 Deployment process

The deployment process started by connecting the Git repository to Railway.

Railway automatically detected the Dockerfile and started building the application.
During the build process, the NestJS application was compiled and packaged into a Docker container.

After the build completed successfully, Railway deployed the container and exposed the API through a public URL.

The database connection was configured through environment variables.
After deployment, Prisma migrations were executed to create the database structure.

## 3.5 Verification and testing

After deployment, the API endpoints were tested to verify that the deployment worked correctly.

The following checks were performed:

* API startup verification
* Database connection testing
* Endpoint testing through HTTP requests
* Environment variable validation
* Docker container runtime verification

The API successfully connected to the PostgreSQL database and processed requests correctly.

Compared to the Raspberry Pi deployment, the Railway deployment provided significantly better stability and performance.

## 3.6 Partial conclusion

Railway provided a stable and reliable alternative deployment platform for the Smart Heaven backend API.
By using Docker and Railway's managed infrastructure, the NestJS API and PostgreSQL database could be deployed
successfully without the hardware limitations encountered on the Raspberry Pi.

The deployment process demonstrated that cloud infrastructure is currently more suitable for the Smart Heaven backend
than the available Raspberry Pi hardware.

# 4. Reflection on the deployment process

## 4.1 Introduction

This chapter reflects on the deployment process and the lessons learned during implementation.

## 4.2 Differences between edge and cloud deployment

The original goal of this project was to explore edge deployment using a Raspberry Pi.
This approach would allow Smart City data to be processed locally with reduced network dependency.

However, the project showed that hardware limitations play a major role when deploying backend services on edge devices.

Cloud deployment through Railway provided:

* Better performance
* More stable infrastructure
* Easier scaling
* Simpler database management
* Faster deployment workflows

At the same time, cloud deployment introduces dependency on internet connectivity and external infrastructure.

## 4.3 Lessons learned

Several important lessons were learned during this deployment process.

First, theoretical feasibility does not always guarantee practical feasibility.
Although the analysis showed that NestJS and Docker can technically run on a Raspberry Pi, the actual hardware
performance of the Raspberry Pi 3 Model B was insufficient for this project.

Second, Docker-based deployments provide flexibility because the same Docker configuration could later be reused on
Railway.
This made switching from edge deployment to cloud deployment much easier.

Finally, this project showed the importance of evaluating hardware requirements early in the development process.
Choosing hardware with sufficient CPU power and memory is essential for backend deployments.

## 4.4 Partial conclusion

The deployment process demonstrated the practical differences between edge deployment and cloud deployment.
While Raspberry Pi deployment was technically possible, the available hardware was not powerful enough for stable
production usage.

The final Railway deployment proved to be a more suitable solution for the Smart Heaven backend API.
At the same time, the project provided valuable experience with Docker, Raspberry Pi environments, and cloud deployment
workflows.

# 5. Conclusion

This realisation process aimed to deploy the Smart Heaven backend API using Docker on a Raspberry Pi.

The Raspberry Pi environment was successfully configured with Raspberry Pi OS, Docker, Node.js, and the Smart Heaven
project files.
The NestJS API and PostgreSQL database could technically run on the device.

However, the Raspberry Pi 3 Model B did not provide enough CPU power and memory to reliably run both services
simultaneously.
This resulted in major performance and stability issues during deployment.

Because the Raspberry Pi deployment was not suitable for stable backend operation, the deployment strategy was changed
to a cloud-based solution.

Railway was selected as the alternative deployment platform.
Using Docker and Railway's managed PostgreSQL service, the API was successfully deployed and tested.

**Based on these findings, the main question can be answered as follows:**
The Smart Heaven backend API was initially deployed on a Raspberry Pi using Docker, but hardware limitations prevented
stable operation.
As a solution, the API was successfully deployed to Railway using Docker and managed cloud infrastructure.

# 6. Recommendations

Based on the deployment experience, the following recommendations are made:

* Use a more powerful Raspberry Pi model such as the Raspberry Pi 5 when running both a backend API and database
  locally.

* Prefer SSD storage instead of microSD cards for Docker workloads because SSD storage provides better performance and
  durability.

* Use lightweight Docker images to reduce memory and CPU usage on limited hardware.

* Consider separating the database and API across different devices when using edge hardware.

* Use cloud infrastructure such as Railway when stability, scalability, and reliability are more important than local
  edge processing.

# 7. References

* Docker Inc. (2025). Docker Documentation. Retrieved May 20, 2026,
  from [https://docs.docker.com/](https://docs.docker.com/)

* Railway. (2026). Railway Documentation. Retrieved May 20, 2026,
  from [https://docs.railway.com/](https://docs.railway.com/)

* NestJS. (n.d.). Documentation | NestJS - A progressive Node.js framework. Retrieved May 20, 2026,
  from [https://docs.nestjs.com/](https://docs.nestjs.com/)

* Raspberry Pi Foundation. (2026). Raspberry Pi Documentation. Retrieved May 20, 2026,
  from [https://www.raspberrypi.com/documentation/](https://www.raspberrypi.com/documentation/)
