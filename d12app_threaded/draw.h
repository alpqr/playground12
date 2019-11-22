#ifndef DRAW_H
#define DRAW_H

#include "builder.h"

void registerDrawCmdListBuilders();

struct BldDefaultRtInit : public Builder
{
    BldDefaultRtInit() : Builder(g_app) { }
    void processEvent(Event e) override;
};

#endif
