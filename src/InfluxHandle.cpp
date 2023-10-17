// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022 Thomas Basler and others
 */
#include "InfluxHandle.h"
#include "Configuration.h"
#include "Datastore.h"
#include "InfluxSettings.h"
#include "NetworkSettings.h"
#include <Hoymiles.h>

InfluxHandleClass InfluxHandle;

Point totalInverter("total");
Point inverter("inverter");

void InfluxHandleClass::init()
{
    totalInverter.addTag("inverter", "total");
}

void InfluxHandleClass::loop()
{
    if (!Hoymiles.isAllRadioIdle()) {
        return;
    }

    const CONFIG_T& config = Configuration.get();

    if (millis() - _lastPublish > (config.Influx_PublishInterval * 1000)) {
        if (totalInverterData()) {
            InfluxSettings.publish(totalInverter);
        }
        handleInverters();
        
        _lastPublish = millis();
    }
}

bool InfluxHandleClass::totalInverterData()
{
    totalInverter.addField("ac-power", Datastore.getTotalAcPowerEnabled());
    totalInverter.addField("ac-yieldday", Datastore.getTotalAcYieldDayEnabled());
    totalInverter.addField("ac-yieldtotal", Datastore.getTotalAcYieldTotalEnabled());
    totalInverter.addField("dc-power", Datastore.getTotalDcPowerEnabled());
    totalInverter.addField("dc-irradiation", Datastore.getTotalDcIrradiation());

    return Datastore.getIsAtLeastOneReachable();
}

void InfluxHandleClass::handleInverters()
{
    for (uint8_t i = 0; i < Hoymiles.getNumInverters(); i++) {
        auto inv = Hoymiles.getInverterByPos(i);
        inverter.clearFields();
        inverter.addField("isReachable", inv->isReachable());
        inverter.addField("isProducing", inv->isProducing());

        if (inv->Statistics()->getLastUpdate() > 0) {
            // Loop all channels by type
            for (auto& type : inv->Statistics()->getChannelTypes()) {
                for (auto& channel : inv->Statistics()->getChannelsByType(type)) {
                    String name = inv->name();
                    StatisticsParser* parser = inv->Statistics();
                    handleInverterChannel(name, parser, type, channel);
                }
            }
        }

        yield();
    }
}

void InfluxHandleClass::handleInverterChannel(String name, StatisticsParser* stats, ChannelType_t type, ChannelNum_t channel) {
    bool hasData = false;
    inverter.clearTags();
    inverter.addTag("name", name);
    inverter.addTag("type", stats->getChannelTypeName(type));
    for (uint8_t f = 0; f < sizeof(_publishFields) / sizeof(FieldId_t); f++) {
        if (stats->hasChannelFieldValue(type, channel, _publishFields[f])) {
            hasData = true;
            inverter.addField(stats->getChannelFieldName(type, channel, _publishFields[f]),
            stats->getChannelFieldValue(type, channel, _publishFields[f]));
        }
    }
    if (hasData) {
        InfluxSettings.publish(inverter);
    }
}