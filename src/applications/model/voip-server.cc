#include <ns3/address-utils.h>
#include <ns3/inet-socket-address.h>
#include <ns3/inet6-socket-address.h>
#include <ns3/ipv4-address.h>
#include <ns3/ipv6-address.h>
#include <ns3/log.h>
#include <ns3/nstime.h>
#include <ns3/packet.h>
#include <ns3/pointer.h>
#include <ns3/simulator.h>
#include <ns3/socket-factory.h>
#include <ns3/socket.h>
#include <ns3/udp-socket.h>
#include <ns3/uinteger.h>

#include "voip-server.h"
#include "voip.h"

namespace ns3 {
int numReceivedPackets = 0;
NS_LOG_COMPONENT_DEFINE("VoIPServerApplication");

NS_OBJECT_ENSURE_REGISTERED(VoIPServer);

TypeId VoIPServer::GetTypeId(void) {
    static TypeId tid =
        TypeId("ns3::VoIPServer")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<VoIPServer>()
            .AddAttribute("NumUsers", "Number of users", UintegerValue(0),
                          MakePointerAccessor(&VoIPServer::m_users),
                          MakePointerChecker<UsersStat>())
            .AddAttribute("Codec", "The type of codec",
                          UintegerValue(Codec::G711),
                          MakeUintegerAccessor(&VoIPServer::m_codec),
                          MakeUintegerChecker<bool>())
            .AddAttribute(
                "Port", "Port on which we listen for incoming packets.",
                UintegerValue(9), MakeUintegerAccessor(&VoIPServer::m_port),
                MakeUintegerChecker<uint16_t>())
            .AddTraceSource("Rx", "A packet has been received",
                            MakeTraceSourceAccessor(&VoIPServer::m_rxTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource(
                "RxWithAddresses", "A packet has been received",
                MakeTraceSourceAccessor(&VoIPServer::m_rxTraceWithAddresses),
                "ns3::Packet::TwoAddressTracedCallback");
    return tid;
}

VoIPServer::VoIPServer() { NS_LOG_FUNCTION(this); }

VoIPServer::~VoIPServer() {
    NS_LOG_FUNCTION(this);
    m_socket = 0;
    m_socket6 = 0;
}

void VoIPServer::DoDispose(void) {
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
}

void VoIPServer::StartApplication(void) {
    NS_LOG_FUNCTION(this);

    if (m_socket == 0) {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        InetSocketAddress local =
            InetSocketAddress(Ipv4Address::GetAny(), m_port);
        if (m_socket->Bind(local) == -1) {
            NS_FATAL_ERROR("Failed to bind socket");
        }
        if (addressUtils::IsMulticast(m_local)) {
            Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket>(m_socket);
            if (udpSocket) {
                // equivalent to setsockopt (MCAST_JOIN_GROUP)
                udpSocket->MulticastJoinGroup(0, m_local);
            } else {
                NS_FATAL_ERROR("Error: Failed to join multicast group");
            }
        }
    }

    if (m_socket6 == 0) {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket6 = Socket::CreateSocket(GetNode(), tid);
        Inet6SocketAddress local6 =
            Inet6SocketAddress(Ipv6Address::GetAny(), m_port);
        if (m_socket6->Bind(local6) == -1) {
            NS_FATAL_ERROR("Failed to bind socket");
        }
        if (addressUtils::IsMulticast(local6)) {
            Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket>(m_socket6);
            if (udpSocket) {
                // equivalent to setsockopt (MCAST_JOIN_GROUP)
                udpSocket->MulticastJoinGroup(0, local6);
            } else {
                NS_FATAL_ERROR("Error: Failed to join multicast group");
            }
        }
    }

    m_socket->SetRecvCallback(MakeCallback(&VoIPServer::HandleRead, this));
    m_socket6->SetRecvCallback(MakeCallback(&VoIPServer::HandleRead, this));
}

void VoIPServer::StopApplication() {
    NS_LOG_FUNCTION(this);

    if (m_socket != 0) {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
    if (m_socket6 != 0) {
        m_socket6->Close();
        m_socket6->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
    // calculation using m_user
    // NS_LOG_UNCOND(Simulator::Now().As(Time::S));
    // NS_LOG_INFO(m_users->num_users);

    double usentTime;
    double ureceiveTime;
    int packetLoss;
    double jitter = 0;
    double sentTime1;
    double sentTime2;
    double receiveTime1;
    double receiveTime2;
    double jitterPerUser = 0;
    int numJitter = 0;
    double totalJitter = 0;
    double throughput;
    double sumDelay = 0;
    double throughputSum = 0;
    double packetLossSum = 0;

    for (int i = 0; i < m_users->num_users; i++) {
        double EndToEndDelay;

        for (int j = 0; j < m_users->users[i].packetTimes.size(); j++) {
            usentTime = m_users->users[i].packetTimes[j].sentTime;
            ureceiveTime = m_users->users[i].packetTimes[j].receivedTime;

            EndToEndDelay += ureceiveTime - usentTime;
        }
        // rounding to four decimal places
        EndToEndDelay = round((EndToEndDelay /
                               (double)m_users->users[i].packetTimes.size()) *
                              10000) /
                        10000;
        sumDelay += EndToEndDelay;
        NS_LOG_INFO("End to end delay for user " << i << " is " << EndToEndDelay
                                                 << "s");

        packetLoss =
            ((m_users->users[i].missed_packets) /
             ((m_users->users[i].missed_packets) + numReceivedPackets)) *
            100;
        packetLossSum += packetLoss;
        NS_LOG_INFO("Packet loss for user " << i << " is " << packetLoss
                                            << "%");

        for (int j = 1; j < m_users->users[i].packetTimes.size(); j++) {
            sentTime1 = m_users->users[i].packetTimes[j - 1].sentTime;
            sentTime2 = m_users->users[i].packetTimes[j].sentTime;

            receiveTime1 = m_users->users[i].packetTimes[j - 1].receivedTime;
            receiveTime2 = m_users->users[i].packetTimes[j].receivedTime;

            double ji = (receiveTime1 - sentTime1) - (receiveTime2 - sentTime2);
            jitter += ji < 0 ? -ji : ji;
            numJitter++;
        }
        jitterPerUser = (jitter / numJitter);
        totalJitter += jitterPerUser;
        numJitter = 0;
        jitter = 0;
        NS_LOG_INFO("Jitter for user " << i << " is " << jitterPerUser << "s");

        std::size_t bytes_received = sizeof(VoIPPacket);
        // using end to end delay but not sure what to include in simulator time
        throughput = (bytes_received * 8) / (EndToEndDelay / 1000000) / 20.0;
        throughputSum += throughput;
        NS_LOG_INFO("Throughput for user " << i << " is " << throughput / 1000
                                           << "kbit/s");
        NS_LOG_INFO("");
    }
    NS_LOG_INFO("Average of End to End Delay for all users is "
                << sumDelay / (m_users->num_users) << "s");
    NS_LOG_INFO("Sum of throughput all users is " << throughputSum / 1000
                                                  << "kbit/s");
    NS_LOG_INFO("Average of packet loss for all users is "
                << packetLossSum / (m_users->num_users) << "%");
    NS_LOG_INFO("Average of Jitter for all users is "
                << totalJitter / (m_users->num_users) << "s");
}

void VoIPServer::HandleRead(Ptr<Socket> socket) {
    NS_LOG_FUNCTION(this << socket);

    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while ((packet = socket->RecvFrom(from))) {
        socket->GetSockName(localAddress);
        m_rxTrace(packet);
        m_rxTraceWithAddresses(packet, from, localAddress);

        packet->RemoveAllPacketTags();
        packet->RemoveAllByteTags();

        VoIPPacket* vpacket = NULL;
        switch (m_codec) {
        case G711:
            vpacket = new G711VoIPPacket();
            break;

        case G726:
            vpacket = new G726VoIPPacket();
            break;
        }

        packet->CopyData((uint8_t*)vpacket, vpacket->GetPacketSize());

        NS_LOG_INFO("At time " << Simulator::Now().As(Time::S)
                               << " server recieved packet "
                               << vpacket->GetIndex() << " from client "
                               << vpacket->GetID());

        if (vpacket->GetID() > m_users->num_users)
            NS_LOG_ERROR("Packet id greater than number of users");
        else {
            if (vpacket->GetIndex() ==
                m_users->users[vpacket->GetID()].next_index)
                m_users->users[vpacket->GetID()].next_index++;
            else if (vpacket->GetIndex() >
                     m_users->users[vpacket->GetID()].next_index) {
                m_users->users[vpacket->GetID()].missed_packets +=
                    vpacket->GetIndex() -
                    m_users->users[vpacket->GetID()].next_index;

                m_users->users[vpacket->GetID()].next_index =
                    vpacket->GetIndex() + 1;
            }
            m_users->users[vpacket->GetID()].packetTimes.push_back(PacketTime(
                vpacket->GetSentTime(), Simulator::Now().GetSeconds()));
        }

        delete vpacket;
        numReceivedPackets++;
    }
}

VoIPServer::UsersStat::UsersStat() {
    NS_LOG_FUNCTION(this);
    users = NULL;
    num_users = 0;
}

VoIPServer::UsersStat::UsersStat(uint32_t num_users) : num_users(num_users) {
    NS_LOG_FUNCTION(this << num_users);
    users = new VoIPServer::UserStat[num_users];
    for (uint32_t i = 0; i < num_users; i++) {
        users[i].next_index = 0;
        users[i].missed_packets = 0;
        users[i].packetTimes = std::vector<PacketTime>();
    }
}

VoIPServer::UsersStat::~UsersStat() {
    NS_LOG_FUNCTION(this);
    if (users != NULL)
        delete users;
}

VoIPServer::PacketTime::PacketTime(double sentTime, double receivedTime)
    : sentTime(sentTime), receivedTime(receivedTime) {}

} // Namespace ns3