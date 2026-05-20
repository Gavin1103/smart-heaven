# Raspberry PI Deployment - Advice

* **Written by:** Gavin Tjin, software engineer
* **Date:** 14-05-2026
* **Classifications:** Public
* **Version:** 1.1
* **Target audience:** Backend and embedded engineers
* **Keywords:** Raspberry Pi, Telemetry, IoT, NestJS, Smart City, API
* **Client:** Gerald Stap (mayor)
* **Company:** Smart Heaven

# Table of contents

1. Introduction
2. Telemetry communication

* 2.1 Introduction
* 2.2 Purpose of telemetry
* 2.3 Recommended communication flow
* 2.4 Partial conclusion

3. Generic telemetry payload

* 3.1 Introduction
* 3.2 Payload structure
* 3.3 Device-specific data
* 3.4 Payload mapping to database fields
* 3.5 Partial conclusion

4. Backend processing

* 4.1 Introduction
* 4.2 Device identification
* 4.3 Device state updates
* 4.4 Automatic log creation
* 4.5 Partial conclusion

5. Request examples

* 5.1 Introduction
* 5.2 Bridge request example
* 5.3 Traffic light request example
* 5.4 Partial conclusion

6. Security and reliability considerations

* 6.1 Introduction
* 6.2 Authentication
* 6.3 Error handling
* 6.4 Partial conclusion

7. Conclusion
8. Recommendations
9. References

# 1. Introduction

In the Smart Heaven project, multiple embedded devices communicate with the backend API. These devices include bridge
nodes, traffic lights, railway barriers, parking sensors, and other Smart City components. Each device can generate
state changes, sensor readings, warnings, and errors.

To process this data in a maintainable way, the backend API should not expose separate endpoints for every device type.
Instead, a generic telemetry endpoint is recommended. Telemetry means that embedded devices automatically send technical
data, such as events, states, and sensor values, to a central backend system.

This advice document builds upon the previous analysis and design documents. The analysis showed that a NestJS API can
run on a Raspberry Pi using Docker, while the design document defined a scalable database structure with separate
entities for devices, device states, and logs.

This advice focuses on how embedded devices should send data to the API and how the backend should process that data
into the existing `DEVICE_STATE` and `LOG` entities.

The following main question is central within this advice research:

**How should embedded Smart City devices send telemetry data to the Smart Heaven API in a generic, scalable, and
maintainable way?**

To answer this question, the following subquestions are defined:

* Why should embedded devices communicate through a generic telemetry endpoint?
* Which payload structure should be used to support multiple device types?
* How should the backend process telemetry into device states and logs?
* What request examples can be used by embedded engineers for bridges and traffic lights?
* Which security and reliability considerations should be taken into account?

# 2. Telemetry communication

## 2.1 Introduction

This chapter explains why telemetry is suitable for communication between embedded devices and the Smart Heaven API.

## 2.2 Purpose of telemetry

Telemetry is the automatic transfer of technical data from a device to a central system (A10 Networks, 2024). In the
Smart Heaven project, this means that devices such as bridges, traffic lights, and railway barriers send their current
state, events, and sensor values to the NestJS backend.

Examples of telemetry events include:

* A bridge detecting a boat
* A traffic light changing from red to green
* A railway barrier opening or closing
* A sensor reporting an error
* A device sending a heartbeat

Telemetry is suitable for this project because embedded devices generate data automatically during operation. The
frontend does not need to request every change manually. Instead, the devices push relevant updates to the backend API.

## 2.3 Recommended communication flow

The recommended flow is:

```txt
Embedded device
    ↓
POST /api/v1/telemetry
    ↓
TelemetryController
    ↓
TelemetryService
    ↓
Update DEVICE_STATE
Create LOG
```

This means that the embedded side only needs to send one API request when something happens. The backend then decides
how this data is stored.

This approach is preferred over requiring embedded devices to call separate endpoints such as `/device-states` and
`/logs`,
because
that would create more network traffic and more failure points. If one request succeeds and the other fails, the
database could become inconsistent. By handling this inside the backend, the API can update the current state and create
a log entry in one controlled process. This approach aligns with the principle of atomic transactions, where related
operations are processed together to maintain consistency and reliability within the system (GeeksforGeeks, 2025).

## 2.4 Partial conclusion

A generic telemetry flow is the most suitable communication approach because it keeps the embedded implementation simple
while allowing the backend to remain responsible for data processing. Embedded devices only send one request, and the
backend updates the current state and stores a historical log entry.

# 3. Generic telemetry payload

## 3.1 Introduction

This chapter describes the recommended payload structure for telemetry requests.

## 3.2 Payload structure

Because Smart Heaven contains multiple device types, the telemetry payload should be generic. The top-level fields
should be the same for every device, while device-specific values should be placed inside the `data` object.

