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
 * Wrapper for MtlWebSocket. This class is in charge of issuing start and stop commands
 * to the websocket. This class also formats grade request to json strings that are passed on to
 * MtlWebSocket. This class will also display received messages/heartbeats from server and any
 * extra information such as heartbeat timeouts, or system errors.
 */

#include "MtlWebSocket.h"
#include "nlohmann/json.hpp"

#include <string>
#include <queue>

class MtlApi
{
public:
    /**
     * Constructor.
     * Creates a queue for json objects and a websocket client initilized with the passed in url.
     * @param url a websocket url
     */
    MtlApi(std::string url);

    /**
     * Destructor.
     */
    ~MtlApi();

    /**
     * Gently shutsdown thread.
     */
    void Shutdown();

    /**
     * Connect to the websocket url.
     */
    void start();

    /**
     * Disconnect from the websocket url.
     */
    void stop();

    /**
     * Sends formatted json object to the websocket
     * @param gradeReqest formated json object for grade requests
     * @see createGradeRequest
     */
    void sendGradeRequest(nlohmann::json gradeRequest);

private:
    typedef std::unique_ptr<std::thread> ThreadPtr_t;
    void              Receive_();  /**< Thread for receiving messages. */
    bool              m_bShutdown;
    ThreadPtr_t       m_thread;
    MtlWebSocketPtr_t m_client;
    JsonQueuePtr_t    m_jsonQueuePtr;
};


