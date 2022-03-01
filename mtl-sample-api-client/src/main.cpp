#include "MtlApi.h"
#include "MtlWebSocket.h" 
#include "nlohmann/json.hpp"

#include <getopt.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>

#define USAGE \
"Sample Client Usage:\n" \
"client -h -r --ip=<ip> --port=<port> --start=<start> --end=<end> --id=<id> --haulingunit=<haulingunit> --interval=<interval>\n" \
"   -h: help\n" \
"   -i, --ip: ip address client connects to\n" \
"   -p, --port: port client connects to\n" \
"   -r: Send request messages\n" \
"   -s, --start: start time to set for the grade request\n" \
"   -e, --end: end time to set for the grade request\n" \
"   -d, --id: message id to send\n" \
"   -u, --haulingunit: identifier for the truck, belt, etc that's sending this request\n" \
"   -t, --interval: frequency at which we set grade requests\n"


bool bShutdown = false;
bool bRequest = false;

void SignalCallbackHandler(int signum)
{
    if (signum == SIGINT || signum == SIGTERM)
    {
        bShutdown = true;
    }
}

int main(int argc, char *argv[])
{
    signal(SIGINT,  SignalCallbackHandler);
    signal(SIGTERM, SignalCallbackHandler);

    std::string ipAddress = "127.0.0.1";
    std::string port = "80";

    std::string startTime = "2019-07-17T22:17:00.000Z";
    std::string endTime = "2019-07-17T22:19:00.000Z";
    std::string requestId = "1";
    std::string haulingUnitId = "truck1";

    int interval = 5;

    struct option long_options[] =
    {
        {"help",              no_argument, 0, 'h'},
        {"ip",          required_argument, 0, 'i'},
        {"port",        required_argument, 0, 'p'},
        {"request",           no_argument, 0, 'r'},
        {"start",       required_argument, 0, 's'},
        {"end",         required_argument, 0, 'e'},
        {"id",          required_argument, 0, 'd'},
        {"haulingunit", required_argument, 0, 'u'},
        {"interval",    required_argument, 0, 't'},
        {0,0,0,0}
    };
    int option_index = 0;
    int c;
    while ((c = getopt_long (argc, argv, "hi:p:rs:e:d:u:t:", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case '0':
                break;
            case 'h':
                printf( USAGE );
                return 0;
            case 'i':
                ipAddress = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            case 'r':
                std::cout << "---Sending gradeRequest messages and listening to heartbeat and gradeResponse messages---" << std::endl;
                bRequest = true;
                break;
            case 's':
                startTime = optarg;
                break;
            case 'e':
                endTime = optarg;
                break;
            case 'd':
                requestId = optarg;
                break;
            case 'u':
                haulingUnitId = optarg;
                break;
            case 't':
                interval = atoi(optarg);;
                break;
            case '?':
            default:
                break;
        }
    }

    nlohmann::json gradeRequest;

    /************************************************************
    *   Adding Required Fields                                  *
    *************************************************************/
    gradeRequest["type"] = "gradeRequest";
    gradeRequest["version"] = "2.3";
    gradeRequest["requestID"] = requestId;
    gradeRequest["startTime"] = startTime;
    gradeRequest["endTime"] = endTime;

    /************************************************************
    *   Adding Optional Fields                                  *
    *************************************************************/
    gradeRequest["haulingUnitID"] = haulingUnitId;
    gradeRequest["position"] = "5577.12,-444,10.5";
    gradeRequest["estimatedGrades"] = nlohmann::json::array();
    nlohmann::json sampleGradeFe =
    {
        {"element", "Fe"},
        {"value", 1.23}
    };
    nlohmann::json sampleGradeCu =
    {
        {"element", "Cu"},
        {"value", 0.65}
    };
    gradeRequest["estimatedGrades"].push_back(sampleGradeFe);
    gradeRequest["estimatedGrades"].push_back(sampleGradeCu);

    // Initilizing client with valid websocket url
    MtlApi client("ws://" + ipAddress + ":" + port);

    // Connect to websocket url
    client.start();

    while (!bShutdown)
    {
        if (bRequest)
        {
            client.sendGradeRequest(gradeRequest);
        }

        sleep(interval);
    }

    // Disconnect from websocket
    client.stop();

    return 0;
}
