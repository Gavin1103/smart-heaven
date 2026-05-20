# Table of Contents

**Made by:** Denzel Purperhart
**Date:** April 2, 2026
**Subject:** Advice document for the backend for smart heaven.
1. [Introduction: Advisory Report for the Backend Database Selection](#1-introduction-advisoryreport-for-the-backend-database-selection)
2. [Project Overview](#2-project-overview)
3. [System Requirements Summary](#3-system-requirements-summary)
4. [Considered Database Options](#4-considered-database-options)
   - [PostgreSQL](#postgresql)
   - [MySQL and MariaDB](#mysql-and-mariadb)
   - [Microsoft SQL Server](#microsoft-sql-server)
   - [SQLite](#sqlite)
5. [Recommendation](#5-recommendation)
6. [Societal Impact](#7-societal-impact)
7. [Conclusion](#8-conclusion)
8. [sources]((#9-Sources))



# 1. introduction advisoryreport for the Backend Database Selection

This advisory report provides a recommendation for selecting a suitable database system for the Smart City Tile project. The system uses ESP32 devices to collect infrastructure data such as traffic light status and bridge activity. The database must store this data reliably and support communication with a monitoring dashboard.

The goal of this report is to recommend the most appropriate relational database solution for the backend developer responsible for implementing the data layer of the system.

---

# 2. Project Overview

During earlier research, multiple relational database management systems (RDBMS) were analysed to determine which solution best fits the Smart City Tile project. The evaluation focused on performance, scalability, reliability, maintainability, and compatibility with embedded IoT devices such as ESP32 modules.

The database selection is based on both functional and non-functional requirements, including:

- the type of infrastructure data that needs to be stored
- the frequency at which devices send updates
- expected concurrent device connections
- integration with a monitoring dashboard
- security requirements defined by stakeholders such as municipal operators

These criteria were used to determine which database solution is most suitable for the backend architecture.

---

# 3. System Requirements Summary

In addition to storing infrastructure status updates, the backend database must support structured experiment-based measurements. During development of the Smart City Tile platform, the system was extended with a measurement session structure that allows infrastructure data to be grouped per experiment run. more on this can be found in my prior research (denzel, 2026)


---

# 4. Considered Database Options

Several relational database systems were evaluated:

## PostgreSQL

PostgreSQL is an open-source relational database system designed for reliability, extensibility, and data integrity. It supports ACID-compliant transactions and uses Multiversion Concurrency Control (MVCC), allowing multiple users and devices to read and write data simultaneously without blocking each other (PostgreSQL Global Development Group, 2024).

These characteristics make PostgreSQL suitable for real-time infrastructure monitoring systems that receive frequent updates from multiple IoT devices.

Additionally, PostgreSQL supports advanced indexing, extensible data types, and strong security features, making it appropriate for scalable smart city environments (DigitalOcean, 2023).

---

## MySQL and MariaDB

MySQL and MariaDB are widely used relational database systems known for strong performance and integration with web applications. They are suitable for dashboard-based monitoring systems and backend APIs but provide fewer extensibility features compared to PostgreSQL (Oracle Corporation, 2024; MariaDB Foundation, 2024).

---

## Microsoft SQL Server

Microsoft SQL Server provides enterprise-level analytics, security, and integration tools. However, licensing costs make it less suitable for educational environments or municipal prototype deployments where open-source solutions are preferred (Microsoft, 2024).

---

## SQLite

SQLite is a lightweight embedded database system designed for low-resource environments. However, SQLite allows only one write operation at a time due to its file-based locking mechanism, which limits performance when multiple devices attempt to send updates simultaneously (SQLite Consortium, 2024).

This makes SQLite less suitable for real-time IoT backend systems requiring concurrent write operations.

---

# 5. Recommendation

Based on the evaluated requirements and technical comparison, PostgreSQL is recommended as the database system for the Smart City Tile backend.

PostgreSQL supports structured infrastructure data storage and can handle multiple ESP32 devices sending updates simultaneously using MVCC concurrency control, which prevents read and write conflicts between database sessions (PostgreSQL Global Development Group, 2024).

It also provides strong transactional reliability through ACID compliance, ensuring that infrastructure status updates remain consistent even during simultaneous device communication.

Additionally, PostgreSQL is fully open source and does not require licensing costs. This makes it suitable for educational, research, and municipal smart city deployments.

Because the system may expand in the future with additional smart tiles and infrastructure components, PostgreSQL provides a scalable and future-proof solution capable of handling increased workloads and analytics integration.

---

# 7. Societal Impact

Using PostgreSQL as the backend database improves the reliability of infrastructure monitoring systems such as traffic lights and bridges. Reliable monitoring enables operators to respond faster to technical failures and improves traffic safety within smart city environments.

Because PostgreSQL is open source, implementation costs remain low. This makes smart infrastructure monitoring solutions more accessible to municipalities and educational institutions (DigitalOcean, 2023).

Secure and structured storage of infrastructure data also contributes to improved system stability and protection against unauthorized access, which is essential for public-sector digital infrastructure.

---

# 8. Conclusion

It is recommended to implement PostgreSQL as the backend database system communicating with ESP32 devices through a secure API connection.

The database should store device identifiers, timestamps, and infrastructure status updates in structured relational tables. In future iterations, the system can be extended with analytics features that support predictive maintenance of city infrastructure components.

This makes PostgreSQL a reliable, scalable, and cost-effective solution for the Smart City Tile backend architecture.

---

# 9. Sources

1. DigitalOcean. (2023). *PostgreSQL features and capabilities*.  
https://docs.digitalocean.com/products/databases/postgresql/details/features/

2. MariaDB Foundation. (2024). *MariaDB server documentation*.  
https://mariadb.org/documentation/

3. Microsoft. (2024). *SQL Server technical documentation*.  
https://learn.microsoft.com/en-us/sql/sql-server/

4. Oracle Corporation. (2024). *MySQL reference manual*.  
https://dev.mysql.com/doc/

5. PostgreSQL Global Development Group. (2024). *Multiversion concurrency control (MVCC)*.  
https://www.postgresql.org/docs/current/mvcc.html

6. SQLite Consortium. (2024). *Isolation in SQLite*.  
https://www.sqlite.org/isolation.html

7. Sign in · GitLab. (z.d.). https://gitlab.fdmci.hva.nl/studio/smart-cities/projecten/2025-2026-semester-2/city-sim-learning-group/city-smart-heaven-city-sim-learning-group/-/blob/main/docs/systems/database-research.md?ref_type=heads