#include "SampleServer.h"

#include <iostream>
#include <regex>

SampleServer::SampleServer(std::string const& webSocketIpAddress, unsigned webSocketPortNumber)
                : mk_ipAddress(webSocketIpAddress)
                , mk_portNumber(webSocketPortNumber)
                , m_wsServer(mk_portNumber, mk_ipAddress)
                , m_sampleDataStore()
{
    InitServer_();
}

SampleServer::~SampleServer()
{
    
}

void SampleServer::InitServer_()
{
    m_wsServer.setOnConnectionCallback(
        [this](std::shared_ptr<ix::WebSocket> pWebSocket,
               std::shared_ptr<ix::ConnectionState> pConnectionState)
        {
            pWebSocket->setOnMessageCallback(
                [pWebSocket, pConnectionState, this](const ix::WebSocketMessagePtr& msg)
                {
                    ServerMessageHandler_(pWebSocket, msg->type, msg->str);
                }
            );
        }
    );

    auto res = m_wsServer.listen();
    if (!res.first)
    {
        std::cout << "[SampleServer] InitServer failed" << std::endl;
    }
    else
    {
        std::cout << "[SampleServer] InitServer succeeded" << std::endl;

        m_wsServer.start();
    }
}

void SampleServer::SendHeartbeat()
{
    nlohmann::json heartbeat;

    heartbeat[mk_keyType]       = mk_heartbeatType;
    heartbeat[mk_keyVersion]    = mk_valueVersion;
    heartbeat[mk_keySystemId]   = mk_valueSystemId;
    heartbeat[mk_keyErrorCode]  = 0;

    for (auto&& client : m_wsServer.getClients())
    {
        std::cout << "[SampleServer] Sending Heartbeat" << std::endl;

        client->send(heartbeat.dump());
    }
}

void SampleServer::SendGradeResponse()
{
    nlohmann::json grades;
    std::string endTime = m_sampleDataStore.GetCurrentTimeAsString();
    timeval endTv = m_sampleDataStore.StringToTimeval(endTime);
    endTv.tv_sec = endTv.tv_sec - 60;
    std::string startTime = m_sampleDataStore.TimevalToString(endTv);

    std::string gradeResponseMsg = m_sampleDataStore.GetGradeResponse("", "testShovel", startTime, endTime, grades);

    for (auto&& client : m_wsServer.getClients())
    {
        std::cout << "[SampleServer] Sending GradeResponse" << std::endl;

        client->send(gradeResponseMsg);
    }
}

void SampleServer::ServerMessageHandler_(std::shared_ptr<ix::WebSocket> pSourceWebSocket, 
                                         ix::WebSocketMessageType messageType, const std::string& str)
{
    if (messageType == ix::WebSocketMessageType::Message)
    {
        std::string type, requestId, haulingUnitId, startTime, endTime, position;
        nlohmann::json grades;

        try
        {
            nlohmann::json json = nlohmann::json::parse(str);

            type = json[mk_keyType];
            
            if (type == mk_requestType)
            {
                requestId       = json[mk_keyRequestId];
                startTime       = json[mk_keyStartTime];
                endTime         = json[mk_keyEndTime];

                if (json.find(mk_keyHaulingUnitId) != json.end())
                {
                    haulingUnitId = json[mk_keyHaulingUnitId];
                }
                else
                {
                    haulingUnitId = "";
                }

                // "grades" and "position" are optional fields, we don't want a parse error for trying to read them
                grades = json[mk_keyGrades];
                if (json.find(mk_keyPosition) != json.end())
                {
                    position = json[mk_keyPosition];
                    ValidatePositionFormat_(position);
                }

                ValidateTimestampFormat_(startTime);
                ValidateTimestampFormat_(endTime);
                ValidateTimeInequality_(startTime, endTime);

                std::string gradeResponseMsg = m_sampleDataStore.GetGradeResponse(requestId, haulingUnitId, startTime, endTime, grades);

                for (auto&& client : m_wsServer.getClients())
                {
                    if (client == pSourceWebSocket)
                    {
                        std::cout << "[SampleServer] Sending GradeResponse" << std::endl;

                        client->send(gradeResponseMsg);
                    }
                }
            }
        }
        catch(std::invalid_argument e)
        {
            std::cout << "[SampleServer] Invalid argument found in incoming message: " << e.what() << std::endl;
            SendRequestError_(pSourceWebSocket, requestId, 6 /* incorrectly formatted value */, e.what());
        }
        catch(nlohmann::json::parse_error e)
        {
            std::cout << "[SampleServer] Failed to parse incoming message: " << e.what() << std::endl;
            SendRequestError_(pSourceWebSocket, requestId, 4 /* json parse failed */, e.what());
        }
        catch(std::exception e)
        {
            std::cout << "[SampleServer] Generic parse error: " << e.what() << std::endl;
            SendRequestError_(pSourceWebSocket, requestId, 5 /* required field missing */, 
                              "Generic parsing error, Likely caused by a missing field in the GradeRequest");
        }
    }
    else
    {
        std::cout << "[SampleServer] Not a 'message' type" << std::endl;
    }
}

void SampleServer::ValidatePositionFormat_(std::string coordinate)
{
    std::regex re(mk_coordinateRegex + ",\\s*" + mk_coordinateRegex + ",\\s*" + mk_coordinateRegex);
    std::smatch match;
    if (!std::regex_search(coordinate, match, re))
    {
        throw std::invalid_argument("Invalid Position Format");
    }
}

void SampleServer::ValidateTimestampFormat_(std::string timestamp)
{
    std::regex re("\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}\\.\\d{3}(Z|((\\+|\\-)\\d{2}:\\d{2}))");
    std::smatch match;
    if (!std::regex_search(timestamp, match, re))
    {
        throw std::invalid_argument("Invalid Timestamp Format");
    }
}

void SampleServer::ValidateTimeInequality_(std::string startTime, std::string endTime)
{
    timeval startTimeTv = m_sampleDataStore.StringToTimeval(startTime);
    timeval endTimeTv = m_sampleDataStore.StringToTimeval(endTime);
    if (timercmp(&startTimeTv, &endTimeTv, >)) {
        throw std::invalid_argument("Start time is earlier than End time");
    }
}

void SampleServer::SendRequestError_(std::shared_ptr<ix::WebSocket> pSourceWebSocket,
                                     std::string requestId, int errorCode, std::string errorString)
{
    nlohmann::json response;

    response[mk_keyType]        = mk_errorType;
    response[mk_keyRequestId]   = requestId;
    response[mk_keyErrorCode]   = errorCode;
    response[mk_keyErrorString] = errorString;

    for (auto&& client : m_wsServer.getClients())
    {
        if (client == pSourceWebSocket)
        {
            std::cout << "[SampleServer] Sending RequestError" << std::endl;

            client->send(response.dump());
        }
    }
}
