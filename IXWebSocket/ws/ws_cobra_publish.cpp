/*
 *  ws_cobra_publish.cpp
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2019 Machine Zone, Inc. All rights reserved.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <atomic>
#include <jsoncpp/json/json.h>
#include <ixcobra/IXCobraMetricsPublisher.h>
#include <spdlog/spdlog.h>

namespace ix
{
    int ws_cobra_publish_main(const std::string& appkey,
                              const std::string& endpoint,
                              const std::string& rolename,
                              const std::string& rolesecret,
                              const std::string& channel,
                              const std::string& path,
                              bool stress)
    {
        std::atomic<int> sentMessages(0);
        std::atomic<int> ackedMessages(0);
        CobraConnection::setPublishTrackerCallback(
            [&sentMessages, &ackedMessages](bool sent, bool acked)
            {
                if (sent) sentMessages++;
                if (acked) ackedMessages++;
            }
        );

        CobraMetricsPublisher cobraMetricsPublisher;
        cobraMetricsPublisher.enable(true);

        bool enablePerMessageDeflate = true;
        cobraMetricsPublisher.configure(appkey, endpoint, channel,
                                        rolename, rolesecret, enablePerMessageDeflate);

        while (!cobraMetricsPublisher.isAuthenticated()) ;

        std::ifstream f(path);
        std::string str((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());

        Json::Value data;
        Json::Reader reader;
        if (!reader.parse(str, data)) return 1;

        if (!stress)
        {
            cobraMetricsPublisher.push(channel, data);
        }
        else
        {
            // Stress mode to try to trigger server and client bugs
            while (true)
            {
                for (int i = 0 ; i < 1000; ++i)
                {
                    cobraMetricsPublisher.push(channel, data);
                }

                cobraMetricsPublisher.suspend();
                cobraMetricsPublisher.resume();

                // FIXME: investigate why without this check we trigger a lock
                while (!cobraMetricsPublisher.isAuthenticated()) ;
            }
        }

        // Wait a bit for the message to get a chance to be sent
        // there isn't any ack on publish right now so it's the best we can do
        // FIXME: this comment is a lie now
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        spdlog::info("Sent messages: {} Acked messages {}", sentMessages, ackedMessages);

        return 0;
    }
}
