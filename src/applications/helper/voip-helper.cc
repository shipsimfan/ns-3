/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "voip-helper.h"
#include "ns3/names.h"
#include "ns3/uinteger.h"
#include "ns3/voip-client.h"
#include "ns3/voip-server.h"
#include <ns3/pointer.h>

namespace ns3 {

VoIPServerHelper::VoIPServerHelper(uint16_t port, uint32_t num_users,
                                   Codec codec) {
    m_factory.SetTypeId(VoIPServer::GetTypeId());
    SetAttribute("Port", UintegerValue(port));
    SetAttribute("NumUsers",
                 PointerValue(new VoIPServer::UsersStat(num_users)));
    SetAttribute("Codec", UintegerValue(codec));
}

void VoIPServerHelper::SetAttribute(std::string name,
                                    const AttributeValue& value) {
    m_factory.Set(name, value);
}

ApplicationContainer VoIPServerHelper::Install(Ptr<Node> node) const {
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer VoIPServerHelper::Install(std::string nodeName) const {
    Ptr<Node> node = Names::Find<Node>(nodeName);
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer VoIPServerHelper::Install(NodeContainer c) const {
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i) {
        apps.Add(InstallPriv(*i));
    }

    return apps;
}

Ptr<Application> VoIPServerHelper::InstallPriv(Ptr<Node> node) const {
    Ptr<Application> app = m_factory.Create<VoIPServer>();
    node->AddApplication(app);

    return app;
}

VoIPClientHelper::VoIPClientHelper(Address address, uint16_t port, uint32_t id,
                                   Codec codec) {
    m_factory.SetTypeId(VoIPClient::GetTypeId());
    SetAttribute("RemoteAddress", AddressValue(address));
    SetAttribute("RemotePort", UintegerValue(port));
    SetAttribute("ID", UintegerValue(id));
    SetAttribute("Codec", UintegerValue(codec));
}

VoIPClientHelper::VoIPClientHelper(Address address) {
    m_factory.SetTypeId(VoIPClient::GetTypeId());
    SetAttribute("RemoteAddress", AddressValue(address));
}

void VoIPClientHelper::SetAttribute(std::string name,
                                    const AttributeValue& value) {
    m_factory.Set(name, value);
}

ApplicationContainer VoIPClientHelper::Install(Ptr<Node> node) const {
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer VoIPClientHelper::Install(std::string nodeName) const {
    Ptr<Node> node = Names::Find<Node>(nodeName);
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer VoIPClientHelper::Install(NodeContainer c) const {
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i) {
        apps.Add(InstallPriv(*i));
    }

    return apps;
}

Ptr<Application> VoIPClientHelper::InstallPriv(Ptr<Node> node) const {
    Ptr<Application> app = m_factory.Create<VoIPClient>();
    node->AddApplication(app);

    return app;
}

} // namespace ns3
