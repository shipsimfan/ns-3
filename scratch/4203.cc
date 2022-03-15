#include <ns3/buildings-helper.h>
#include <ns3/core-module.h>
#include <ns3/lte-module.h>
#include <ns3/mobility-module.h>
#include <ns3/network-module.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("4203Simulator");

enum Codec {
    G711,
    G729,
};

void display_simulation_info(Codec codec, int num_users);
void run_simulation(Codec codec, int num_users);

int main(int argc, char *argv[]) {
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
        codec = Codec::G711;
    else if (strcmp(argv[1], "g729") == 1)
        codec = Codec::G729;
    else {
        std::string error_message("Invalid codec: \"");
        error_message.append(argv[1]);
        error_message.append("\"");
        NS_LOG_ERROR(error_message);
        return 1;
    }

    // Parse node users
    int num_users = std::stoi(argv[2]);

    // Display simulation information
    display_simulation_info(codec, num_users);

    // Run simulation
    run_simulation(codec, num_users);

    NS_LOG_INFO("Simulation completed successfully");

    return 0;
}

const char *get_codec_name(Codec codec) {
    switch (codec) {
    case Codec::G711:
        return "G711";
    case Codec::G729:
        return "G729";
    default:
        return "Unknown Codec";
    }
}

void display_simulation_info(Codec codec, int num_users) {
    std::string codec_string("Selected Codec: ");
    codec_string.append(get_codec_name(codec));
    NS_LOG_INFO(codec_string);

    std::string num_string("Number of Users: ");
    num_string.append(std::to_string(num_users));
    NS_LOG_INFO(num_string);
}

// Modified from src/lte/examples/lena-simple.cc
void run_simulation(Codec codec, int num_users) {
    // Create LTE Helper
    Ptr<LteHelper> lte_helper = CreateObject<LteHelper>();

    // Create ENB Nodes
    NodeContainer base_station;
    base_station.Create(1);

    // Create UE Nodes
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

    // Create devices
    NetDeviceContainer base_station_devices;
    NetDeviceContainer user_devices;

    base_station_devices = lte_helper->InstallEnbDevice(base_station);
    user_devices = lte_helper->InstallUeDevice(users);

    // Attach the users to the base station
    lte_helper->Attach(user_devices, base_station_devices.Get(0));

    // Active a "data radio bearer" (i.e. the connection)
    enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
    EpsBearer bearer(q);
    lte_helper->ActivateDataRadioBearer(user_devices, bearer);
    lte_helper->EnableTraces();

    Simulator::Stop(Seconds(1.0));
    Simulator::Run();

    Simulator::Destroy();
}