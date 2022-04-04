#include "voip-client.h"
#include "codec-711.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"

#include "voip.h"
#include <fstream>
#include <math.h>

#define PI 3.1415926
#define PACKETRATE 50.0
namespace ns3 {

NS_LOG_COMPONENT_DEFINE("VoIPClientApplication");

NS_OBJECT_ENSURE_REGISTERED(VoIPClient);

const Time PACKET_INTERVAL = Seconds(1.0 / PACKETRATE);
const double SAMPLE_RATE = (1.0 / PACKETRATE) / 1024.0;

TypeId VoIPClient::GetTypeId(void) {
    static TypeId tid =
        TypeId("ns3::VoIPClient")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<VoIPClient>()
            .AddAttribute("ID", "User ID", UintegerValue(0),
                          MakeUintegerAccessor(&VoIPClient::m_id),
                          MakeUintegerChecker<uint32_t>())
            .AddAttribute(
                "Start", "When to start making calls", TimeValue(Seconds(1.0)),
                MakeTimeAccessor(&VoIPClient::m_start), MakeTimeChecker())
            .AddAttribute("Frequency", "How often to attempt a call",
                          TimeValue(Seconds(0.0)),
                          MakeTimeAccessor(&VoIPClient::m_frequency),
                          MakeTimeChecker())
            .AddAttribute(
                "Duration", "How long to make a call", TimeValue(Seconds(2.0)),
                MakeTimeAccessor(&VoIPClient::m_duration), MakeTimeChecker())
            .AddAttribute("RemoteAddress",
                          "The destination Address of the outbound packets",
                          AddressValue(),
                          MakeAddressAccessor(&VoIPClient::m_peerAddress),
                          MakeAddressChecker())
            .AddAttribute(
                "RemotePort", "The destination port of the outbound packets",
                UintegerValue(0), MakeUintegerAccessor(&VoIPClient::m_peerPort),
                MakeUintegerChecker<uint16_t>())
            .AddTraceSource("Tx", "A new packet is created and is sent",
                            MakeTraceSourceAccessor(&VoIPClient::m_txTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("Rx", "A packet has been received",
                            MakeTraceSourceAccessor(&VoIPClient::m_rxTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource(
                "TxWithAddresses", "A new packet is created and is sent",
                MakeTraceSourceAccessor(&VoIPClient::m_txTraceWithAddresses),
                "ns3::Packet::TwoAddressTracedCallback")
            .AddTraceSource(
                "RxWithAddresses", "A packet has been received",
                MakeTraceSourceAccessor(&VoIPClient::m_rxTraceWithAddresses),
                "ns3::Packet::TwoAddressTracedCallback");
    return tid;
}

VoIPClient::VoIPClient() {
    NS_LOG_FUNCTION(this);
    m_sent = 0;
    m_socket = 0;
    m_sendEvent = EventId();
}

VoIPClient::~VoIPClient() {
    NS_LOG_FUNCTION(this);
    m_socket = 0;
}

void VoIPClient::SetRemote(Address ip, uint16_t port) {
    NS_LOG_FUNCTION(this << ip << port);
    m_peerAddress = ip;
    m_peerPort = port;
}

void VoIPClient::SetRemote(Address addr) {
    NS_LOG_FUNCTION(this << addr);
    m_peerAddress = addr;
}

void VoIPClient::DoDispose(void) {
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
}

void VoIPClient::StartApplication(void) {
    NS_LOG_FUNCTION(this);

    if (m_socket == 0) {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        if (Ipv4Address::IsMatchingType(m_peerAddress) == true) {
            if (m_socket->Bind() == -1) {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            m_socket->Connect(InetSocketAddress(
                Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
        } else if (Ipv6Address::IsMatchingType(m_peerAddress) == true) {
            if (m_socket->Bind6() == -1) {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            m_socket->Connect(Inet6SocketAddress(
                Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));
        } else if (InetSocketAddress::IsMatchingType(m_peerAddress) == true) {
            if (m_socket->Bind() == -1) {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            m_socket->Connect(m_peerAddress);
        } else if (Inet6SocketAddress::IsMatchingType(m_peerAddress) == true) {
            if (m_socket->Bind6() == -1) {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            m_socket->Connect(m_peerAddress);
        } else {
            NS_ASSERT_MSG(false,
                          "Incompatible address type: " << m_peerAddress);
        }
    }

    m_next_index = 0;
    m_current = Seconds(0.);

    m_socket->SetAllowBroadcast(true);
    ScheduleTransmit(m_start);
}

void VoIPClient::StopApplication() {
    NS_LOG_FUNCTION(this);

    if (m_socket != 0) {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        m_socket = 0;
    }

    Simulator::Cancel(m_sendEvent);
}

void VoIPClient::ScheduleTransmit(Time dt) {
    NS_LOG_FUNCTION(this << dt);
    m_sendEvent = Simulator::Schedule(dt, &VoIPClient::Send, this);
}

short VoIPClient::VoiceGenerator(double time) {
    // input a time to the sound wave function
    // generate a double
    // use for encode

    // * 30000 just to get the double big enough to convert
    double voiceDataDouble = sin(440 * 2 * PI * time) * 30000;
    // convert it to short
    short voiceData = (short)voiceDataDouble;

    return voiceData;
}

void VoIPClient::Send(void) {
    NS_LOG_FUNCTION(this);

    NS_ASSERT(m_sendEvent.IsExpired());

    VoIPPacket vpacket;
    vpacket.id = m_id;
    vpacket.index = m_next_index;
    vpacket.sent_time = Simulator::Now().GetMilliSeconds();

    memset(vpacket.data, 0, sizeof(vpacket.data));

    // TODO: Generate voice data & encode it
    double now = Simulator::Now().GetSeconds();

    // make a input list
    short input[G711_DATA_LENGTH];

    // assign it through for loop
    for (int i = 0; i < G711_DATA_LENGTH; i++) {
        input[i] = VoiceGenerator(now + i * SAMPLE_RATE);
    }

    G711encode(input, vpacket.data);

    if (m_id == 0 && m_next_index == 0) {
        std::ofstream file("./G711encoding.csv");
        for (int i = 0; i < G711_DATA_LENGTH; i++) {
            file << input[i] << ", ";
        }
        file << std::endl;

        for (int i = 0; i < G711_DATA_LENGTH; i++) {
            file << (int)vpacket.data[i] << ", ";
        }

        G711decode(vpacket.data, input);

        file << std::endl;
        for (int i = 0; i < G711_DATA_LENGTH; i++) {
            file << input[i] << ", ";
        }
    }

    m_next_index++;

    Ptr<Packet> p = Create<Packet>((uint8_t*)(&vpacket), sizeof(vpacket));

    Address localAddress;
    m_socket->GetSockName(localAddress);
    // call to the trace sinks before the packet is actually sent,
    // so that tags added to the packet can be sent as well
    m_txTrace(p);
    if (Ipv4Address::IsMatchingType(m_peerAddress)) {
        m_txTraceWithAddresses(
            p, localAddress,
            InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress),
                              m_peerPort));
    } else if (Ipv6Address::IsMatchingType(m_peerAddress)) {
        m_txTraceWithAddresses(
            p, localAddress,
            Inet6SocketAddress(Ipv6Address::ConvertFrom(m_peerAddress),
                               m_peerPort));
    }
    m_socket->Send(p);
    ++m_sent;

    NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " client " << m_id
                           << " sent packet " << vpacket.index);

    m_current += PACKET_INTERVAL;

    if (m_current < m_duration)
        ScheduleTransmit(PACKET_INTERVAL);
    else if (m_frequency != Seconds(0.)) {
        m_current = Seconds(0.);
        ScheduleTransmit(m_frequency - m_duration);
    }
}

} // Namespace ns3
