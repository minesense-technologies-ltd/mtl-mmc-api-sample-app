/*
 *  IXSocketTest.cpp
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2019 Machine Zone. All rights reserved.
 */

#include <iostream>
#include <ixwebsocket/IXHttpClient.h>
#include <ixwebsocket/IXHttpServer.h>
#include "IXGetFreePort.h"

#include "catch.hpp"

using namespace ix;

TEST_CASE("http server", "[httpd]")
{
    SECTION("Connect to a local HTTP server")
    {
        int port = getFreePort();
        ix::HttpServer server(port, "127.0.0.1");

        auto res = server.listen();
        REQUIRE(res.first);
        server.start();

        HttpClient httpClient;
        WebSocketHttpHeaders headers;

        std::string url("http://127.0.0.1:");
        url += std::to_string(port);
        url += "/data/foo.txt";
        auto args = httpClient.createRequest(url);

        args->extraHeaders = headers;
        args->connectTimeout = 60;
        args->transferTimeout = 60;
        args->followRedirects = true;
        args->maxRedirects = 10;
        args->verbose = true;
        args->compress = true;
        args->logger = [](const std::string& msg)
        {
            std::cout << msg;
        };
        args->onProgressCallback = [](int current, int total) -> bool
        {
            std::cerr << "\r" << "Downloaded "
                      << current << " bytes out of " << total;
            return true;
        };

        auto response = httpClient.get(url, args);

        for (auto it : response->headers)
        {
            std::cerr << it.first << ": " << it.second << std::endl;
        }

        std::cerr << "Upload size: " << response->uploadSize << std::endl;
        std::cerr << "Download size: " << response->downloadSize << std::endl;
        std::cerr << "Status: " << response->statusCode << std::endl;
        std::cerr << "Error message: " << response->errorMsg << std::endl;

        REQUIRE(response->errorCode == HttpErrorCode::Ok);
        REQUIRE(response->statusCode == 200);

        server.stop();
    }
}
