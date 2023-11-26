// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022 Thomas Basler and others
 */
#include "InfluxHandle.h"
#include "Configuration.h"
#include "Datastore.h"
#include "InfluxSettings.h"
#include "NetworkSettings.h"
#include "MessageOutput.h"
#include <Hoymiles.h>

InfluxHandleClass InfluxHandle;

void InfluxHandleClass::init()
{
}

void InfluxHandleClass::loop()
{
    if (!Hoymiles.isAllRadioIdle()) {
        return;
    }

    //const CONFIG_T& config = Configuration.get();

    if (millis() - _lastPublish > (60000 /*config.Influx_PublishInterval * 1000*/)) {
        Point totalInverter("total");
        totalInverter.addTag("inverter", "total");
        if (totalInverterData(totalInverter)) {
            
            InfluxSettings.publish(totalInverter);
            // handleInverters();
        }
        
        Point memory("mem");
        memory.addField("emptyHeap", ESP.getFreeHeap());
        memory.addField("usedHeap", ESP.getHeapSize() - ESP.getFreeHeap());
        InfluxSettings.publish(memory);
        
        _lastPublish = millis();
    }
}

bool InfluxHandleClass::totalInverterData(Point totalInverter)
{
    totalInverter.addField("AcPower", Datastore.getTotalAcPowerEnabled());
    totalInverter.addField("AcYieldDay", Datastore.getTotalAcYieldDayEnabled());
    totalInverter.addField("AcYieldTotal", Datastore.getTotalAcYieldTotalEnabled());
    totalInverter.addField("DcPower", Datastore.getTotalDcPowerEnabled());
    totalInverter.addField("DcIrradiation", Datastore.getTotalDcIrradiation());

    return Datastore.getIsAtLeastOneReachable();
}

void InfluxHandleClass::handleInverters()
{
    Point inverter("inverter");
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
                    handleInverterChannel(inverter, name, parser, type, channel);
                }
            }
        }

        yield();
    }
}

void InfluxHandleClass::handleInverterChannel(Point inverter, String name, StatisticsParser* stats, ChannelType_t type, ChannelNum_t channel) {
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