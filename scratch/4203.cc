#include <ns3/core-module.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("4203Simulator");

enum Codec {
    G711,
    G729,
};

void display_simulation_info(Codec codec, int *node_users, int node_users_len);

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

int main(int argc, char *argv[]) {
    // Verify argument count
    if (argc < 3) {
        std::string usage_message("USAGE: ");
        usage_message.append(argv[0]);
        usage_message.append(" CODEC NODE_USERS+");

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
    int node_users[argc - 2];
    for (int i = 2; i < argc; i++)
        node_users[i - 2] = std::stoi(argv[i], 0, 0);

    // Display simulation information
    display_simulation_info(codec, node_users, argc - 2);

    // Create simulation

    return 0;
}

void display_simulation_info(Codec codec, int *node_users, int node_users_len) {
    std::string codec_string("Selected Codec: ");
    codec_string.append(get_codec_name(codec));
    NS_LOG_INFO(codec_string);

    for (int i = 0; i < node_users_len; i++) {
        std::string display("Node ");
        display.append(std::to_string(i + 1));
        display.append(": ");
        display.append(std::to_string(node_users[i]));
        display.append(" Users");
        NS_LOG_INFO(display);
    }
}