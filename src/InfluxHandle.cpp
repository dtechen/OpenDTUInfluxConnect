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

        if (inverterData()) {
            InfluxSettings.publish(inverter);
        }
        
        _lastPublish = millis();
    }
}

bool InfluxHandleClass::totalInverterData()
{
    totalInverter.clearFields();
    totalInverter.addField("ac-power", String(Datastore.getTotalAcPowerEnabled(), Datastore.getTotalAcPowerDigits()));
    totalInverter.addField("ac-yieldday", String(Datastore.getTotalAcYieldDayEnabled(), Datastore.getTotalAcYieldDayDigits()));
    totalInverter.addField("dc-power", String(Datastore.getTotalDcPowerEnabled(), Datastore.getTotalDcPowerDigits()));
    totalInverter.addField("dc-irradiation", String(Datastore.getTotalDcIrradiation(), 3));

    return Datastore.getIsAllEnabledReachable();
}

bool InfluxHandleClass::inverterData()
{
    for (uint8_t i = 0; i < Hoymiles.getNumInverters(); i++) {
        auto inv = Hoymiles.getInverterByPos(i);
        inverter.clearFields();
        inverter.clearTags();
        inverter.addTag("name", inv->name());
        inverter.addField("isReachable", inv->isReachable());
        inverter.addField("isProducing", inv->isProducing());

        if (inv->Statistics()->getLastUpdate() > 0) {
            // Loop all channels
            for (auto& t : inv->Statistics()->getChannelTypes()) {
                for (auto& c : inv->Statistics()->getChannelsByType(t)) {
                    // todo
                    ;
                }
            }
        }

        yield();
    }

    return false;     
}