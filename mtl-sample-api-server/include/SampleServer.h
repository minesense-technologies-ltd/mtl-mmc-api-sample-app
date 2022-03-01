#pragma once

/*--------------------------------------------------------------------------------------
* Copyright (c) 2005-2019 by MineSense Technologies Ltd.("Minesense"). All rights
* reserved.
*
* The copyright to the computer software herein is the property of MineSense.
* The software may be used and/or copied only with the written permission of
* MineSense or in accordance with the terms and conditions stipulated in
* the agreement/contract under which the software has been supplied.
*------------------------------------------------------------------------------------*/

#include "SampleDataStore.h"

#include "IXWebSocketServer.h"
#include "nlohmann/json.hpp"

class SampleServer
{
public:
    SampleServer(std::string const& webSocketIpAddress, unsigned webSocketPortNumber);

    ~SampleServer();

    /** SendHeartbeat
     * Send a heartbeat message containing basic health status data to all connected clients
     */
    void SendHeartbeat();

    /** SendGradeResponse
     * Send a grade response message containing randomized grade data
     */
    void SendGradeResponse();

private:
    std::string const mk_ipAddress;
    unsigned    const mk_portNumber;

    ix::WebSocketServer m_wsServer;

    SampleDataStore m_sampleDataStore;

    void InitServer_();
    void ServerMessageHandler_(std::shared_ptr<ix::WebSocket> pSourceWebSocket, 
                               ix::WebSocketMessageType messageType, const std::string& str);

    void ValidatePositionFormat_(std::string coordinate);
    void ValidateTimestampFormat_(std::string timestamp);
    void ValidateTimeInequality_(std::string startTime, std::string endTime);

    void SendRequestError_(std::shared_ptr<ix::WebSocket> pSourceWebSocket,
                           std::string requestId, int errorCode, std::string errorString);

    // Message keys and values
    std::string const mk_keyType            = "type";

    // Keys shared across multiple message types
    std::string const mk_keyRequestId       = "requestID"; // gradeRequest and requestError
    std::string const mk_keyErrorCode       = "errorCode"; // heartbeat and requestError

    // heartbeat keys
    std::string const mk_heartbeatType      = "heartbeat";
    std::string const mk_keyVersion         = "version";
    std::string const mk_keySystemId        = "systemID";
    std::string const mk_valueVersion       = "2.3";
    std::string const mk_valueSystemId      = "testServer";

    // gradeRequest
    std::string const mk_requestType        = "gradeRequest";
    std::string const mk_keyHaulingUnitId   = "haulingUnitID";
    std::string const mk_keyPosition        = "position";
    std::string const mk_keyStartTime       = "startTime";
    std::string const mk_keyEndTime         = "endTime";
    std::string const mk_keyGrades          = "estimatedGrades";
    std::string const mk_keyGradeElement    = "element";
    std::string const mk_keyGradeValue      = "value";

    // requestError
    std::string const mk_errorType          = "requestError";
    std::string const mk_keyErrorString     = "errorString";
    
    // Position validation constant
    std::string const mk_coordinateRegex = "((-?)([0-9]+)(\\.([0-9]+))?)";
};
