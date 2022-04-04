#ifndef VOIP_CLIENT_H
#define VOIP_CLIENT_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup voip
 * \brief A VoIP client
 *
 * Every packet sent should be returned by the server and received here.
 */
class VoIPClient : public Application {
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId(void);

    VoIPClient();

    virtual ~VoIPClient();

    /**
     * \brief set the remote address and port
     * \param ip remote IP address
     * \param port remote port
     */
    void SetRemote(Address ip, uint16_t port);
    /**
     * \brief set the remote address
     * \param addr remote address
     */
    void SetRemote(Address addr);

  protected:
    virtual void DoDispose(void);

  private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    /**
     * \brief Schedule the next packet transmission
     * \param dt time interval between packets.
     */
    void ScheduleTransmit(Time dt);

    short VoiceGenerator(double time);

    /**
     * \brief Send a packet
     */
    void Send(void);

    Time m_frequency;
    Time m_duration;
    Time m_start;
    uint32_t m_id;
    uint32_t m_next_index;
    Time m_current;

    uint32_t m_sent;       //!< Counter for sent packets
    Ptr<Socket> m_socket;  //!< Socket
    Address m_peerAddress; //!< Remote peer address
    uint16_t m_peerPort;   //!< Remote peer port
    EventId m_sendEvent;   //!< Event to send the next packet

    /// Callbacks for tracing the packet Tx events
    TracedCallback<Ptr<const Packet>> m_txTrace;

    /// Callbacks for tracing the packet Rx events
    TracedCallback<Ptr<const Packet>> m_rxTrace;

    /// Callbacks for tracing the packet Tx events, includes source and
    /// destination addresses
    TracedCallback<Ptr<const Packet>, const Address&, const Address&>
        m_txTraceWithAddresses;

    /// Callbacks for tracing the packet Rx events, includes source and
    /// destination addresses
    TracedCallback<Ptr<const Packet>, const Address&, const Address&>
        m_rxTraceWithAddresses;
};

} // namespace ns3

#endif /* VoIP_CLIENT_H */
