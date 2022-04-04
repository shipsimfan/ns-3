#ifndef VOIP_H
#define VOIP_H

#include "codec-711.h"
#include "codec-726.h"
#include "ns3/type-name.h"

#define CODEC_INPUT_LENGTH G711_DATA_LENGTH

enum Codec {
    G711 = 0,
    G726 = 1,
};

class VoIPPacket {
  public:
    VoIPPacket() = default;
    VoIPPacket(uint32_t id, uint32_t index);
    virtual ~VoIPPacket() = default;

    uint32_t GetID() const noexcept;
    uint32_t GetIndex() const noexcept;
    double GetSentTime() const noexcept;

    virtual uint8_t* GetData() noexcept = 0;
    virtual std::size_t GetPacketSize() const noexcept = 0;

  private:
    VoIPPacket(VoIPPacket&) = delete;

    uint32_t id;
    uint32_t index;
    double sent_time;
};

class G711VoIPPacket : public VoIPPacket {
  public:
    G711VoIPPacket() = default;
    G711VoIPPacket(uint32_t id, uint32_t index);
    ~G711VoIPPacket() = default;

    uint8_t* GetData() noexcept override;
    std::size_t GetPacketSize() const noexcept override;

  private:
    G711VoIPPacket(G711VoIPPacket&) = delete;

    uint8_t data[G711_DATA_LENGTH];
};

class G726VoIPPacket : public VoIPPacket {
  public:
    G726VoIPPacket() = default;
    G726VoIPPacket(uint32_t id, uint32_t index);
    ~G726VoIPPacket() = default;

    uint8_t* GetData() noexcept override;
    std::size_t GetPacketSize() const noexcept override;

  private:
    G726VoIPPacket(G726VoIPPacket&) = delete;

    uint8_t data[G726_DATA_LENGTH];
};

#endif