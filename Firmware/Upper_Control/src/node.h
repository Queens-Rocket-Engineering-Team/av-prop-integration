#ifndef NODE_H
#define NODE_H

#include <Arduino.h>
#include <cstddef>
#include <cstdint>

#include <aim_can_driver.h>
#include <aim_network.h>
#include <aim_safety.h>

#include "pinouts.h"

// Node-level identity and interface configuration lives in this file.
#define NODE_ORIGIN AIM_ORG_UPROP
#define NODE_NAME "UPPER_CONTROL"
#define NODE_PRIMARY_DEST AIM_DEST_LPROP

#define NODE_CAN_BAUD 500000U
#define NODE_CAN_RX_PIN CAN_RX_PIN
#define NODE_CAN_TX_PIN CAN_TX_PIN

#define NODE_SERIAL_BAUD 115200U

static constexpr uint8_t NODE_ENDPOINT_LOCAL_MAX = 15U;
static constexpr uint8_t NODE_ENDPOINT_LOWER_BASE = 16U;

enum NodeEndpointId : uint8_t {
  NODE_ENDPOINT_SYSTEM = 0U,
  NODE_ENDPOINT_PT1 = 1U,
  NODE_ENDPOINT_PT2 = 2U,
  NODE_ENDPOINT_VALVE1 = 4U,
  NODE_ENDPOINT_VALVE2 = 5U
};

enum NodeState : uint8_t {
  INIT = 0U,
  OPERATIONAL = 1U,
  DEBUG_CONSOLE = 2U,
  FLASH_DUMP = 3U,
  FLASH_ERASE = 4U,
  SAFE_MODE = 5U,
  LOW_POWER = 6U,
  FAULT = 7U
};

enum NodeActuatorCommand : uint32_t {
  NODE_ACTUATOR_CLOSED = 0U,
  NODE_ACTUATOR_OPEN = 1U
};

const char* nodeConfigJson(void);
size_t nodeConfigJsonLen(void);

bool nodeInitHardware(void);
void nodeServiceLocalTelemetry(uint32_t schedulerNowMs, uint32_t networkNowMs, AimNetwork& aim);
bool nodeHandleCanPacket(const aimPkt& pkt, uint32_t networkNowMs, AimNetwork& aim);

// Add node-specific periodic behavior in nodeUpdate().
void nodeUpdate(uint32_t schedulerNowMs);

#endif  // NODE_H
