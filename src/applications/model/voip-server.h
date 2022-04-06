#ifndef VOIP_SERVER_H
#define VOIP_SERVER_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "voip.h"

#include <vector>

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup applications
 * \defgroup voip VoIP
 */

/**
 * \ingroup voip
 * \brief A VoIP server
 *
 * Every packet received is sent back.
 */
class VoIPServer : public Application {
  public:
    struct PacketTime {
        double sentTime;
        double receivedTime;

        PacketTime(double sentTime, double receivedTime);
    };
    struct UserStat {
        uint32_t next_index;
        uint32_t missed_packets;
        std::vector<PacketTime> packetTimes;
    };

    struct UsersStat : public Object {
        UserStat* users;
        uint32_t num_users;

        UsersStat();
        UsersStat(uint32_t num_users);
        ~UsersStat();
    };

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId(void);
    VoIPServer();
    virtual ~VoIPServer();

  protected:
    virtual void DoDispose(void);

  private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    /**
     * \brief Handle a packet reception.
     *
     * This function is called by lower layers.
     *
     * \param socket the socket the packet was received to.
     */
    void HandleRead(Ptr<Socket> socket);

    uint16_t m_port;       //!< Port on which we listen for incoming packets.
    Ptr<Socket> m_socket;  //!< IPv4 Socket
    Ptr<Socket> m_socket6; //!< IPv6 Socket
    Address m_local;       //!< local multicast address

    Ptr<UsersStat> m_users;
    Codec m_codec;

    /// Callbacks for tracing the packet Rx events
    TracedCallback<Ptr<const Packet>> m_rxTrace;

    /// Callbacks for tracing the packet Rx events, includes source and
    /// destination addresses
    TracedCallback<Ptr<const Packet>, const Address&, const Address&>
        m_rxTraceWithAddresses;
};

} // namespace ns3

#endif /* VoIP_SERVER_H */