The recommended payload structure is:

```json
{
  "deviceCode": "bridge-01",
  "eventType": "BRIDGE_OPENED",
  "state": "Open",
  "level": "INFO",
  "message": "Bridge opened successfully",
  "data": {
    "fromState": "Opening",
    "toState": "Open",
    "uptimeMs": 98450,
    "sensors": {
      "boatDetected": true,
      "obstacleDetected": false
    }
  }
}
```

### Field explanation

| Field        | Type   | Description                                                                   |
|--------------|--------|-------------------------------------------------------------------------------|
| `deviceCode` | string | Unique device identifier used by the embedded device and backend              |
| `eventType`  | string | Type of event, such as `BRIDGE_OPENED` or `TRAFFIC_LIGHT_CHANGED`             |
| `state`      | string | Current functional state of the device                                        |
| `level`      | string | Severity level: `INFO`, `WARNING`, or `ERROR`                                 |
| `message`    | string | Human-readable message for dashboards or debugging                            |
| `data`       | object | Optional device-specific details such as sensors, transitions, or timing data |

## 3.3 Device-specific data

The `data` object should contain values that are specific to a device type. This keeps the API generic while still
allowing each device to send relevant technical details.

For example, a bridge may send:

```json
{
  "fromState": "IdleClosed",
  "toState": "BoatDetected",
  "sensors": {
    "boatDetected": true,
    "reedClosed": true,
    "stepperPos": 0
  }
}
```

A traffic light may send:

```json
{
  "previousColor": "RED",
  "currentColor": "GREEN",
  "durationSeconds": 30,
  "pedestrianButtonPressed": false
}
```

This prevents the database schema from needing new columns for every hardware type. Instead, device-specific values can
be stored in flexible JSON structures. Flexible schemas are commonly used in systems where different devices or data
sources produce varying types of information (Towards flexible data schemas, 2024). It also matches the flexible data
and payload JSON fields defined in the database design.

## 3.4 Payload mapping to database fields

When the backend receives a telemetry request, the top-level fields are mapped to the corresponding database entities as
follows:

| Telemetry field | Database entity | Database field   |
|-----------------|-----------------|------------------|
| `state`         | `DEVICE_STATE`  | `state`          |
| `message`       | `DEVICE_STATE`  | `status_message` |
| `data`          | `DEVICE_STATE`  | `data`           |
| `eventType`     | `LOG`           | `event_type`     |
| `level`         | `LOG`           | `level`          |
| `message`       | `LOG`           | `message`        |
| `data`          | `LOG`           | `payload`        |

This means that the backend uses a single incoming `data` object to populate both `DEVICE_STATE.data` and `LOG.payload`.
The embedded device does not need to be aware of this separation, as the backend handles the mapping internally.

## 3.5 Partial conclusion

A generic telemetry payload is recommended because it allows all embedded devices to communicate with the same endpoint.
The required top-level fields provide structure, while the `data` object allows each device type to send its own
specific details without requiring database changes. The backend is responsible for mapping incoming fields to the
correct database entities, keeping the embedded implementation as simple as possible.

# 4. Backend processing

## 4.1 Introduction

This chapter explains how the backend should process incoming telemetry requests.

## 4.2 Device identification

The backend should identify devices using `deviceCode` instead of the internal database `id`. This is recommended
because internal database IDs should not be exposed to embedded devices. Using separate public identifiers improves the
abstraction between the API and the database structure and reduces direct dependency on internal database records
(Anwar, 2021). A device code is also more readable and more stable for hardware communication.

Example:

```json
{
  "deviceCode": "bridge-01"
}
```

The backend should use this value to find the corresponding `DEVICE` record.

If the device code does not exist, the API should return a `404 Not Found` response.

## 4.3 Device state updates

After the device is found, the backend should update the `DEVICE_STATE` entity. Because each device has at most one
current state, the backend should use an upsert operation:

* If a state already exists for the device, update it.
* If no state exists yet, create it.

The `state` field should contain the current state of the device. For example:

```txt
Bridge: Open
Traffic light: GREEN
Railway barrier: CLOSED
Parking sensor: OCCUPIED
```

The `data` field should store additional runtime information.

## 4.4 Automatic log creation

After the current state is updated, the backend should automatically create a `LOG` entry. This log entry stores the
historical event.

This means that embedded devices do not need to send a second request to create a log. The backend should create the log
as part of the telemetry process.

For example, if a bridge sends:

```json
{
  "eventType": "BRIDGE_OPENED",
  "state": "Open"
}
```

The backend updates `DEVICE_STATE` to `Open` and creates a log with event type `BRIDGE_OPENED`.

The recommended backend process is:

```txt
1. Receive telemetry request
2. Find device by deviceCode
3. Upsert DEVICE_STATE
4. Create LOG
5. Return success response
```

