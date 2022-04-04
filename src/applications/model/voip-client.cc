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
const double SAMPLE_RATE = (1.0 / PACKETRATE) / CODEC_INPUT_LENGTH;

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
            .AddAttribute("Codec", "The type of codec",
                          UintegerValue(Codec::G711),
                          MakeUintegerAccessor(&VoIPClient::m_codec),
                          MakeUintegerChecker<bool>())
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

    // Make a packet
    VoIPPacket* vpacket = NULL;
    switch (m_codec) {
    case G711:
        vpacket = new G711VoIPPacket(m_id, m_next_index);
        break;

    case G726:
        vpacket = new G726VoIPPacket(m_id, m_next_index);
        break;
    }

    // Make an input list
    short* input = new short[CODEC_INPUT_LENGTH];

    // Generate voice data & encode it
    double now = Simulator::Now().GetSeconds();

    // Assign it through for loop
    for (int i = 0; i < CODEC_INPUT_LENGTH; i++)
        input[i] = VoiceGenerator(now + i * SAMPLE_RATE);

    // Encode input data
    switch (m_codec) {
    case G711:
        G711encode(input, vpacket->GetData());
        break;

    case G726:
        G726encode(input, vpacket->GetData());
        break;
    }

    // Write to a csv if this is the first packet by the first device
    if (m_id == 0 && m_next_index == 0) {
        std::ofstream* file = NULL;
        int data_length = 0;
        switch (m_codec) {
        case G711:
            file = new std::ofstream("G711encoding.csv");
            data_length = G711_DATA_LENGTH;
            break;

        case G726:
            file = new std::ofstream("G726encoding.csv");
            data_length = G726_DATA_LENGTH;
            break;
        }

        for (int i = 0; i < CODEC_INPUT_LENGTH; i++)
            *file << input[i] << ", ";

        *file << std::endl;

        for (int i = 0; i < data_length; i++)
            *file << (int)vpacket->GetData()[i] << ", ";

        *file << std::endl;

        switch (m_codec) {
        case G711:
            G711decode(vpacket->GetData(), input);
            break;

        case G726:
            G726decode(vpacket->GetData(), input);
            break;
        }

        for (int i = 0; i < CODEC_INPUT_LENGTH; i++)
            *file << input[i] << ", ";

        delete file;
    }

    Ptr<Packet> p = Create<Packet>((uint8_t*)vpacket, vpacket->GetPacketSize());

    delete vpacket;

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
                           << " sent packet " << m_next_index);

    m_next_index++;
    m_current += PACKET_INTERVAL;

    if (m_current < m_duration)
        ScheduleTransmit(PACKET_INTERVAL);
    else if (m_frequency != Seconds(0.)) {
        m_current = Seconds(0.);
        ScheduleTransmit(m_frequency - m_duration);
    }
}

} // Namespace ns3
