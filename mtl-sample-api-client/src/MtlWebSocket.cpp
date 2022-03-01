#include "MtlWebSocket.h"

#include "IXWebSocket.h"
#include "IXSocket.h"
#include "nlohmann/json.hpp"

#include <iostream>
#include <queue>
#include <sstream>
#include <string>

MtlWebSocket::MtlWebSocket(std::string url, JsonQueuePtr_t queuePtr) :
    m_url(url),
    m_jsonQueuePtr(queuePtr)
{
}

void MtlWebSocket::messageHandler(const ix::WebSocketMessagePtr& msg)
{
    std::stringstream ss;
    if (msg->type == ix::WebSocketMessageType::Open)
    {
        std::cout << "Uri: " << msg->openInfo.uri << std::endl;
        std::cout << "Handshake Headers:" << std::endl;
        for (auto it : msg->openInfo.headers)
        {
            std::cout << it.first << ": " << it.second << std::endl;
        }
    }
    else if (msg->type == ix::WebSocketMessageType::Close)
    {
        ss << "MtlWebSocket: disconnected:"
           << " code " << msg->closeInfo.code
           << " reason " << msg->closeInfo.reason
           << msg->str;
        std::cout<<ss.str()<<std::endl;
    }
    else if (msg->type == ix::WebSocketMessageType::Message)
    {
        // When a message is received, added it to the queue
        try
        {
            nlohmann::json json = nlohmann::json::parse(msg->str);
            m_jsonQueuePtr->push(json);
        }
        catch (nlohmann::json::parse_error &e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
    else if (msg->type == ix::WebSocketMessageType::Error)
    {
        ss << "Connection error: " << msg->errorInfo.reason      << std::endl;
        ss << "#retries: "         << msg->errorInfo.retries     << std::endl;
        ss << "Wait time(ms): "    << msg->errorInfo.wait_time   << std::endl;
        ss << "HTTP Status: "      << msg->errorInfo.http_status << std::endl;
        std::cout<<ss.str()<<std::endl;
    }
    else
    {
        ss << "Invalid ix::WebSocketMessageType";
        std::cout<<ss.str()<<std::endl;
    }
}
void MtlWebSocket::start()
{
    m_webSocket.setUrl(m_url);
    std::cout<<"Connecting to:" + m_url<<std::endl;
    m_webSocket.setOnMessageCallback([this]
                                    (const ix::WebSocketMessagePtr& msg)
    {
        messageHandler(msg);
    });

    // Start background thread to send and receive messsages
    m_webSocket.start();
}

void MtlWebSocket::stop()
{
    // Stop the connection to the websocket url
    m_webSocket.stop();
}

void MtlWebSocket::send(std::string jsonStr)
{
    // Sends a JSON string to the server
    m_webSocket.send(jsonStr);
}
