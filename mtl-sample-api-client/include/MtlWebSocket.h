#pragma once
/*--------------------------------------------------------------------------------------
 * Copyright (c) 2005-2019 by MineSense Technologies Ltd.("MineSense"). All rights
 * reserved.
 *
 * The copyright to the computer software herein is the property of MineSense.
 * The software may be used and/or copied only with the written permission of
 * MineSense or in accordance with the terms and conditions stipulated in
 * the agreement/contract under which the software has been supplied.
 *------------------------------------------------------------------------------------*/

/**
 * Class that connects and disconnects from a given websocket url. In charge of sending
 * and receiving messages from a server.
 */

#include "IXWebSocket.h"
#include "IXSocket.h"
#include "nlohmann/json.hpp"

#include <string>
#include <queue>

typedef std::shared_ptr<std::queue<nlohmann::json>> JsonQueuePtr_t;

class MtlWebSocket
{
public:
    /**
     * Constructor.
     * @param url a url websocket
     * @param queuePtr a queue pointer for json objects
     */
    MtlWebSocket(std::string url, JsonQueuePtr_t queuePtr);

    /**
     * Connects to the websocket url.
     */
    void start();

    /**
     * Disconnects from the websocket url.
     */
    void stop();

    /**
     * Sends a JSON string to the websocket.
     * @param jsonStr JSON string
     */
    void send(std::string jsonStr);

    /**
     * Function to handle when messages are received. Received messages are added
     * to the queuePtr.
     *
     * Obtained from IXWebSocket library
     */
    void messageHandler(const ix::WebSocketMessagePtr& msg);

private:
    std::string m_url;
    JsonQueuePtr_t m_jsonQueuePtr;
    ix::WebSocket m_webSocket;
};

typedef std::shared_ptr<MtlWebSocket> MtlWebSocketPtr_t;
