#ifndef VOIP_H
#define VOIP_H

#include "ns3/type-name.h"

struct VoIPPacket {
    uint32_t id;
    uint32_t index;
    int64_t sent_time;
    uint8_t data[1024];
};

#endif