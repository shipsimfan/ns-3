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

namespace ns3 {

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
                          MakePointerChecker<UserPairs>())
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
        }

        delete vpacket;
    }
}

VoIPServer::UserPairs::UserPairs() {
    NS_LOG_FUNCTION(this);
    users = NULL;
    num_users = 0;
}

VoIPServer::UserPairs::UserPairs(uint32_t num_users) : num_users(num_users) {
    NS_LOG_FUNCTION(this << num_users);
    users = new VoIPServer::UserPair[num_users];
    for (uint32_t i = 0; i < num_users; i++) {
        users[i].next_index = 0;
        users[i].missed_packets = 0;
    }
}

VoIPServer::UserPairs::~UserPairs() {
    NS_LOG_FUNCTION(this);
    if (users != NULL)
        delete users;
}

} // Namespace ns3