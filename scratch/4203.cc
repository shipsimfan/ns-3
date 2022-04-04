#include <ns3/applications-module.h>
#include <ns3/buildings-helper.h>
#include <ns3/core-module.h>
#include <ns3/epc-helper.h>
#include <ns3/internet-module.h>
#include <ns3/lte-module.h>
#include <ns3/mobility-module.h>
#include <ns3/network-module.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/voip-helper.h>
#include <ns3/voip.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("4203Simulator");

void display_simulation_info(Codec codec, uint32_t num_users);
void run_simulation(Codec codec, uint32_t num_users);

int main(int argc, char* argv[]) {
    LogComponentEnable("4203Simulator", LOG_LEVEL_INFO);

    // Verify argument count
    if (argc != 3) {
        std::string usage_message("USAGE: ");
        usage_message.append(argv[0]);
        usage_message.append(" CODEC USERS");

        NS_LOG_ERROR("No specified codec or simulation parameters");
        NS_LOG_ERROR(usage_message);

        return 1;
    }

    // Parse codec
    Codec codec;
    if (strcmp(argv[1], "g711") == 0)
        codec = G711;
    else if (strcmp(argv[1], "g726") == 0)
        codec = G726;
    else {
        std::string error_message("Invalid codec: \"");
        error_message.append(argv[1]);
        error_message.append("\"");
        NS_LOG_ERROR(error_message);
        return 1;
    }

    // Parse node users
    uint32_t num_users = (uint32_t)std::stoi(argv[2]);

    // Display simulation information
    display_simulation_info(codec, num_users);

    // Run simulation
    run_simulation(codec, num_users);

    NS_LOG_INFO("Simulation completed successfully");

    return 0;
}

const char* get_codec_name(Codec codec) {
    switch (codec) {
    case G711:
        return "G711";
    case G726:
        return "G726";
    default:
        return "Unknown Codec";
    }
}

void display_simulation_info(Codec codec, uint32_t num_users) {
    std::string codec_string("Selected Codec: ");
    codec_string.append(get_codec_name(codec));
    NS_LOG_INFO(codec_string);

    std::string num_string("Number of Users: ");
    num_string.append(std::to_string(num_users));
    NS_LOG_INFO(num_string);
}

// Modified from src/lte/examples/lena-simple.cc
//   && src/lte/examples/lena-ipv6-ue-rh.cc

// Connection Type:    LTE     SGW     PGW     ETH
// Data Path:       UE --> ENB --> EPC --> P2P --> RH
void run_simulation(Codec codec, uint32_t num_users) {
    // Create LTE & EPC Helpers
    Ptr<LteHelper> lte_helper = CreateObject<LteHelper>();
    Ptr<PointToPointEpcHelper> epc_helper =
        CreateObject<PointToPointEpcHelper>();

    // Connects LTE --> EPC
    lte_helper->SetEpcHelper(epc_helper);

    // Get PDN Gateway (EPC --> Internet Gateway)
    Ptr<Node> pgw = epc_helper->GetPgwNode();

    // Create Remote Host
    NodeContainer remote_host_container;
    remote_host_container.Create(1);
    Ptr<Node> remote_host = remote_host_container.Get(0);
    InternetStackHelper internet;
    internet.Install(remote_host_container);

    // Create the Internet
    PointToPointHelper p2p_helper;
    p2p_helper.SetDeviceAttribute("DataRate",
                                  DataRateValue(DataRate("100Gb/s")));
    p2p_helper.SetDeviceAttribute("Mtu", UintegerValue(1500));
    p2p_helper.SetChannelAttribute("Delay", TimeValue(MilliSeconds(10)));
    // Connects PGW --> Internet (Point-to-Point) --> Remote Host
    NetDeviceContainer internet_devices = p2p_helper.Install(pgw, remote_host);

    // Create ENB Nodes (Base Stations)
    NodeContainer base_station;
    base_station.Create(1);

    // Create UE (User Endpoint) Nodes
    NodeContainer users;
    users.Create(num_users);

    // Constant position for the base station
    MobilityHelper mobility_helper;
    mobility_helper.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility_helper.Install(base_station);
    BuildingsHelper::Install(base_station);

    // Constant position for the users
    mobility_helper.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility_helper.Install(users);
    BuildingsHelper::Install(users);

    // Install the IP stack on the users
    internet.Install(users);

    // Connect the user devices to the LTE network
    NetDeviceContainer base_station_devices =
        lte_helper->InstallEnbDevice(base_station);
    NetDeviceContainer user_devices = lte_helper->InstallUeDevice(users);

    // Setup default routing for users
    Ipv6InterfaceContainer ue_ip_interface =
        epc_helper->AssignUeIpv6Address(NetDeviceContainer(user_devices));
    Ipv6StaticRoutingHelper ipv6_routing_helper;
    for (uint32_t i = 0; i < num_users; i++) {
        Ptr<Node> node = users.Get(i);
        Ptr<Ipv6StaticRouting> static_routing =
            ipv6_routing_helper.GetStaticRouting(node->GetObject<Ipv6>());
        static_routing->SetDefaultRoute(
            epc_helper->GetUeDefaultGatewayAddress6(), 1);
    }

    // Attach the users to the base station
    lte_helper->Attach(user_devices, base_station_devices.Get(0));

    // Assign IP addresses
    Ipv6AddressHelper ipv6_address_helper;
    ipv6_address_helper.SetBase(Ipv6Address("6001:db80::"), Ipv6Prefix(64));
    Ipv6InterfaceContainer internet_ip_interfaces =
        ipv6_address_helper.Assign(internet_devices);

    internet_ip_interfaces.SetForwarding(0, true);
    internet_ip_interfaces.SetDefaultRouteInAllNodes(0);

    Ptr<Ipv6StaticRouting> remote_host_static_routing =
        ipv6_routing_helper.GetStaticRouting(remote_host->GetObject<Ipv6>());
    remote_host_static_routing->AddNetworkRouteTo(
        "7777:f00d::", Ipv6Prefix(64), internet_ip_interfaces.GetAddress(0, 1),
        1, 0);

    Ipv6Address remote_host_addr = internet_ip_interfaces.GetAddress(1, 1);

    // Start echo applications
    VoIPServerHelper voip_server(9, num_users, codec);

    ApplicationContainer server_applications = voip_server.Install(remote_host);

    server_applications.Start(Seconds(1.0));
    server_applications.Stop(Seconds(21.0));

    ApplicationContainer client_applications[num_users];
    for (uint32_t i = 0; i < num_users; i++) {
        VoIPClientHelper voip_client(remote_host_addr, 9, i, codec);

        // How often to make a call
        voip_client.SetAttribute("Frequency", TimeValue(Seconds(8.0)));
        // How long for the call
        voip_client.SetAttribute("Duration", TimeValue(Seconds(2.0)));
        // When to start calling
        voip_client.SetAttribute("Start", TimeValue(Seconds((double)i)));

        client_applications[i] = voip_client.Install(users.Get(i));

        client_applications[i].Start(Seconds(1.0));
        client_applications[i].Stop(Seconds(20.0));
    }

    LogComponentEnable("VoIPClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("VoIPServerApplication", LOG_LEVEL_INFO);

    Simulator::Stop(Seconds(21.0));
    Simulator::Run();

    Simulator::Destroy();
}