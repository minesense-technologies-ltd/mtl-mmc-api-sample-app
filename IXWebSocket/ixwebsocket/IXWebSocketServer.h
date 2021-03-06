/*
 *  IXWebSocketServer.h
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2018 Machine Zone, Inc. All rights reserved.
 */

#pragma once

#include "IXSocketServer.h"
#include "IXWebSocket.h"
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <utility> // pair

namespace ix
{
    class WebSocketServer final : public SocketServer
    {
    public:
        using OnConnectionCallback =
            std::function<void(std::shared_ptr<WebSocket>, std::shared_ptr<ConnectionState>)>;

        WebSocketServer(int port = SocketServer::kDefaultPort,
                        const std::string& host = SocketServer::kDefaultHost,
                        int backlog = SocketServer::kDefaultTcpBacklog,
                        size_t maxConnections = SocketServer::kDefaultMaxConnections,
                        int handshakeTimeoutSecs = WebSocketServer::kDefaultHandShakeTimeoutSecs);
        virtual ~WebSocketServer();
        virtual void stop() final;

        void enablePong();
        void disablePong();

        void setOnConnectionCallback(const OnConnectionCallback& callback);

        // Get all the connected clients
        std::set<std::shared_ptr<WebSocket>> getClients();

    private:
        // Member variables
        int _handshakeTimeoutSecs;
        bool _enablePong;

        OnConnectionCallback _onConnectionCallback;

        std::mutex _clientsMutex;
        std::set<std::shared_ptr<WebSocket>> _clients;

        const static int kDefaultHandShakeTimeoutSecs;
        const static bool kDefaultEnablePong;

        // Methods
        virtual void handleConnection(int fd,
                                      std::shared_ptr<ConnectionState> connectionState) final;
        virtual size_t getConnectedClientsCount() final;
    };
} // namespace ix