If possible, the state update and log creation should happen in one database transaction. This keeps the data
consistent. If one operation fails, the other should not be committed either. Database transactions are designed to
bundle multiple operations into one all-or-nothing process, which helps prevent partial updates and inconsistent data
(3.4. Transactions, 2026).

## 4.5 Partial conclusion

The backend should be responsible for converting telemetry requests into device state updates and log entries. Device
identification is handled using `deviceCode` rather than internal database IDs, which keeps communication readable and
prevents exposure of internal identifiers. This keeps embedded devices simple and prevents inconsistent data. Using a
transaction for state and log creation improves reliability and ensures that current state and historical logs remain
connected.

# 5. Request examples

## 5.1 Introduction

This chapter provides request examples for embedded engineers. These examples show how different device types can use
the same generic telemetry endpoint.

## 5.2 Bridge request example

This example is based on a bridge detecting a boat. The bridge moves from `IdleClosed` to `BoatDetected`.

```http
POST /api/v1/telemetry
Content-Type: application/json
```

```json
{
  "deviceCode": "bridge-01",
  "eventType": "BOAT_DETECTED",
  "state": "BoatDetected",
  "level": "INFO",
  "message": "Boat detected near bridge",
  "data": {
    "uptimeMs": 45231,
    "eventCode": 2,
    "fromState": "IdleClosed",
    "toState": "BoatDetected",
    "faultReason": null,
    "sensors": {
      "boatDetected": true,
      "obstacleDetected": false,
      "carDetected": false,
      "reedClosed": true,
      "stepperPos": 0
    }
  }
}
```

This request results in the following backend behaviour:

```txt
DEVICE_STATE.state = BoatDetected
DEVICE_STATE.statusMessage = Boat detected near bridge
LOG.eventType = BOAT_DETECTED
LOG.level = INFO
```

## 5.3 Traffic light request example

This example shows a traffic light changing from red to green.

```http
POST /api/v1/telemetry
Content-Type: application/json
```

```json
{
  "deviceCode": "traffic-light-01",
  "eventType": "TRAFFIC_LIGHT_CHANGED",
  "state": "GREEN",
  "level": "INFO",
  "message": "Traffic light changed from RED to GREEN",
  "data": {
    "previousState": "RED",
    "newState": "GREEN",
    "durationSeconds": 30,
    "pedestrianButtonPressed": false,
    "intersection": "Main street crossing"
  }
}
```

This request results in the following backend behaviour:

```txt
DEVICE_STATE.state = GREEN
DEVICE_STATE.statusMessage = Traffic light changed from RED to GREEN
LOG.eventType = TRAFFIC_LIGHT_CHANGED
LOG.level = INFO
```

## 5.4 Partial conclusion

Both the bridge and traffic light can use the same telemetry endpoint because the top-level payload structure is
generic. Device-specific information is placed inside the `data` object, allowing each embedded system to send its own
relevant values without requiring separate endpoints.

# 6. Security and reliability considerations

## 6.1 Introduction

This chapter describes important security and reliability considerations for telemetry communication.

## 6.2 Authentication

During local development, telemetry requests may be accepted without authentication if the API only runs on a local
network. However, when the API is deployed outside a local network, authentication should be added.

The recommended approach is to use a device API key. Each device should have its own API key, and the backend should
store only the hashed version of that key in the `DEVICE` entity. Storing hashed API keys reduces the risk of exposing
sensitive credentials if the database is compromised (Dhandala, 2026).

The device can then send the key using a header:

```http
x-api-key: <device-api-key>
```

Using separate API keys per device is preferred over sharing one global secret across all devices, because individual
keys can be rotated, revoked, or disabled when needed. This improves security and device management (Dhandala, 2026).

## 6.3 Error handling

The embedded device should treat a successful `2xx` response as confirmation that the telemetry was received. If the
backend returns an error, the device may log the error locally.

For the current Smart Heaven implementation, it is an intentional trade-off that devices do not retry failed telemetry
requests. According to Gregersen (2024), a _best-effort_ delivery approach without retransmission can be suitable in
IoT environments where occasional data loss is acceptable for non-critical updates. This approach simplifies the
embedded implementation and reduces the risk of duplicate entries. However, it also means that missing telemetry records
may represent connectivity gaps rather than actual system events. For this project, this trade-off is considered
acceptable because reducing implementation complexity is prioritized over guaranteed delivery.

The backend should return clear error responses:

| Situation                  | Recommended status          |
|----------------------------|-----------------------------|
| Invalid payload            | `400 Bad Request`           |
| Invalid or missing API key | `401 Unauthorized`          |
| Device not found           | `404 Not Found`             |
| Server/database error      | `500 Internal Server Error` |

## 6.4 Partial conclusion

