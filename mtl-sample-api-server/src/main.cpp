#include "SampleServer.h"

#include <getopt.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>

#define USAGE \
"Sample Server Usage:\n" \
"server -h --ip=<ip> --port=<port> --passive=<interval> \n" \
"   -h: help\n" \
"   -i, --ip: ip address server connects to\n" \
"   -p, --port: port server connects to\n" \
"   -v, --passive: server simulates sending live grade data as it is generated. Argument specifies grade frequency (in seconds)"

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

    std::cout << "[Main] ---Running Sample Server main.cpp---" << std::endl;

    std::string ipAddress = "127.0.0.1";
    unsigned port = 80;
    bool isPassivePublish = false;
    unsigned passivePublishInterval = 90;

    struct option long_options[] =
    {
        {"help",          no_argument, 0, 'h'},
        {"ip",      required_argument, 0, 'i'},
        {"port",    required_argument, 0, 'p'},
        {"passive", required_argument, 0, 'v'},
        {0,0,0,0}
    };
    int option_index = 0;
    int c;
    while ((c = getopt_long (argc, argv, "hi:p:v:", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case '0':
                break;
            case 'h':
                printf(USAGE);
                return 0;
            case 'i':
                ipAddress = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'v':
                std::cout << "[Main] Passive grade publishing mode activated" << std::endl;
                passivePublishInterval = atoi(optarg);
                isPassivePublish = true;
                break;
            case '?':
            default:
                break;
        }
    }

    SampleServer server(ipAddress, port);
    
    unsigned sleepInterval = 5;
    time_t intervalStartTime;
    time_t intervalCurrentTime;
    time(&intervalStartTime);
    while (!bShutdown)
    {
        server.SendHeartbeat();

        if (isPassivePublish)
        {
            time(&intervalCurrentTime);

            if (intervalCurrentTime - intervalStartTime > passivePublishInterval)
            {
                server.SendGradeResponse();
                intervalStartTime = intervalCurrentTime;
            }
        }

        sleep(sleepInterval);
    }

    std::cout << "[Main] Shutdown signal called, exiting" << std::endl;

    return 0;
}
