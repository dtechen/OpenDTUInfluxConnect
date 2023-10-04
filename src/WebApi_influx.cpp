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

    root["influx_enabled"] = config.Influx_Enabled;
    root["influx_hostname"] = config.Influx_Hostname;
    root["influx_port"] = config.Influx_Port;
    root["influx_org"] = config.Influx_Org;
    root["influx_bucket"] = config.Influx_Bucket;
    root["influx_publish_interval"] = config.Influx_PublishInterval;

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

    root["influx_enabled"] = config.Influx_Enabled;
    root["influx_hostname"] = config.Influx_Hostname;
    root["influx_port"] = config.Influx_Port;
    root["influx_token"] = config.Influx_Token;
    root["influx_org"] = config.Influx_Org;
    root["influx_bucket"] = config.Influx_Bucket;
    root["influx_publish_interval"] = config.Influx_PublishInterval;

    response->setLength();
    request->send(response);
}

void WebApiInfluxClass::onInfluxAdminPost(AsyncWebServerRequest* request)
{
    if (!WebApi.checkCredentials(request)) {
        return;
    }

        AsyncJsonResponse* response = new AsyncJsonResponse(false, INFLUX_JSON_DOC_SIZE);
    JsonObject retMsg = response->getRoot();
    retMsg["type"] = "warning";

    if (!request->hasParam("data", true)) {
        retMsg["message"] = "No values found!";
        retMsg["code"] = WebApiError::GenericNoValueFound;
        response->setLength();
        request->send(response);
        return;
    }

    String json = request->getParam("data", true)->value();

    if (json.length() > INFLUX_JSON_DOC_SIZE) {
        retMsg["message"] = "Data too large!";
        retMsg["code"] = WebApiError::GenericDataTooLarge;
        response->setLength();
        request->send(response);
        return;
    }

    DynamicJsonDocument root(INFLUX_JSON_DOC_SIZE);
    DeserializationError error = deserializeJson(root, json);

    if (error) {
        retMsg["message"] = "Failed to parse data!";
        retMsg["code"] = WebApiError::GenericParseError;
        response->setLength();
        request->send(response);
        return;
    }

    if (!(root.containsKey("influx_enabled")
            && root.containsKey("influx_hostname")
            && root.containsKey("influx_port")
            && root.containsKey("influx_org")
            && root.containsKey("influx_token")
            && root.containsKey("influx_bucket")
            && root.containsKey("influx_publish_interval"))) {
        retMsg["message"] = "Values are missing!";
        retMsg["code"] = WebApiError::GenericValueMissing;
        response->setLength();
        request->send(response);
        return;
    }

    if (root["influx_enabled"].as<bool>()) {
        if (root["influx_hostname"].as<String>().length() == 0 || root["influx_hostname"].as<String>().length() > INFLUX_MAX_HOSTNAME_STRLEN) {
            retMsg["message"] = "Influx Cloud Server must between 1 and " STR(INFLUX_MAX_HOSTNAME_STRLEN) " characters long!";
            retMsg["code"] = WebApiError::InfluxHostnameLength;
            retMsg["param"]["max"] = INFLUX_MAX_HOSTNAME_STRLEN;
            response->setLength();
            request->send(response);
            return;
        }

        if (root["influx_org"].as<String>().length() > INFLUX_MAX_ORG_STRLEN) {
            retMsg["message"] = "Org must not be longer than " STR(INFLUX_MAX_ORG_STRLEN) " characters!";
            retMsg["code"] = WebApiError::InfluxOrgLength;
            retMsg["param"]["max"] = INFLUX_MAX_ORG_STRLEN;
            response->setLength();
            request->send(response);
            return;
        }
        if (root["influx_token"].as<String>().length() > INFLUX_MAX_TOKEN_STRLEN) {
            retMsg["message"] = "Token must not be longer than " STR(INFLUX_MAX_TOKEN_STRLEN) " characters!";
            retMsg["code"] = WebApiError::InfluxTokenLength;
            retMsg["param"]["max"] = INFLUX_MAX_TOKEN_STRLEN;
            response->setLength();
            request->send(response);
            return;
        }
        if (root["influx_bucket"].as<String>().length() > INFLUX_MAX_BUCKET_STRLEN) {
            retMsg["message"] = "Bucket must not be longer than " STR(INFLUX_MAX_BUCKET_STRLEN) " characters!";
            retMsg["code"] = WebApiError::InfluxBucketLength;
            retMsg["param"]["max"] = INFLUX_MAX_BUCKET_STRLEN;
            response->setLength();
            request->send(response);
            return;
        }

        if (root["influx_port"].as<uint>() == 0 || root["influx_port"].as<uint>() > 65535) {
            retMsg["message"] = "Port must be a number between 1 and 65535!";
            retMsg["code"] = WebApiError::InfluxPort;
            response->setLength();
            request->send(response);
            return;
        }

        if (root["influx_publish_interval"].as<uint32_t>() < 5 || root["influx_publish_interval"].as<uint32_t>() > 65535) {
            retMsg["message"] = "Publish interval must be a number between 5 and 65535!";
            retMsg["code"] = WebApiError::InfluxPublishInterval;
            retMsg["param"]["min"] = 5;
            retMsg["param"]["max"] = 65535;
            response->setLength();
            request->send(response);
            return;
        }
    }

    CONFIG_T& config = Configuration.get();
    config.Influx_Enabled = root["influx_enabled"].as<bool>();
    config.Influx_Port = root["influx_port"].as<uint>();
    strlcpy(config.Influx_Hostname, root["influx_hostname"].as<String>().c_str(), sizeof(config.Influx_Hostname));
    strlcpy(config.Influx_Org, root["influx_org"].as<String>().c_str(), sizeof(config.Influx_Org));
    strlcpy(config.Influx_Token, root["influx_token"].as<String>().c_str(), sizeof(config.Influx_Token));
    strlcpy(config.Influx_Bucket, root["influx_bucket"].as<String>().c_str(), sizeof(config.Influx_Bucket));
    config.Influx_PublishInterval = root["influx_publish_interval"].as<uint32_t>();
    Configuration.write();

    retMsg["type"] = "success";
    retMsg["message"] = "Settings saved!";
    retMsg["code"] = WebApiError::GenericSuccess;

    response->setLength();
    request->send(response);
}
