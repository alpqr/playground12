#ifndef DRAW_H
#define DRAW_H

#include "builder.h"

struct BldRes0 : public ResourceBuilder
{
    BldRes0() : ResourceBuilder(Type::NoCommandList) { }
    void buildResources() override;
    void releaseResources() override;
};

struct BldRes1 : public ResourceBuilder
{
    BldRes1() : ResourceBuilder(Type::NoCommandList) { }
    void buildResources() override;
    void releaseResources() override;
};

struct BldDefaultRt : public Builder
{
    BldDefaultRt() : Builder(Type::GraphicsCommandList) { }
    void processEvent(Event e) override;
};

struct BuilderHost
{
    BuilderHost();
    const BuilderTable *frame();
    void releaseResourcesNotify();

    bool m_resourcesBuilt = false;
    BuilderList m_res;
    BuilderTable m_bldTab;
    struct {
        BldRes0 *res0;
        BldRes1 *res1;
        BldDefaultRt *defaultRt;
    } m_builders;
};

#endif
