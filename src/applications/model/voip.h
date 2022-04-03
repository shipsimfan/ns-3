#ifndef VOIP_H
#define VOIP_H

#include "ns3/type-name.h"

#define G711_DATA_LENGTH 125

struct VoIPPacket {
    uint32_t id;
    uint32_t index;
    int64_t sent_time;
    uint8_t data[G711_DATA_LENGTH];
};

#endif