Telemetry should be kept simple during local development, but security should be added when the system is exposed
outside the local network. Device-level API keys are recommended because they fit the existing database design and allow
individual devices to be managed securely. On the reliability side, the decision not to implement retry logic is a
deliberate trade-off that keeps the embedded implementation simple at the cost of occasional data gaps during
connectivity failures. Clear error responses from the backend allow embedded devices to handle errors locally when
needed.

# 7. Conclusion

This advice research examined how embedded Smart City devices should send telemetry data to the Smart Heaven API in a
generic, scalable, and maintainable way.

From the first subquestion, it can be concluded that a generic telemetry endpoint is the most suitable approach because
it allows all embedded devices to send data through one consistent API flow.

The second subquestion showed that the telemetry payload should use fixed top-level fields such as `deviceCode`,
`eventType`, `state`, `level`, and `message`, while storing device-specific details in the flexible `data` object. The
backend maps these fields to the correct database entities, populating both `DEVICE_STATE` and `LOG` from a single
request.

The third subquestion demonstrated that the backend should process telemetry by identifying the device using
`deviceCode`, updating `DEVICE_STATE`, and automatically creating a `LOG` entry, preferably within a single database
transaction.

The fourth subquestion showed that both bridge and traffic light devices can use the same request structure, even though
their device-specific data differs.

The fifth subquestion showed that authentication using device-level API keys and structured error handling should be
considered, especially when the API is exposed outside the local network. The decision not to implement retry logic is a
deliberate trade-off that simplifies the embedded implementation.

**Based on these findings, the main question can be answered as follows:**

Embedded Smart City devices should send telemetry data to the Smart Heaven API through a generic
`POST /api/v1/telemetry` endpoint. The request should contain a standard top-level structure for identification, event
type, current state, log level, and message, while device-specific values should be placed inside a flexible `data`
object.

The backend should then convert this telemetry request into a current `DEVICE_STATE` update and a historical `LOG`
entry. This approach keeps embedded communication simple, reduces unnecessary API calls, and supports multiple hardware
types without requiring separate endpoints or frequent database changes.

# 8. Recommendations

Based on the telemetry flow and the existing database design, the following recommendations are made:

* Use one generic telemetry endpoint for all embedded devices, because this keeps the API consistent and avoids
  device-specific endpoints.

* Require a shared top-level payload structure, because this makes validation, logging, and backend processing easier.

* Store device-specific values inside the `data` object, because bridges, traffic lights, railway barriers, and parking
  sensors all require different runtime details.

* Use `deviceCode` instead of internal database IDs, because this keeps embedded communication readable and avoids
  exposing internal identifiers.

* Let the backend automatically create logs after telemetry is received, because embedded devices should not need to
  make separate API calls for state and logging.

* Use a database transaction when updating `DEVICE_STATE` and creating a `LOG`, because this prevents inconsistent data.

* Add device-level API key authentication before exposing the API outside a local network, because embedded devices
  communicate directly with the backend.

* Document the telemetry contract clearly for embedded engineers, because all devices must follow the same structure for
  the API to remain generic.

* Accept the trade-off of not implementing retry logic on embedded devices, but document this explicitly so that
  connectivity gaps can be distinguished from missing events during monitoring.

# 9. References

* Dhandala, N. (2026, February 20). API key Management Best Practices for Secure Services. OneUptime | One Complete
  Observability platform. Retrieved May 8, 2026,
  from [https://oneuptime.com/blog/post/2026-02-20-api-key-management-best-practices/view#storing-api-keys-securely](https://oneuptime.com/blog/post/2026-02-20-api-key-management-best-practices/view#storing-api-keys-securely)
* What is IoT Telemetry? | Glossary | A10 Networks. (2024, May 23). A10
  Networks. https://www.a10networks.com/glossary/what-is-iot-telemetry/
* GeeksforGeeks. (2025, July 23). Using atomic transactions to power an idempotent API.
  GeeksforGeeks. https://www.geeksforgeeks.org/dbms/using-atomic-transactions-to-power-an-idempotent-api/
* Towards flexible data schemas. (2024, 13 mei). Medium. Retrieved May 8, 2026,
  from https://medium.com/radiant-earth-insights/towards-flexible-data-schemas-483735a0993c
* Anwar. (2021, September 25). Do not expose database ids in your URLs. DEV
  Community. Retrieved May 8, 2026, from https://dev.to/anwar_nairi/do-not-expose-database-ids-in-your-urls-567
* 3.4. transactions. (2026, May 14). PostgreSQL
  Documentation. https://www.postgresql.org/docs/current/tutorial-transactions.html
* Gregersen, C. (2024, November 19). Ensuring quality of service (QOS) in IoT networks.
  Nabto. https://www.nabto.com/ensuring-quality-of-service-qos-in-iot-networks/