// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <Hoymiles.h>
#include <cstdint>

class InfluxHandleClass {
public:
    void init();
    void loop();

private:
    uint32_t _lastPublish = 0;
    FieldId_t _publishFields[14] = {
        FLD_UDC,
        FLD_IDC,
        FLD_PDC,
        FLD_YD,
        FLD_YT,
        FLD_UAC,
        FLD_IAC,
        FLD_PAC,
        FLD_F,
        FLD_T,
        FLD_PF,
        FLD_EFF,
        FLD_IRR,
        FLD_Q
    };
    bool totalInverterData();
    void handleInverters();
    void handleInverterChannel(String name, StatisticsParser* stats, ChannelType_t type, ChannelNum_t channel);
};

extern InfluxHandleClass InfluxHandle;