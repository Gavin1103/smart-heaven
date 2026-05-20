# Raspberry PI deployment - Analyse

- **Written by:** Gavin Tjin, software engineer
- **Date:** 16-04-2026
- **Classifications:** Public
- **Version:** 3.0
- **Client:** Gerald Stap (mayor)
- **Company:** Smart Heaven
- **Keywords:** Raspberry Pi, Docker, NestJS
- **Client:** Gerald Stap (mayor)
- **Company:** Smart Heaven

# Table of contents

1. Introduction
2. Docker on Raspberry Pi  
   2.1 Introduction  
   2.2 Docker as a containerisation platform  
   2.3 Requirements  
   2.4 Installation methods  
   2.5 Limitations  
   2.6 Partial conclusion
3. NestJS on Raspberry Pi  
   3.1 Introduction  
   3.2 NestJS as a backend framework  
   3.3 Requirements  
   3.4 Limitations  
   3.5 Partial conclusion
4. Deploying NestJS with Docker on Raspberry Pi  
   4.1 Introduction  
   4.2 Node.js Docker images  
   4.3 Deployment steps  
   4.4 Partial conclusion
5. Conclusion
6. Recommendations
7. References

# 1. Introduction

In the Smart City project **Smart Heaven**, multiple hardware components such as traffic lights, parking sensors, and
railway barriers continuously generate data that must be processed and stored.

As a backend developer, managing this data flow is essential. Traditional cloud deployments introduce challenges such as
latency, network dependency, and reduced reliability when connectivity is unstable.

To improve reliability and reduce latency, edge computing using devices like a Raspberry Pi is considered. This enables
local processing, ensuring the system continues functioning even without a stable internet connection.

This research is specifically aimed at backend developers who are responsible for designing, deploying, and maintaining
APIs in a Smart City environment. It provides insights into how backend services can be deployed on edge hardware
instead of relying solely on cloud infrastructure.

This research focuses on the following main question:
**How can a NestJS application be deployed on a Raspberry Pi with Docker, and what are the requirements and limitations
of running Docker and NestJS on a Raspberry Pi?**

To answer this, the following subquestions are defined:

- What are the requirements and limitations of running Docker on a Raspberry Pi?
- What are the requirements and limitations of running a NestJS application on a Raspberry Pi?
- How can Docker be used to deploy a NestJS application on a Raspberry Pi?

This research first explores Docker on a Raspberry Pi, then the requirements of running a NestJS application, and
finally the deployment steps using Docker.

# 2. Docker on Raspberry Pi

## 2.1 Introduction

This chapter explores the requirements and limitations of running Docker on a Raspberry Pi to determine whether it is
suitable for deploying backend applications.

## 2.2 Docker as a containerisation platform

According to Sun & Sun (2024), Docker is a platform where you can package and run applications in isolated environments
called containers. These containers include everything an application needs to run, such as code, libraries, and
dependencies, while ensuring consistency across different environments.

## 2.3 Requirements

To run Docker Engine on a Raspberry Pi, there are a few things that are necessary.
If you have a 32-bit Pi OS, you need a 32-bit Raspberry Pi OS Bookworm 12 (stable) or Bullseye 11 (oldstable).
Do you have a 64-bit (ARM64)? Then you need Debian Trixie (stable), Debian Bookworm (oldstable) or
Debian Bullseye 11 (oldoldstable). One of the very important things to do before installation is to make sure you have
deleted old packages
(related to docker) to avoid package conflicts.
(Docker Inc, 2025)

It is also recommended to have at least 2 GB of RAM (4 to 8 GB of RAM is the sweet spot).
And to use a microSD card (of quality) or a USB-SSD. SSD is recommended for Docker workloads because they are faster.
(Dhandala, 2026)

## 2.4 Installation methods

This section is based on (Docker Inc, 2025).

Docker Engine has multiple ways to be installed. The easiest and quickest way to install it is
with the Docker Desktop for Linux. But this method is not recommended for Raspberry Pi because
one of the requirements is, for example, 4 GB of RAM

The second method is to install it using the apt repository; this is the standard method. For this method you need
to set up the Docker apt repository before you can install and update the Docker Engine.

