// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022 Thomas Basler and others
 */
#include "InfluxSettings.h"
#include "Configuration.h"
#include "MessageOutput.h"
#include <InfluxDbCloud.h>

InfluxSettingsClass::InfluxSettingsClass()
{
}

void InfluxSettingsClass::performConnect()
{
    if (NetworkSettings.isConnected() && Configuration.get().Influx_Enabled) {
        using std::placeholders::_1;
        using std::placeholders::_2;
        using std::placeholders::_3;
        using std::placeholders::_4;
        using std::placeholders::_5;
        using std::placeholders::_6;

        std::lock_guard<std::mutex> lock(_clientLock);
        if (influxClient != nullptr) {
            MessageOutput.println("Disconnecting from Influx before connecting again...");
            delete influxClient;
            influxClient = nullptr;
        }

        MessageOutput.println("Connecting to Influx...");
        const CONFIG_T& config = Configuration.get();
        const String influxDbUrl = config.Influx_Hostname;
        const String influxDbOrg = config.Influx_Org;
        const String influxDbBucket = config.Influx_Bucket;
        const String influxDbToken = config.Influx_Token;
        MessageOutput.print("Using hostname " + influxDbUrl);
        MessageOutput.print("Using Org " + influxDbOrg);
        MessageOutput.print("Using bucket " + influxDbBucket);
        MessageOutput.println("Using token " + influxDbToken);
        influxClient = new InfluxDBClient(influxDbUrl, influxDbOrg, influxDbBucket, influxDbToken, InfluxDbCloud2CACert);

        checkConnection();
    }
}

void InfluxSettingsClass::performDisconnect()
{
    MessageOutput.println("Disconnecting from Influx...");
    std::lock_guard<std::mutex> lock(_clientLock);
    if (influxClient != nullptr) {
        delete influxClient;
        influxClient = nullptr;
    }
}

bool InfluxSettingsClass::checkConnection()
{
    // Check server connection
    if (influxClient->validateConnection()) {
        return true;
    } else {
        MessageOutput.print("Influx connection failed: ");
        MessageOutput.println(influxClient->getLastErrorMessage());
        return false;
    }
}


void InfluxSettingsClass::publish(Point measurement)
{
    std::lock_guard<std::mutex> lock(_clientLock);
    if (influxClient == nullptr) {
        return;
    }

    if (checkConnection()) {
        if (!influxClient->writePoint(measurement)) {
            MessageOutput.print("Writing to Influx failed: ");
            MessageOutput.println(influxClient->getLastErrorMessage());
        }
    }
}

void InfluxSettingsClass::init()
{
    if (NetworkSettings.isConnected() && Configuration.get().Influx_Enabled) {
        performConnect();
    } else {
        performDisconnect();
    }
}

InfluxSettingsClass InfluxSettings;