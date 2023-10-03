// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022 Thomas Basler and others
 */
#include "WebApi_influx.h"
#include "Configuration.h"
#include "WebApi.h"
#include "WebApi_errors.h"
#include "helper.h"
#include <AsyncJson.h>

void WebApiInfluxClass::init(AsyncWebServer* server)
{
    using std::placeholders::_1;

    _server = server;

    _server->on("/api/influx/status", HTTP_GET, std::bind(&WebApiInfluxClass::onInfluxStatus, this, _1));
    _server->on("/api/influx/config", HTTP_GET, std::bind(&WebApiInfluxClass::onInfluxAdminGet, this, _1));
    _server->on("/api/influx/config", HTTP_POST, std::bind(&WebApiInfluxClass::onInfluxAdminPost, this, _1));
}

void WebApiInfluxClass::loop()
{
}

void WebApiInfluxClass::onInfluxStatus(AsyncWebServerRequest* request)
{
    //if (!WebApi.checkCredentialsReadonly(request)) {
    //    return;
    //}

    AsyncJsonResponse* response = new AsyncJsonResponse(false, INFLUX_JSON_DOC_SIZE);
    JsonObject root = response->getRoot();
    const CONFIG_T& config = Configuration.get();

    root["influx_enabled"] = "true";
    root["influx_hostname"] = "some test";
    root["influx_port"] = 2134;
    root["influx_token"] = "some token";
    root["influx_org"] = "some org";
    root["influx_bucket"] = "some bucket";
    root["influx_publish_interval"] = 5;

    response->setLength();
    request->send(response);
}

void WebApiInfluxClass::onInfluxAdminGet(AsyncWebServerRequest* request)
{
    //if (!WebApi.checkCredentials(request)) {
    //    return;
    //}

    AsyncJsonResponse* response = new AsyncJsonResponse(false, INFLUX_JSON_DOC_SIZE);
    JsonObject root = response->getRoot();
    const CONFIG_T& config = Configuration.get();

    root["influx_enabled"] = "true";
    root["influx_hostname"] = "some test";
    root["influx_port"] = 2134;
    root["influx_token"] = "some token";
    root["influx_org"] = "some org";
    root["influx_bucket"] = "some bucket";
    root["influx_publish_interval"] = 5;

    response->setLength();
    request->send(response);
}

void WebApiInfluxClass::onInfluxAdminPost(AsyncWebServerRequest* request)
{
    if (!WebApi.checkCredentials(request)) {
        return;
    }
}
