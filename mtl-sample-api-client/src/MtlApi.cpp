#include "MtlApi.h"

#include "nlohmann/json.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>

unsigned const k_waitTimeoutMs = 5000;

MtlApi::MtlApi(std::string url) :
    m_bShutdown(false)
{
    m_jsonQueuePtr = std::make_shared<std::queue<nlohmann::json>>();
    m_client = std::make_shared<MtlWebSocket>(url, m_jsonQueuePtr);
    m_thread = ThreadPtr_t(new std::thread(&MtlApi::Receive_, this));
}

MtlApi::~MtlApi()
{
    Shutdown();
}

void MtlApi::Shutdown()
{
    m_bShutdown = true;
    if (m_thread->joinable())
        m_thread->join();
}

void MtlApi::start()
{
    // Start a background thread to receive messages
    m_client->start();
}

void MtlApi::stop()
{
    // Stop the connection to the websocket url
    m_client->stop();
}

void MtlApi::Receive_()
{
    while (!m_bShutdown)
    {
        // When a message is received from the background thread
        // it will be added to this queue. Any received message
        // will be written to 'output.txt'
        if (!m_jsonQueuePtr->empty())
        {
            nlohmann::json json = m_jsonQueuePtr->front();
            std::cout<<json.dump(4)<<std::endl;
            // Expected ["type"] values are {"gradeResponse", "heartbeat", "requestError"}
            if (json["type"] == "gradeResponse")
            {
                std::cout<<"gradeMessage Received"<<std::endl;
            }
            else if (json["type"] == "heartbeat")
            {
                std::cout<<"heartbeat Received"<<std::endl;
            }
            else if (json["type"] == "requestError")
            {
                std::cout<<"requestError Received"<<std::endl;
            }
            else
            {
                std::cout<<"Received unknown message: " << json["type"] <<std::endl;
            }
            std::ofstream file("output.txt", std::ofstream::app);
            file << json.dump(4) << std::endl;
            file.close();
            std::cout<<"Received message has been written to \"output.txt\""<<std::endl;
            m_jsonQueuePtr->pop();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(k_waitTimeoutMs));
    }
}

void MtlApi::sendGradeRequest(nlohmann::json gradeReqest)
{
    // Turn json object to a string
    std::string jsonStr = gradeReqest.dump();
    m_client->send(jsonStr);
}
