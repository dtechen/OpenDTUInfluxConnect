// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <cstdint>

class InfluxHandleClass {
public:
    void init();
    void loop();

private:
    uint32_t _lastPublish = 0;
    bool totalInverterData();
    bool inverterData();
};

extern InfluxHandleClass InfluxHandle;