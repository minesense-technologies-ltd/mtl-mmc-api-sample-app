#include "MtlApi.h"
#include "MtlWebSocket.h"

#include "IXWebSocketServer.h"
#include "nlohmann/json.hpp"
#include <gtest/gtest.h>

#include <fstream>
#include <streambuf>
#include <unistd.h>

bool startEchoServer(ix::WebSocketServer& server)
{
    server.setOnConnectionCallback(
        [](std::shared_ptr<ix::WebSocket> webSocket,
           std::shared_ptr<ix::ConnectionState> connectionState)
        {
            webSocket->setOnMessageCallback(
                [webSocket, connectionState](ix::WebSocketMessageType messageType,
                    const std::string& str,
                    size_t wireSize,
                    const ix::WebSocketErrorInfo& error,
                    const ix::WebSocketOpenInfo& openInfo,
                    const ix::WebSocketCloseInfo& closeInfo)
                {
                    if (messageType == ix::WebSocket_MessageType_Open)
                    {
                        std::cerr << "New connection" << std::endl;
                        std::cerr << "Uri: " << openInfo.uri << std::endl;
                        std::cerr << "Headers:" << std::endl;
                        for (auto it : openInfo.headers)
                        {
                            std::cerr << it.first << ": " << it.second << std::endl;
                        }
                    }
                    else if (messageType == ix::WebSocket_MessageType_Close)
                    {
                        std::cerr << "Closed connection"
                                    << " code " << closeInfo.code
                                    << " reason " << closeInfo.reason << std::endl;
                    }
                    else if (messageType == ix::WebSocket_MessageType_Error)
                    {
                        std::stringstream ss;
                        ss << "Connection error: " << error.reason      << std::endl;
                        ss << "#retries: "         << error.retries     << std::endl;
                        ss << "Wait time(ms): "    << error.wait_time   << std::endl;
                        ss << "HTTP Status: "      << error.http_status << std::endl;
                        std::cerr << ss.str();
                    }
                    else if (messageType == ix::WebSocket_MessageType_Message)
                    {
                        std::cerr << "Received "
                                    << wireSize << " bytes"
                                    << std::endl;
                        webSocket->send(str);
                    }
                }
            );
        }
    );

    auto res = server.listen();
    if (!res.first)
    {
        std::cerr << res.second << std::endl;
        return false;
    }

    server.start();
    return true;
}

std::string exec(const char* cmd)
{
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    try
    {
        while (fgets(buffer, sizeof buffer, pipe) != NULL)
        {
            result += buffer;
        }
    }
    catch (...)
    {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}


TEST(MtlWebSocket, send)
{
    MtlWebSocketPtr_t client;
    JsonQueuePtr_t jsonQueuePtr = std::make_shared<std::queue<nlohmann::json>>();
    std::string url = "ws://127.0.0.1:8080";
    client = std::make_shared<MtlWebSocket>(url, jsonQueuePtr);
    client->start();
    // Wait for client to connect to server
    sleep(1);

    nlohmann::json jsonEmpty;
    client->send(jsonEmpty.dump());

    nlohmann::json jsonHeader;
    jsonHeader["type"] = "gradeRequest";
    client->send(jsonHeader.dump());

    nlohmann::json jsonFull;
    jsonFull["type"] = "gradeRequest";
    jsonFull["truckId"] = 16;
    jsonFull["startTime"] = "2019-03-20 09:04:05.001";
    jsonFull["endTime"] = "2019-03-20 09:05:11.112";
    client->send(jsonFull.dump());

    // Wait for message to return from echo server
    sleep(1);

    EXPECT_EQ(jsonQueuePtr->front().dump(), jsonEmpty.dump());
    jsonQueuePtr->pop();
    EXPECT_EQ(jsonQueuePtr->front().dump(), jsonHeader.dump());
    jsonQueuePtr->pop();
    EXPECT_EQ(jsonQueuePtr->front().dump(), jsonFull.dump());
    jsonQueuePtr->pop();

    client->stop();
}

TEST(MtlApi, verifyReceivedMessage)
{
    MtlApi apiClient("ws://127.0.0.1:8080");
    apiClient.start();
    // Wait for client to connect to server
    sleep(1);
    nlohmann::json gradeRequest;
    gradeRequest["type"] = "gradeRequest";
    gradeRequest["requestID"] = "1";
    gradeRequest["startTime"] = "2019-05-25 22:15:00.000";
    gradeRequest["endTime"] = "2019-05-25 22:16:00.000";
    apiClient.sendGradeRequest(gradeRequest);
    // Wait for message to return from echo server
    sleep(1);
    apiClient.stop();

    // Longer sleep here to allow return message to be written to a file
    sleep(5);
    // Read file to one string
    std::ifstream file("output.txt");
    std::string output((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    exec("rm output.txt");
}

int main(int argc, char *argv[])
{
    // Create echo server to send and receive messages from
    std::string hostname = "127.0.0.1";
    int port = 8080;
    ix::WebSocketServer server(port,hostname);
    std::cout << "Listening on " << hostname << ":" << port << std::endl;
    startEchoServer(server);
    sleep(1);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
