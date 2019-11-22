#ifndef DRAW_H
#define DRAW_H

#include "builder.h"

struct BldDefaultRtInit : public Builder
{
    void processEvent(Event e) override;
};

struct BuilderHost
{
    BuilderHost();
    const BuilderTable *frameFunc();

    BuilderTable bldTab;
    BldDefaultRtInit *defaultRtInit;
};

#endif
