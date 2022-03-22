#ifndef VOIP_H
#define VOIP_H

#include "ns3/type-name.h"

struct VoIPPacket {
    uint32_t id;
    uint32_t index;
    uint8_t data[1024];
};

#endif