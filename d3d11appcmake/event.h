#ifndef EVENT_H
#define EVENT_H

#include "render.h"

enum class EventType {
    MouseLeftDown = 1,
    MouseLeftUp,
    MouseRightDown,
    MouseRightUp,
    MouseMiddleDown,
    MouseMiddleUp,
    MouseMove,
    KeyDown,
    KeyUp,
    Char
};

enum class Key {
    KeyUnknown,
    KeyCancel,
    KeyBackspace,
    KeyTab,
    KeyClear,
    KeyReturn,
    KeyShift,
    KeyControl,
    KeyAlt,
    KeyPause,
    KeyCapsLock,
    KeyEscape,
    KeyModeSwitch,
    KeySpace,
    KeyPageUp,
    KeyPageDown,
    KeyEnd,
    KeyHome,
    KeyLeft,
    KeyUp,
    KeyRight,
    KeyDown,
    KeySelect,
    KeyPrinter,
    KeyExecute,
    KeyPrint,
    KeyInsert,
    KeyDelete,
    KeyHelp,
    Key0,
    Key1,
    Key2,
    Key3,
    Key4,
    Key5,
    Key6,
    Key7,
    Key8,
    Key9,
    KeyA,
    KeyB,
    KeyC,
    KeyD,
    KeyE,
    KeyF,
    KeyG,
    KeyH,
    KeyI,
    KeyJ,
    KeyK,
    KeyL,
    KeyM,
    KeyN,
    KeyO,
    KeyP,
    KeyQ,
    KeyR,
    KeyS,
    KeyT,
    KeyU,
    KeyV,
    KeyW,
    KeyX,
    KeyY,
    KeyZ,
    KeyMeta,
    KeyMenu,
    KeySleep,
    KeyNum0,
    KeyNum1,
    KeyNum2,
    KeyNum3,
    KeyNum4,
    KeyNum5,
    KeyNum6,
    KeyNum7,
    KeyNum8,
    KeyNum9,
    KeyAsterisk,
    KeyPlus,
    KeyMinus,
    KeySlash,
    KeyF1,
    KeyF2,
    KeyF3,
    KeyF4,
    KeyF5,
    KeyF6,
    KeyF7,
    KeyF8,
    KeyF9,
    KeyF10,
    KeyF11,
    KeyF12,
    KeyF13,
    KeyF14,
    KeyF15,
    KeyF16,
    KeyF17,
    KeyF18,
    KeyF19,
    KeyF20,
    KeyF21,
    KeyF22,
    KeyF23,
    KeyF24,
    KeyNumLock,
    KeyScrollLock,
    KeyBack,
    KeyForward,
    KeyRefresh,
    KeyStop,
    KeySearch,
    KeyFavorites,
    KeyHomePage,
    KeyVolumeMute,
    KeyVolumeDown,
    KeyVolumeUp,
    KeyMediaNext,
    KeyMediaPrevious,
    KeyMediaStop,
    KeyMediaPlay,
    KeyLaunchMail,
    KeyLaunchMedia,
    KeyLaunch0,
    KeyLaunch1,
    KeyPlay,
    KeyZoom
};

struct Event
{
    enum ModifierFlag {
        Shift = 0x01,
        Ctrl = 0x02,
        Alt = 0x04,
        Super = 0x08
    };

    EventType type;
    int x;
    int y;
    Key key;
    uint32_t ch;
    int modifiers;
    bool repeat;

    static Event mouseEvent(EventType type, LPARAM lParam);
    static Event keyEvent(EventType type, WPARAM wParam, LPARAM lParam);
    static Event charEvent(WPARAM wParam, LPARAM lParam);

    static int currentModifiers();
};

#endif
