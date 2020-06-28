static const Key keyMap[] = {
    Key::KeyUnknown,    //   0   0x00
    Key::KeyUnknown,    //   1   0x01   VK_LBUTTON          | Left mouse button
    Key::KeyUnknown,    //   2   0x02   VK_RBUTTON          | Right mouse button
    Key::KeyCancel,     //   3   0x03   VK_CANCEL           | Control-Break processing
    Key::KeyUnknown,    //   4   0x04   VK_MBUTTON          | Middle mouse button
    Key::KeyUnknown,    //   5   0x05   VK_XBUTTON1         | X1 mouse button
    Key::KeyUnknown,    //   6   0x06   VK_XBUTTON2         | X2 mouse button
    Key::KeyUnknown,    //   7   0x07   -- unassigned --
    Key::KeyBackspace,  //   8   0x08   VK_BACK             | BackSpace key
    Key::KeyTab,        //   9   0x09   VK_TAB              | Tab key
    Key::KeyUnknown,    //  10   0x0A   -- reserved --
    Key::KeyUnknown,    //  11   0x0B   -- reserved --
    Key::KeyClear,      //  12   0x0C   VK_CLEAR            | Clear key
    Key::KeyReturn,     //  13   0x0D   VK_RETURN           | Enter key
    Key::KeyUnknown,    //  14   0x0E   -- unassigned --
    Key::KeyUnknown,    //  15   0x0F   -- unassigned --
    Key::KeyShift,      //  16   0x10   VK_SHIFT            | Shift key
    Key::KeyControl,    //  17   0x11   VK_CONTROL          | Ctrl key
    Key::KeyAlt,        //  18   0x12   VK_MENU             | Alt key
    Key::KeyPause,      //  19   0x13   VK_PAUSE            | Pause key
    Key::KeyCapsLock,   //  20   0x14   VK_CAPITAL          | Caps-Lock
    Key::KeyUnknown,    //  21   0x15   VK_KANA / VK_HANGUL | IME Kana or Hangul mode
    Key::KeyUnknown,    //  22   0x16   -- unassigned --
    Key::KeyUnknown,    //  23   0x17   VK_JUNJA            | IME Junja mode
    Key::KeyUnknown,    //  24   0x18   VK_FINAL            | IME final mode
    Key::KeyUnknown,    //  25   0x19   VK_HANJA / VK_KANJI | IME Hanja or Kanji mode
    Key::KeyUnknown,    //  26   0x1A   -- unassigned --
    Key::KeyEscape,     //  27   0x1B   VK_ESCAPE           | Esc key
    Key::KeyUnknown,    //  28   0x1C   VK_CONVERT          | IME convert
    Key::KeyUnknown,    //  29   0x1D   VK_NONCONVERT       | IME non-convert
    Key::KeyUnknown,    //  30   0x1E   VK_ACCEPT           | IME accept
    Key::KeyModeSwitch, //  31   0x1F   VK_MODECHANGE       | IME mode change request
    Key::KeySpace,      //  32   0x20   VK_SPACE            | Spacebar
    Key::KeyPageUp,     //  33   0x21   VK_PRIOR            | Page Up key
    Key::KeyPageDown,   //  34   0x22   VK_NEXT             | Page Down key
    Key::KeyEnd,        //  35   0x23   VK_END              | End key
    Key::KeyHome,       //  36   0x24   VK_HOME             | Home key
    Key::KeyLeft,       //  37   0x25   VK_LEFT             | Left arrow key
    Key::KeyUp,         //  38   0x26   VK_UP               | Up arrow key
    Key::KeyRight,      //  39   0x27   VK_RIGHT            | Right arrow key
    Key::KeyDown,       //  40   0x28   VK_DOWN             | Down arrow key
    Key::KeySelect,     //  41   0x29   VK_SELECT           | Select key
    Key::KeyPrinter,    //  42   0x2A   VK_PRINT            | Print key
    Key::KeyExecute,    //  43   0x2B   VK_EXECUTE          | Execute key
    Key::KeyPrint,      //  44   0x2C   VK_SNAPSHOT         | Print Screen key
    Key::KeyInsert,     //  45   0x2D   VK_INSERT           | Ins key
    Key::KeyDelete,     //  46   0x2E   VK_DELETE           | Del key
    Key::KeyHelp,       //  47   0x2F   VK_HELP             | Help key
    Key::Key0,          //  48   0x30   (VK_0)              | 0 key
    Key::Key1,          //  49   0x31   (VK_1)              | 1 key
    Key::Key2,          //  50   0x32   (VK_2)              | 2 key
    Key::Key3,          //  51   0x33   (VK_3)              | 3 key
    Key::Key4,          //  52   0x34   (VK_4)              | 4 key
    Key::Key5,          //  53   0x35   (VK_5)              | 5 key
    Key::Key6,          //  54   0x36   (VK_6)              | 6 key
    Key::Key7,          //  55   0x37   (VK_7)              | 7 key
    Key::Key8,          //  56   0x38   (VK_8)              | 8 key
    Key::Key9,          //  57   0x39   (VK_9)              | 9 key
    Key::KeyUnknown,    //  58   0x3A   -- unassigned --
    Key::KeyUnknown,    //  59   0x3B   -- unassigned --
    Key::KeyUnknown,    //  60   0x3C   -- unassigned --
    Key::KeyUnknown,    //  61   0x3D   -- unassigned --
    Key::KeyUnknown,    //  62   0x3E   -- unassigned --
    Key::KeyUnknown,    //  63   0x3F   -- unassigned --
    Key::KeyUnknown,    //  64   0x40   -- unassigned --
    Key::KeyA,          //  65   0x41   (VK_A)              | A key
    Key::KeyB,          //  66   0x42   (VK_B)              | B key
    Key::KeyC,          //  67   0x43   (VK_C)              | C key
    Key::KeyD,          //  68   0x44   (VK_D)              | D key
    Key::KeyE,          //  69   0x45   (VK_E)              | E key
    Key::KeyF,          //  70   0x46   (VK_F)              | F key
    Key::KeyG,          //  71   0x47   (VK_G)              | G key
    Key::KeyH,          //  72   0x48   (VK_H)              | H key
    Key::KeyI,          //  73   0x49   (VK_I)              | I key
    Key::KeyJ,          //  74   0x4A   (VK_J)              | J key
    Key::KeyK,          //  75   0x4B   (VK_K)              | K key
    Key::KeyL,          //  76   0x4C   (VK_L)              | L key
    Key::KeyM,          //  77   0x4D   (VK_M)              | M key
    Key::KeyN,          //  78   0x4E   (VK_N)              | N key
    Key::KeyO,          //  79   0x4F   (VK_O)              | O key
    Key::KeyP,          //  80   0x50   (VK_P)              | P key
    Key::KeyQ,          //  81   0x51   (VK_Q)              | Q key
    Key::KeyR,          //  82   0x52   (VK_R)              | R key
    Key::KeyS,          //  83   0x53   (VK_S)              | S key
    Key::KeyT,          //  84   0x54   (VK_T)              | T key
    Key::KeyU,          //  85   0x55   (VK_U)              | U key
    Key::KeyV,          //  86   0x56   (VK_V)              | V key
    Key::KeyW,          //  87   0x57   (VK_W)              | W key
    Key::KeyX,          //  88   0x58   (VK_X)              | X key
    Key::KeyY,          //  89   0x59   (VK_Y)              | Y key
    Key::KeyZ,          //  90   0x5A   (VK_Z)              | Z key
    Key::KeyMeta,       //  91   0x5B   VK_LWIN             | Left Windows  - MS Natural kbd
    Key::KeyMeta,       //  92   0x5C   VK_RWIN             | Right Windows - MS Natural kbd
    Key::KeyMenu,       //  93   0x5D   VK_APPS             | Application key-MS Natural kbd
    Key::KeyUnknown,    //  94   0x5E   -- reserved --
    Key::KeySleep,      //  95   0x5F   VK_SLEEP
    Key::KeyNum0,       //  96   0x60   VK_NUMPAD0          | Numeric keypad 0 key
    Key::KeyNum1,       //  97   0x61   VK_NUMPAD1          | Numeric keypad 1 key
    Key::KeyNum2,       //  98   0x62   VK_NUMPAD2          | Numeric keypad 2 key
    Key::KeyNum3,       //  99   0x63   VK_NUMPAD3          | Numeric keypad 3 key
    Key::KeyNum4,       // 100   0x64   VK_NUMPAD4          | Numeric keypad 4 key
    Key::KeyNum5,       // 101   0x65   VK_NUMPAD5          | Numeric keypad 5 key
    Key::KeyNum6,       // 102   0x66   VK_NUMPAD6          | Numeric keypad 6 key
    Key::KeyNum7,       // 103   0x67   VK_NUMPAD7          | Numeric keypad 7 key
    Key::KeyNum8,       // 104   0x68   VK_NUMPAD8          | Numeric keypad 8 key
    Key::KeyNum9,       // 105   0x69   VK_NUMPAD9          | Numeric keypad 9 key
    Key::KeyAsterisk,   // 106   0x6A   VK_MULTIPLY         | Multiply key
    Key::KeyPlus,       // 107   0x6B   VK_ADD              | Add key
    Key::KeyUnknown,    // 108   0x6C   VK_SEPARATOR        | Separator key (locale-dependent)
    Key::KeyMinus,      // 109   0x6D   VK_SUBTRACT         | Subtract key
    Key::KeyUnknown,    // 110   0x6E   VK_DECIMAL          | Decimal key (locale-dependent)
    Key::KeySlash,      // 111   0x6F   VK_DIVIDE           | Divide key
    Key::KeyF1,         // 112   0x70   VK_F1               | F1 key
    Key::KeyF2,         // 113   0x71   VK_F2               | F2 key
    Key::KeyF3,         // 114   0x72   VK_F3               | F3 key
    Key::KeyF4,         // 115   0x73   VK_F4               | F4 key
    Key::KeyF5,         // 116   0x74   VK_F5               | F5 key
    Key::KeyF6,         // 117   0x75   VK_F6               | F6 key
    Key::KeyF7,         // 118   0x76   VK_F7               | F7 key
    Key::KeyF8,         // 119   0x77   VK_F8               | F8 key
    Key::KeyF9,         // 120   0x78   VK_F9               | F9 key
    Key::KeyF10,        // 121   0x79   VK_F10              | F10 key
    Key::KeyF11,        // 122   0x7A   VK_F11              | F11 key
    Key::KeyF12,        // 123   0x7B   VK_F12              | F12 key
    Key::KeyF13,        // 124   0x7C   VK_F13              | F13 key
    Key::KeyF14,        // 125   0x7D   VK_F14              | F14 key
    Key::KeyF15,        // 126   0x7E   VK_F15              | F15 key
    Key::KeyF16,        // 127   0x7F   VK_F16              | F16 key
    Key::KeyF17,        // 128   0x80   VK_F17              | F17 key
    Key::KeyF18,        // 129   0x81   VK_F18              | F18 key
    Key::KeyF19,        // 130   0x82   VK_F19              | F19 key
    Key::KeyF20,        // 131   0x83   VK_F20              | F20 key
    Key::KeyF21,        // 132   0x84   VK_F21              | F21 key
    Key::KeyF22,        // 133   0x85   VK_F22              | F22 key
    Key::KeyF23,        // 134   0x86   VK_F23              | F23 key
    Key::KeyF24,        // 135   0x87   VK_F24              | F24 key
    Key::KeyUnknown,    // 136   0x88   -- unassigned --
    Key::KeyUnknown,    // 137   0x89   -- unassigned --
    Key::KeyUnknown,    // 138   0x8A   -- unassigned --
    Key::KeyUnknown,    // 139   0x8B   -- unassigned --
    Key::KeyUnknown,    // 140   0x8C   -- unassigned --
    Key::KeyUnknown,    // 141   0x8D   -- unassigned --
    Key::KeyUnknown,    // 142   0x8E   -- unassigned --
    Key::KeyUnknown,    // 143   0x8F   -- unassigned --
    Key::KeyNumLock,    // 144   0x90   VK_NUMLOCK          | Num Lock key
    Key::KeyScrollLock, // 145   0x91   VK_SCROLL           | Scroll Lock key
    Key::KeyUnknown,    // 146   0x92   VK_OEM_FJ_JISHO     | 'Dictionary' key /
                          //              VK_OEM_NEC_EQUAL  = key on numpad on NEC PC-9800 kbd
    Key::KeyUnknown,    // 147   0x93   VK_OEM_FJ_MASSHOU   | 'Unregister word' key
    Key::KeyUnknown,    // 148   0x94   VK_OEM_FJ_TOUROKU   | 'Register word' key
    Key::KeyUnknown,    // 149   0x95   VK_OEM_FJ_LOYA  | 'Left OYAYUBI' key
    Key::KeyUnknown,    // 150   0x96   VK_OEM_FJ_ROYA  | 'Right OYAYUBI' key
    Key::KeyUnknown,    // 151   0x97   -- unassigned --
    Key::KeyUnknown,    // 152   0x98   -- unassigned --
    Key::KeyUnknown,    // 153   0x99   -- unassigned --
    Key::KeyUnknown,    // 154   0x9A   -- unassigned --
    Key::KeyUnknown,    // 155   0x9B   -- unassigned --
    Key::KeyUnknown,    // 156   0x9C   -- unassigned --
    Key::KeyUnknown,    // 157   0x9D   -- unassigned --
    Key::KeyUnknown,    // 158   0x9E   -- unassigned --
    Key::KeyUnknown,    // 159   0x9F   -- unassigned --
    Key::KeyShift,      // 160   0xA0   VK_LSHIFT           | Left Shift key
    Key::KeyShift,      // 161   0xA1   VK_RSHIFT           | Right Shift key
    Key::KeyControl,    // 162   0xA2   VK_LCONTROL         | Left Ctrl key
    Key::KeyControl,    // 163   0xA3   VK_RCONTROL         | Right Ctrl key
    Key::KeyAlt,        // 164   0xA4   VK_LMENU            | Left Menu key
    Key::KeyAlt,        // 165   0xA5   VK_RMENU            | Right Menu key
    Key::KeyBack,       // 166   0xA6   VK_BROWSER_BACK     | Browser Back key
    Key::KeyForward,    // 167   0xA7   VK_BROWSER_FORWARD  | Browser Forward key
    Key::KeyRefresh,    // 168   0xA8   VK_BROWSER_REFRESH  | Browser Refresh key
    Key::KeyStop,       // 169   0xA9   VK_BROWSER_STOP     | Browser Stop key
    Key::KeySearch,     // 170   0xAA   VK_BROWSER_SEARCH   | Browser Search key
    Key::KeyFavorites,  // 171   0xAB   VK_BROWSER_FAVORITES| Browser Favorites key
    Key::KeyHomePage,   // 172   0xAC   VK_BROWSER_HOME     | Browser Start and Home key
    Key::KeyVolumeMute, // 173   0xAD   VK_VOLUME_MUTE      | Volume Mute key
    Key::KeyVolumeDown, // 174   0xAE   VK_VOLUME_DOWN      | Volume Down key
    Key::KeyVolumeUp,   // 175   0xAF   VK_VOLUME_UP        | Volume Up key
    Key::KeyMediaNext,  // 176   0xB0   VK_MEDIA_NEXT_TRACK | Next Track key
    Key::KeyMediaPrevious, //177 0xB1   VK_MEDIA_PREV_TRACK | Previous Track key
    Key::KeyMediaStop,  // 178   0xB2   VK_MEDIA_STOP       | Stop Media key
    Key::KeyMediaPlay,  // 179   0xB3   VK_MEDIA_PLAY_PAUSE | Play/Pause Media key
    Key::KeyLaunchMail, // 180   0xB4   VK_LAUNCH_MAIL      | Start Mail key
    Key::KeyLaunchMedia,// 181   0xB5   VK_LAUNCH_MEDIA_SELECT Select Media key
    Key::KeyLaunch0,    // 182   0xB6   VK_LAUNCH_APP1      | Start Application 1 key
    Key::KeyLaunch1,    // 183   0xB7   VK_LAUNCH_APP2      | Start Application 2 key
    Key::KeyUnknown,    // 184   0xB8   -- reserved --
    Key::KeyUnknown,    // 185   0xB9   -- reserved --
    Key::KeyUnknown,    // 186   0xBA   VK_OEM_1            | ';:' for US
    Key::KeyUnknown,    // 187   0xBB   VK_OEM_PLUS         | '+' any country
    Key::KeyUnknown,    // 188   0xBC   VK_OEM_COMMA        | ',' any country
    Key::KeyUnknown,    // 189   0xBD   VK_OEM_MINUS        | '-' any country
    Key::KeyUnknown,    // 190   0xBE   VK_OEM_PERIOD       | '.' any country
    Key::KeyUnknown,    // 191   0xBF   VK_OEM_2            | '/?' for US
    Key::KeyUnknown,    // 192   0xC0   VK_OEM_3            | '`~' for US
    Key::KeyUnknown,    // 193   0xC1   -- reserved --
    Key::KeyUnknown,    // 194   0xC2   -- reserved --
    Key::KeyUnknown,    // 195   0xC3   -- reserved --
    Key::KeyUnknown,    // 196   0xC4   -- reserved --
    Key::KeyUnknown,    // 197   0xC5   -- reserved --
    Key::KeyUnknown,    // 198   0xC6   -- reserved --
    Key::KeyUnknown,    // 199   0xC7   -- reserved --
    Key::KeyUnknown,    // 200   0xC8   -- reserved --
    Key::KeyUnknown,    // 201   0xC9   -- reserved --
    Key::KeyUnknown,    // 202   0xCA   -- reserved --
    Key::KeyUnknown,    // 203   0xCB   -- reserved --
    Key::KeyUnknown,    // 204   0xCC   -- reserved --
    Key::KeyUnknown,    // 205   0xCD   -- reserved --
    Key::KeyUnknown,    // 206   0xCE   -- reserved --
    Key::KeyUnknown,    // 207   0xCF   -- reserved --
    Key::KeyUnknown,    // 208   0xD0   -- reserved --
    Key::KeyUnknown,    // 209   0xD1   -- reserved --
    Key::KeyUnknown,    // 210   0xD2   -- reserved --
    Key::KeyUnknown,    // 211   0xD3   -- reserved --
    Key::KeyUnknown,    // 212   0xD4   -- reserved --
    Key::KeyUnknown,    // 213   0xD5   -- reserved --
    Key::KeyUnknown,    // 214   0xD6   -- reserved --
    Key::KeyUnknown,    // 215   0xD7   -- reserved --
    Key::KeyUnknown,    // 216   0xD8   -- unassigned --
    Key::KeyUnknown,    // 217   0xD9   -- unassigned --
    Key::KeyUnknown,    // 218   0xDA   -- unassigned --
    Key::KeyUnknown,    // 219   0xDB   VK_OEM_4            | '[{' for US
    Key::KeyUnknown,    // 220   0xDC   VK_OEM_5            | '\|' for US
    Key::KeyUnknown,    // 221   0xDD   VK_OEM_6            | ']}' for US
    Key::KeyUnknown,    // 222   0xDE   VK_OEM_7            | ''"' for US
    Key::KeyUnknown,    // 223   0xDF   VK_OEM_8
    Key::KeyUnknown,    // 224   0xE0   -- reserved --
    Key::KeyUnknown,    // 225   0xE1   VK_OEM_AX           | 'AX' key on Japanese AX kbd
    Key::KeyUnknown,    // 226   0xE2   VK_OEM_102          | "<>" or "\|" on RT 102-key kbd
    Key::KeyUnknown,    // 227   0xE3   VK_ICO_HELP         | Help key on ICO
    Key::KeyUnknown,    // 228   0xE4   VK_ICO_00           | 00 key on ICO
    Key::KeyUnknown,    // 229   0xE5   VK_PROCESSKEY       | IME Process key
    Key::KeyUnknown,    // 230   0xE6   VK_ICO_CLEAR        |
    Key::KeyUnknown,    // 231   0xE7   VK_PACKET           | Unicode char as keystrokes
    Key::KeyUnknown,    // 232   0xE8   -- unassigned --
    Key::KeyUnknown,    // 233   0xE9   VK_OEM_RESET
    Key::KeyUnknown,    // 234   0xEA   VK_OEM_JUMP
    Key::KeyUnknown,    // 235   0xEB   VK_OEM_PA1
    Key::KeyUnknown,    // 236   0xEC   VK_OEM_PA2
    Key::KeyUnknown,    // 237   0xED   VK_OEM_PA3
    Key::KeyUnknown,    // 238   0xEE   VK_OEM_WSCTRL
    Key::KeyUnknown,    // 239   0xEF   VK_OEM_CUSEL
    Key::KeyUnknown,    // 240   0xF0   VK_OEM_ATTN
    Key::KeyUnknown,    // 241   0xF1   VK_OEM_FINISH
    Key::KeyUnknown,    // 242   0xF2   VK_OEM_COPY
    Key::KeyUnknown,    // 243   0xF3   VK_OEM_AUTO
    Key::KeyUnknown,    // 244   0xF4   VK_OEM_ENLW
    Key::KeyUnknown,    // 245   0xF5   VK_OEM_BACKTAB
    Key::KeyUnknown,    // 246   0xF6   VK_ATTN             | Attn key
    Key::KeyUnknown,    // 247   0xF7   VK_CRSEL            | CrSel key
    Key::KeyUnknown,    // 248   0xF8   VK_EXSEL            | ExSel key
    Key::KeyUnknown,    // 249   0xF9   VK_EREOF            | Erase EOF key
    Key::KeyPlay,       // 250   0xFA   VK_PLAY             | Play key
    Key::KeyZoom,       // 251   0xFB   VK_ZOOM             | Zoom key
    Key::KeyUnknown,    // 252   0xFC   VK_NONAME           | Reserved
    Key::KeyUnknown,    // 253   0xFD   VK_PA1              | PA1 key
    Key::KeyClear,      // 254   0xFE   VK_OEM_CLEAR        | Clear key
    Key::KeyUnknown
};