If you can't install it with the apt repository, you can do it manually (from the package).
You have to download the **deb** file for your release and install it manually. You have to download a new file each
time you want to upgrade the Docker Engine

The last method is to install it using the convenience script. Docker provides this convenience script
to install docker into development environments non-interactively. This script is not recommended for production
environments but mostly for testing and development

## 2.5 Limitations

This section is based on (Dhandala, 2026).

There are some limitations to keep in mind when using the Docker Engine on a Raspberry Pi.
Not every Docker image supports ARM64. When pulling an image, check if the image supports
multi-architecture. Most official images (nginx, postgres, node, etc.) support
**linux/arm64** natively. So if you are using an image that only supports **linux/arm64**,
it will not work with your Pi.

Another thing is to keep in mind that a Pi has limited RAM and CPU usage. But fortunately, you can configure
Docker to be conservative with your memory usage. Just know that running multiple or
resource-intensive containers can overwhelm the limited CPU and memory of the Pi.

Also, be aware that microSD cards are slow and wear out with heavy writing.
Docker image layers and container writes can quickly wear a SD card out.

## 2.6 Partial conclusion

Docker can be installed and used on a Raspberry Pi when the correct operating system (32-bit or 64-bit) and
ARM-compatible images are used. The analysis shows that specific requirements such as sufficient RAM (preferably 2–4
GB), proper storage (preferably SSD), and correct installation methods (e.g. apt repository) are necessary for stable
operation.

However, there are important limitations. Not all Docker images support ARM architectures, and the limited CPU and
memory of the Raspberry Pi restrict the number and size of containers that can be run. In addition, storage limitations,
especially when using microSD cards, can affect performance and durability.

Therefore, Docker is suitable for deploying lightweight backend applications on a Raspberry Pi, as long as these
requirements and limitations are taken into account.

# 3. NestJS on Raspberry Pi

## 3.1 Introduction

This chapter examines what is required to run a NestJS application on a Raspberry Pi and whether it introduces any
limitations.

## 3.2 NestJS as a backend framework

NestJS is a framework based on Node.js to build web applications. It uses TypeScript
as the standard language. It supports structure, clean code, modularity, and the
MVC pattern.
(NestJS: The Perfect JavaScript Backend Framework For Structure, Clean Code, And Modularity, n.d)

## 3.3 Requirements

According to the NestJS documentation (n.d), NestJS requires Node.js (version ⇒ 20). And for
running
node on a Pi, you need at least 2–4 GB of RAM and a microSD card with 16GB+.

For installing packages and running scripts, you need a package manager like:
npm, pnpm, and yarn. This research is not focused on package managers, but if you
have to choose, choose npm because that is the standard package manager for Node.js.
(An Introduction to the Npm Package Manager | Node.js Learn, n.d)

NestJs is also written in TypeScript, so it needs a TypeScript compiler (tsc) to
turn the code into JavaScript. But most of the time you won't be using this directly
if you work with a framework like NestJS, because you can just run the command: npm run
build or npm run start (for dev) and it will compile it for you (using tsc).
(Understanding The TypeScript Compiler And Its Options, n.d)

## 3.4 Limitations

Because Node.js runs fine on a Raspberry Pi, the main limitations are
hardware-related. If you have, for example, many
requests to process, it will cost you CPU performance. The same for RAM usage when
your application is getting bigger.

## 3.5 Partial conclusion

A NestJS application can run on a Raspberry Pi when the required software environment is available, including Node.js (
version ≥ 20), a package manager, and sufficient system resources. The analysis shows that the technical requirements
for running NestJS are relatively straightforward and compatible with the Raspberry Pi platform.

The main limitations are not related to NestJS itself, but to the underlying hardware. Limited CPU power and memory can
impact performance, especially under higher load or when handling multiple requests.

As a result, NestJS is suitable for running backend applications on a Raspberry Pi in small-scale or edge computing
scenarios, where resource usage remains limited.

# 4. Deploying NestJS with Docker on Raspberry Pi

## 4.1 Introduction

This chapter explains how Docker can be used to deploy a NestJS application on a Raspberry Pi.

