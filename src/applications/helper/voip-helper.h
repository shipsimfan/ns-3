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
#ifndef VOIP_HELPER_H
#define VOIP_HELPER_H

#include "ns3/application-container.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include <ns3/voip.h>
#include <stdint.h>

namespace ns3 {

/**
 * \ingroup voip
 * \brief Create a server application which waits for input UDP packets
 *        and sends them back to the original sender.
 */
class VoIPServerHelper {
  public:
    /**
     * Create VoIPServerHelper which will make life easier for people trying
     * to set up simulations with echos.
     *
     * \param port The port the server will wait on for incoming packets
     */
    VoIPServerHelper(uint16_t port, uint32_t num_users, Codec codec);

    /**
     * Record an attribute to be set in each Application after it is is created.
     *
     * \param name the name of the attribute to set
     * \param value the value of the attribute to set
     */
    void SetAttribute(std::string name, const AttributeValue& value);

    /**
     * Create a VoIPServerApplication on the specified Node.
     *
     * \param node The node on which to create the Application.  The node is
     *             specified by a Ptr<Node>.
     *
     * \returns An ApplicationContainer holding the Application created,
     */
    ApplicationContainer Install(Ptr<Node> node) const;

    /**
     * Create a VoIPServerApplication on specified node
     *
     * \param nodeName The node on which to create the application.  The node
     *                 is specified by a node name previously registered with
     *                 the Object Name Service.
     *
     * \returns An ApplicationContainer holding the Application created.
     */
    ApplicationContainer Install(std::string nodeName) const;

    /**
     * \param c The nodes on which to create the Applications.  The nodes
     *          are specified by a NodeContainer.
     *
     * Create one udp echo server application on each of the Nodes in the
     * NodeContainer.
     *
     * \returns The applications created, one Application per Node in the
     *          NodeContainer.
     */
    ApplicationContainer Install(NodeContainer c) const;

  private:
    /**
     * Install an ns3::VoIPServer on the node configured with all the
     * attributes set with SetAttribute.
     *
     * \param node The node on which an VoIPServer will be installed.
     * \returns Ptr to the application installed.
     */
    Ptr<Application> InstallPriv(Ptr<Node> node) const;

    ObjectFactory m_factory; //!< Object factory.
};

/**
 * \ingroup voip
 * \brief Create an application which sends a UDP packet and waits for an echo
 * of this packet
 */
class VoIPClientHelper {
  public:
    /**
     * Create VoIPClientHelper which will make life easier for people trying
     * to set up simulations with echos. Use this variant with addresses that do
     * not include a port value (e.g., Ipv4Address and Ipv6Address).
     *
     * \param ip The IP address of the remote udp echo server
     * \param port The port number of the remote udp echo server
     */
    VoIPClientHelper(Address ip, uint16_t port, uint32_t id, Codec codec);
    /**
     * Create VoIPClientHelper which will make life easier for people trying
     * to set up simulations with echos. Use this variant with addresses that do
     * include a port value (e.g., InetSocketAddress and Inet6SocketAddress).
     *
     * \param addr The address of the remote udp echo server
     */
    VoIPClientHelper(Address addr);

    /**
     * Record an attribute to be set in each Application after it is is created.
     *
     * \param name the name of the attribute to set
     * \param value the value of the attribute to set
     */
    void SetAttribute(std::string name, const AttributeValue& value);

    /**
     * Create a udp echo client application on the specified node.  The Node
     * is provided as a Ptr<Node>.
     *
     * \param node The Ptr<Node> on which to create the VoIPClientApplication.
     *
     * \returns An ApplicationContainer that holds a Ptr<Application> to the
     *          application created
     */
    ApplicationContainer Install(Ptr<Node> node) const;

    /**
     * Create a udp echo client application on the specified node.  The Node
     * is provided as a string name of a Node that has been previously
     * associated using the Object Name Service.
     *
     * \param nodeName The name of the node on which to create the
     * VoIPClientApplication
     *
     * \returns An ApplicationContainer that holds a Ptr<Application> to the
     *          application created
     */
    ApplicationContainer Install(std::string nodeName) const;

    /**
     * \param c the nodes
     *
     * Create one udp echo client application on each of the input nodes
     *
     * \returns the applications created, one application per input node.
     */
    ApplicationContainer Install(NodeContainer c) const;

  private:
    /**
     * Install an ns3::VoIPClient on the node configured with all the
     * attributes set with SetAttribute.
     *
     * \param node The node on which an VoIPClient will be installed.
     * \returns Ptr to the application installed.
     */
    Ptr<Application> InstallPriv(Ptr<Node> node) const;
    ObjectFactory m_factory; //!< Object factory.
};

} // namespace ns3

#endif /* VoIP_HELPER_H */
