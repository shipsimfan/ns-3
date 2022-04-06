#include "voip.h"
#include "ns3/simulator.h"

using namespace ns3;

VoIPPacket::VoIPPacket(uint32_t id, uint32_t index)
    : id(id), index(index), sent_time(Simulator::Now().GetSeconds()) {}

uint32_t VoIPPacket::GetID() const noexcept { return id; }

uint32_t VoIPPacket::GetIndex() const noexcept { return index; }

double VoIPPacket::GetSentTime() const noexcept { return sent_time; }

G711VoIPPacket::G711VoIPPacket(uint32_t id, uint32_t index)
    : VoIPPacket(id, index) {
    memset(data, 0, G711_DATA_LENGTH);
}

uint8_t* G711VoIPPacket::GetData() noexcept { return data; }

std::size_t G711VoIPPacket::GetPacketSize() const noexcept {
    return sizeof(G711VoIPPacket);
}

G726VoIPPacket::G726VoIPPacket(uint32_t id, uint32_t index)
    : VoIPPacket(id, index) {
    memset(data, 0, G726_DATA_LENGTH);
}

uint8_t* G726VoIPPacket::GetData() noexcept { return data; }

std::size_t G726VoIPPacket::GetPacketSize() const noexcept {
    return sizeof(G726VoIPPacket);
}