## 4.2 Node.js Docker images

This section is based on (Choosing the Best Node.js Docker Image, 2025).

With Docker, we containerise our application, and to containerise it, you need an image.
NestJS runs on Node.js, so we need a Node image that is compatible with ARM 64 or 32.

**Node:latest:**
This is the default Node official image. It is Debian and includes everything;
the full operating system, package managers, build tools, and many more things that
you probably won't be using in production.

Pros:

- Complete environment with all the tools
- Big community support and documentation
- Debian-based, Debian supports ARM 64/32

Cons:

- It is a massive image size
- Slow to pull and deploy

**Node:slim**
This is a small version of the official image (node:latest). It removes all the
unnecessary packages while keeping the debian base.

Pros:

- About 50% size compared to the full image
- It is still Debian-based
- Debian-based, Debian supports ARM 64/32

Cons:

- Still has a lot of packages you won't be needing
- Manual patching required
- Has a lot of vulnerabilities

**Node:alpine**
This one is based on Alpine Linux. It is smaller because it used the "musl libc"
library instead of the "glibc".

Pros:

- Image size is very small (often < 100MB)
- Fewer packages
- Fast to pull and deploy
- Alpine Linux supports ARM 64/32

Cons:

- Compatibility issues with modules expecting glibc
- Requires manual security updates

## 4.3 Deployment steps

This section is based on (GeeksforGeeks, 2026; Docker Inc., 2024).

Now that we know which Docker images can be used on our Pi, we need to know how to
deploy it. And for that we need a Dockerfile.

A Dockerfile is a simple text file that contains a script of instructions for building
a Docker Image. You can see it as a blueprint for the image. The Docker engine
reads the file, executes the commands, and assembles a runnable image. This
automated process is the foundation of containerisation, ensuring that your
application runs the same way on other devices.

When you create your Dockerfile, you need to build. Here is how a Docker container is built
using a (basic) Dockerfile.

- **Step 1: FROM:** This is the base image that the container will be based on. For this case,
  we are using a Node image.
- **Step 2: WORKDIR:** This instruction specifies the "directory" or path in the image where files will be copied and
  commands
  will be executed
- **Step 3: COPY:** This command copies the contents of the local directory to the container.
- **Step 4: RUN:** This instruction tells the builder to run the specified command (e.g. install packages)
- **Step 5: CMD:** This command sets the default command a container will be using when running the image.

Now that we know how to create an image (Dockerfile), we need to build.
The most basic **docker build** command looks like this:``docker build -t <image-name> .``

When you run a build, the builder pulls the base image (FROM) and then runs the rest of the instructions in the
Dockerfile. Once the image is built, you can push it to a registry with the **docker push** command:
``docker push <image-name>``

After the image is pushed and available in a registry. You can run the image with the **docker run** command:
``docker run -p HOST_PORT:CONTAINER_PORT <image-name>``

- HOST_PORT: This is the port number on your host machine that the container will be listening to.
- CONTAINER_PORT: This is the port number that the container will be listening to

If you want to run the container on port 3000, for example:
``docker run -p 3000:3000 <image-name>``

Now the container (instance of the image) is running, and you can access it on port <raspberry-pi-ip>:3000.

## 4.4 Partial conclusion

Deploying a NestJS application on a Raspberry Pi using Docker is technically feasible and provides a consistent and
reproducible deployment method. The analysis shows that this process requires the use of ARM-compatible Node.js images,
a properly configured Dockerfile, and correct port and network configuration.

At the same time, limitations such as image compatibility (e.g. Alpine vs Debian differences), container size, and
system resource constraints must be considered when designing the deployment.

Overall, Docker enables a structured way to deploy NestJS applications on a Raspberry Pi, but it is most effective when
using lightweight configurations and when the hardware limitations of the device are taken into account.

# 5. Conclusion

This research aimed to answer how a NestJS application can be deployed on a Raspberry Pi using Docker, including the
requirements and limitations of this approach.

From the first subquestion, it can be concluded that Docker can run on a Raspberry Pi when the correct operating system,
sufficient hardware resources, and ARM-compatible images are used. However, limitations such as restricted CPU
performance, limited memory, and storage constraints must be taken into account.

