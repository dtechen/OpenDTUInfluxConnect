// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "NetworkSettings.h"
#include <MqttSubscribeParser.h>
#include <Ticker.h>
#include <InfluxDbClient.h>
#include <mutex>

class InfluxSettingsClass {
public:
    InfluxSettingsClass();
    void init();
    void publish(Point measurement);

private:
    void performConnect();
    void performDisconnect();
    bool checkConnection();
    InfluxDBClient * influxClient = nullptr;
    std::mutex _clientLock;
};

extern InfluxSettingsClass InfluxSettings;