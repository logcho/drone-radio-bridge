# ugv-radio-bridge

**Jetson Nano &harr; Radio Module &harr; Ground Station**

The UGV needs to be able to communicate with either the Ground Station or the UAV over a radio link.

This is a simple, reliable packet protocol that:
- Sends commands from the ground/UAV &larr; UGV
- Sends telemetry from UGV &larr; ground/UAV
- Handles packet loss, timing, and failsafes
- Runs on your Jetson Nano + ROS2

---

**The goal** is to build a minimal communication layer that is reliable and easy to debug for competition UGV tasks.

ROS2 doesn't automatically run over radios.

ROS2 requires IP-level networking.
The radio module probably uses serial, UART, or low-bandwidth wireless that cannot directly run DDS/ROS2 messages.

This is the translator layer.

Radios drop packets — framing, CRC, and heartbeats

**Raw serial data is unreliable.**

We need:
- packet delimiters
- length field
- CRC integrity check
- sequence numbers
- timeouts

This is the entire point of this protocol.

**Packet Structure**

Every packet follows this binary layout:

```
[HEADER][LEN][MSG_ID][SEQ][PAYLOAD...][CRC16]
```

| Field       | Size     | Description                          |
| ----------- | -------- | ------------------------------------ |
| **HEADER**  | 2 bytes  | Always `0xAA 0x55` (start of packet) |
| **LEN**     | 1 byte   | Number of bytes from MSG_ID → CRC    |
| **MSG_ID**  | 1 byte   | Identifies what the packet means     |
| **SEQ**     | 1 byte   | Increases each packet; detects loss  |
| **PAYLOAD** | variable | Depends on message                   |
| **CRC16**   | 2 bytes  | CRC-16/CCITT-FALSE                   |

**Why:**
- HEADER = framing
- LEN = knowing packet size
- SEQ = detect duplicates and drops
- CRC = detect corruption
- Binary = efficient for low-bandwidth radios

**Message ID Table**

Commands (Ground → UGV)

| ID     | Name         | Payload                      | Description          |
| ------ | ------------ | ---------------------------- | -------------------- |
| `0x01` | HEARTBEAT    | none                         | Keeps UGV alive      |
| `0x02` | CMD_VEL      | float32 lin_x, float32 ang_z | Drive commands       |
| `0x03` | SET_MODE     | uint8 mode_id                | Auto/Manual/Stop     |
| `0x04` | SET_WAYPOINT | float32 x, y, yaw            | Navigation           |
| `0x05` | ESTOP        | none                         | Immediate motor stop |

Telemetry (UGV → Ground)

| ID     | Name         | Payload                     | Description        |
| ------ | ------------ | --------------------------- | ------------------ |
| `0x10` | TELE_POSE    | float32 x, y, yaw           | UGV's position     |
| `0x11` | TELE_BATTERY | float32 voltage             | Battery monitoring |
| `0x12` | TELE_STATUS  | uint8 status_id             | OK/FAILSAFE/ERROR  |
| `0x13` | TELE_ARUCO   | float32 x, y, yaw, int32 id | Marker detection   |

**UGV Modes**

| mode_id | Meaning |
| ------- | ------- |
| 0       | MANUAL  |
| 1       | AUTO    |
| 2       | STOP    |

**Status Codes**

| status_id | Meaning                   |
| --------- | ------------------------- |
| 0         | OK                        |
| 1         | FAILSAFE (lost heartbeat) |
| 2         | ESTOP (override)          |
| 3         | ERROR                     |

**CRC Specification**

We use industry-standard CRC-16/CCITT-FALSE:
- Polynomial: 0x1021
- Init value: 0xFFFF
- Final XOR: 0x0000
- Bits not reflected

**Why**: Reliable, widely used in robotics, prevents corrupted packets.

**Timing & Heartbeats**

**Ground → UGV heartbeat:**
- Sent every 100 ms
- If none received for > 500 ms, UGV enters FAILSAFE (stop motors)

**UGV → Ground telemetry:**
- Sent every 200–300 ms

This ensures:
- stable control
- predictable radio load
- safety if link dies

**Command Flow**
- Ground station sends CMD_VEL packet (lin_x, ang_z)
- Jetson Nano receives packet
- ROS2 node publishes to /cmd_vel
- FC/motor controller applies speeds

**Telemetry Flow**
- Jetson computes pose or receives ArUco data
- ROS2 publishes /pose
- Radio bridge encodes TELE_POSE packet
- Ground station receives updated position

**Failsafe Flow**
- UGV stops receiving heartbeats
- After 500 ms &larr; FAILSAFE
- Sends TELE_STATUS with status_id = 1
- Motors go to safe stop