The second subquestion showed that NestJS applications can run on a Raspberry Pi without major software limitations, as
long as the required environment (Node.js and dependencies) is properly configured. The main constraints are
hardware-related and affect performance under higher workloads.

The third subquestion demonstrated that Docker can be used to deploy a NestJS application by containerising the
application using a Dockerfile and running it as a container on the Raspberry Pi.

**Based on these findings, the main research question can be answered as follows:**
A NestJS application can be deployed on a Raspberry Pi using Docker by creating a container with ARM-compatible
lightweight images
and running it in a properly configured environment. However, this approach is mainly suitable for lightweight or edge
computing scenarios due to hardware limitations such as limited CPU, memory, and storage performance.

## 6. Recommendations

Based on the identified hardware limitations and deployment requirements, the following recommendations are made:

- Use lightweight Docker images such as `node:alpine`, because the Raspberry Pi has limited CPU and memory resources,
  and smaller images reduce both memory usage and deployment time.

- Prefer SSD storage over microSD cards, because Docker workloads involve frequent read and write operations, which can
  negatively impact performance and reduce the lifespan of SD cards.

- Limit the number of running containers, because the Raspberry Pi has limited processing power and memory, which can
  lead to performance issues when running multiple services.

- Use the Raspberry Pi mainly for edge computing tasks, because it is best suited for lightweight, local processing
  rather than handling high workloads.

- Consider a hybrid architecture (edge + cloud), because the Raspberry Pi alone is not suitable for scalable systems,
  and combining it with cloud infrastructure improves scalability and reliability.

# 7. References:

- GeeksforGeeks. (2026, April 9). Dockerfile. GeeksforGeeks. Retrieved April 16, 2026,
  from https://www.geeksforgeeks.org/cloud-computing/what-is-dockerfile/

- Docker Inc. (2024, October 22). Publishing and exposing ports. Docker Documentation. Retrieved April 16, 2026,
  from https://docs.docker.com/get-started/docker-concepts/running-containers/publishing-ports/

- Choosing the best Node.js Docker image. (2025, November 14). Retrieved April 16, 2026,
  from https://www.chainguard.dev/supply-chain-security-101/choosing-the-best-node-js-docker-image

- NestJS: The perfect JavaScript backend framework for structure, clean code, and modularity. (n.d.). Medium. Retrieved
  April 16, 2026,
  from https://medium.com/@ajonesb/nestjs-the-perfect-javascript-backend-framework-for-structure-clean-code-and-modularity-ae1b3a6e1418

- Understanding the TypeScript compiler and its options. (n.d.). Medium. Retrieved April 16, 2026,
  from https://medium.com/@leroyleowdev/understanding-the-typescript-compiler-and-its-options-63bbc3c494ab

- NestJS. (n.d.). Documentation | NestJS - A progressive Node.js framework. Retrieved April 16, 2026,
  from https://docs.nestjs.com/first-steps

- Node.js. (n.d.). An introduction to the npm package manager. Retrieved April 16, 2026,
  from https://nodejs.org/learn/getting-started/an-introduction-to-the-npm-package-manager

- Dhandala, N. (2026, February 8). How to install Docker on Raspberry Pi OS (64-bit). OneUptime. Retrieved April 16,
  2026,
  from https://oneuptime.com/blog/post/2026-02-08-how-to-install-docker-on-raspberry-pi-os-64-bit/view

- Docker Inc. (2025, October 7). Raspberry Pi OS (32-bit / armhf). Docker Documentation. Retrieved April 16, 2026,
  from https://docs.docker.com/engine/install/raspberry-pi-os/

- Sun, F., & Sun, F. (2024, September 29). Raspberry Pi Docker: From installation to advanced usage and troubleshooting.
  SunFounder. Retrieved April 16, 2026,
  from https://www.sunfounder.com/blogs/news/raspberry-pi-docker-from-installation-to-advanced-usage-and-troubleshooting?srsltid=AfmBOorW-9klXZAPYio47FJS4Vc_K6MozirF-qonCspz_Mm1KI9b3UVZ


