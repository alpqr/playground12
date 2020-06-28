#ifndef APP_H
#define APP_H

#include "event.h"

struct App
{
    Device dev;
    Swapchain swapchain;
    bool renderable;
    bool wantsRender;

    void requestRender() { wantsRender = true; }
    void maybeRender() {
        if (wantsRender) {
            wantsRender = false;
            render();
        }
    }
    void render();

    void event(const Event &e);
};

#endif
