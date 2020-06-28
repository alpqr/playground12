#ifndef FRAME_H
#define FRAME_H

struct App;
struct Event;

void recordFrame(App *app);
void handleEvent(const Event &e);

#endif
