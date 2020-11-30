// Lic:
// TQSE.cpp
// Tricky's Quick SDL Events
// version: 20.11.30
// Copyright (C) 2020 Jeroen P. Broks
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
// EndLic

// C++
#include <iostream>
#include <map>
#include <vector>

// SDL
#include <sdl.h>

// My own header
#include "TQSE.hpp"

#undef TQSE_Key_Init_Verslag


namespace TrickyUnits {    

    static const int maxmousebuttons = 16;

    static bool TQSE_InitDone = false;

    static bool stAppTerminate = false;

    static std::map<int, bool> stKeyDown;
    static std::map<int, bool> stKeyHit;
    static std::map<int, bool> stKeyOldDown;
    static std::vector<int> stAllKeys;

    static bool MsButDown[maxmousebuttons];
    static bool MsButOldDown[maxmousebuttons];
    static bool MsButHit[maxmousebuttons];

    static void InitCheck() {
        if (!TQSE_InitDone) {
            std::cout << "\a\x1b[31mTQSE check done without calling TQSE first!\x1b[0m\n";
            TQSE_Init();
        }

    }

    static void KeyClean(bool full = false) {
        for (auto& i : stAllKeys) {
            if (full) {
                stKeyDown[i] = false;
                stKeyOldDown[i] = false;
            } else stKeyOldDown[i] = stKeyDown[i];
            stKeyHit[i] = false;
#ifdef TQSE_Key_Init_Verslag
            if (full) std::cout << "Initiated Keycode #" << i << " (" << SDL_GetKeyName(i) << ")  Count:" << stKeyDown.size() << "\n";
#endif

        }
    }

    static void MouseClean(bool full=false) {
        for (unsigned int i = 0; i < maxmousebuttons; i++) {
            if (full) MsButOldDown[i] = false; else MsButOldDown[i] = MsButDown[i];
            MsButDown[i] = false;
            MsButHit[i] = false;
        }
    }

    void TQSE_Init() {
        // Since C++ does not have a GOOD way to cover this up (these are the moments I miss C#)
        // All I could do was doing it THIS way:
        stAllKeys.push_back(SDLK_UNKNOWN); // That is 0,

        stAllKeys.push_back(SDLK_RETURN); // That is '\r',
        stAllKeys.push_back(SDLK_ESCAPE); // That is '\033',
        stAllKeys.push_back(SDLK_BACKSPACE); // That is '\b',
        stAllKeys.push_back(SDLK_TAB); // That is '\t',
        stAllKeys.push_back(SDLK_SPACE); // That is ' ',
        stAllKeys.push_back(SDLK_EXCLAIM); // That is '!',
        stAllKeys.push_back(SDLK_QUOTEDBL); // That is '"',
        stAllKeys.push_back(SDLK_HASH); // That is '#',
        stAllKeys.push_back(SDLK_PERCENT); // That is '%',
        stAllKeys.push_back(SDLK_DOLLAR); // That is '$',
        stAllKeys.push_back(SDLK_AMPERSAND); // That is '&',
        stAllKeys.push_back(SDLK_QUOTE); // That is '\'',
        stAllKeys.push_back(SDLK_LEFTPAREN); // That is '(',
        stAllKeys.push_back(SDLK_RIGHTPAREN); // That is ')',
        stAllKeys.push_back(SDLK_ASTERISK); // That is '*',
        stAllKeys.push_back(SDLK_PLUS); // That is '+',
        stAllKeys.push_back(SDLK_COMMA); // That is ',',
        stAllKeys.push_back(SDLK_MINUS); // That is '-',
        stAllKeys.push_back(SDLK_PERIOD); // That is '.',
        stAllKeys.push_back(SDLK_SLASH); // That is '/',
        stAllKeys.push_back(SDLK_0); // That is '0',
        stAllKeys.push_back(SDLK_1); // That is '1',
        stAllKeys.push_back(SDLK_2); // That is '2',
        stAllKeys.push_back(SDLK_3); // That is '3',
        stAllKeys.push_back(SDLK_4); // That is '4',
        stAllKeys.push_back(SDLK_5); // That is '5',
        stAllKeys.push_back(SDLK_6); // That is '6',
        stAllKeys.push_back(SDLK_7); // That is '7',
        stAllKeys.push_back(SDLK_8); // That is '8',
        stAllKeys.push_back(SDLK_9); // That is '9',
        stAllKeys.push_back(SDLK_COLON); // That is ':',
        stAllKeys.push_back(SDLK_SEMICOLON); // That is ';',
        stAllKeys.push_back(SDLK_LESS); // That is '<',
        stAllKeys.push_back(SDLK_EQUALS); // That is '); // That is',
        stAllKeys.push_back(SDLK_GREATER); // That is '>',
        stAllKeys.push_back(SDLK_QUESTION); // That is '?',
        stAllKeys.push_back(SDLK_AT); // That is '@',
        /*
           Skip uppercase letters
         */
        stAllKeys.push_back(SDLK_LEFTBRACKET); // That is '[',
        stAllKeys.push_back(SDLK_BACKSLASH); // That is '\\',
        stAllKeys.push_back(SDLK_RIGHTBRACKET); // That is ']',
        stAllKeys.push_back(SDLK_CARET); // That is '^',
        stAllKeys.push_back(SDLK_UNDERSCORE); // That is '_',
        stAllKeys.push_back(SDLK_BACKQUOTE); // That is '`',
        stAllKeys.push_back(SDLK_a); // That is 'a',
        stAllKeys.push_back(SDLK_b); // That is 'b',
        stAllKeys.push_back(SDLK_c); // That is 'c',
        stAllKeys.push_back(SDLK_d); // That is 'd',
        stAllKeys.push_back(SDLK_e); // That is 'e',
        stAllKeys.push_back(SDLK_f); // That is 'f',
        stAllKeys.push_back(SDLK_g); // That is 'g',
        stAllKeys.push_back(SDLK_h); // That is 'h',
        stAllKeys.push_back(SDLK_i); // That is 'i',
        stAllKeys.push_back(SDLK_j); // That is 'j',
        stAllKeys.push_back(SDLK_k); // That is 'k',
        stAllKeys.push_back(SDLK_l); // That is 'l',
        stAllKeys.push_back(SDLK_m); // That is 'm',
        stAllKeys.push_back(SDLK_n); // That is 'n',
        stAllKeys.push_back(SDLK_o); // That is 'o',
        stAllKeys.push_back(SDLK_p); // That is 'p',
        stAllKeys.push_back(SDLK_q); // That is 'q',
        stAllKeys.push_back(SDLK_r); // That is 'r',
        stAllKeys.push_back(SDLK_s); // That is 's',
        stAllKeys.push_back(SDLK_t); // That is 't',
        stAllKeys.push_back(SDLK_u); // That is 'u',
        stAllKeys.push_back(SDLK_v); // That is 'v',
        stAllKeys.push_back(SDLK_w); // That is 'w',
        stAllKeys.push_back(SDLK_x); // That is 'x',
        stAllKeys.push_back(SDLK_y); // That is 'y',
        stAllKeys.push_back(SDLK_z); // That is 'z',

        stAllKeys.push_back(SDLK_CAPSLOCK); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CAPSLOCK),

        stAllKeys.push_back(SDLK_F1); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F1),
        stAllKeys.push_back(SDLK_F2); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F2),
        stAllKeys.push_back(SDLK_F3); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F3),
        stAllKeys.push_back(SDLK_F4); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F4),
        stAllKeys.push_back(SDLK_F5); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F5),
        stAllKeys.push_back(SDLK_F6); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F6),
        stAllKeys.push_back(SDLK_F7); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F7),
        stAllKeys.push_back(SDLK_F8); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F8),
        stAllKeys.push_back(SDLK_F9); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F9),
        stAllKeys.push_back(SDLK_F10); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F10),
        stAllKeys.push_back(SDLK_F11); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F11),
        stAllKeys.push_back(SDLK_F12); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F12),

        stAllKeys.push_back(SDLK_PRINTSCREEN); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRINTSCREEN),
        stAllKeys.push_back(SDLK_SCROLLLOCK); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SCROLLLOCK),
        stAllKeys.push_back(SDLK_PAUSE); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAUSE),
        stAllKeys.push_back(SDLK_INSERT); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_INSERT),
        stAllKeys.push_back(SDLK_HOME); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HOME),
        stAllKeys.push_back(SDLK_PAGEUP); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEUP),
        stAllKeys.push_back(SDLK_DELETE); // That is '\177',
        stAllKeys.push_back(SDLK_END); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_END),
        stAllKeys.push_back(SDLK_PAGEDOWN); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEDOWN),
        stAllKeys.push_back(SDLK_RIGHT); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RIGHT),
        stAllKeys.push_back(SDLK_LEFT); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LEFT),
        stAllKeys.push_back(SDLK_DOWN); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DOWN),
        stAllKeys.push_back(SDLK_UP); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UP),

        stAllKeys.push_back(SDLK_NUMLOCKCLEAR); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_NUMLOCKCLEAR),
        stAllKeys.push_back(SDLK_KP_DIVIDE); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DIVIDE),
        stAllKeys.push_back(SDLK_KP_MULTIPLY); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MULTIPLY),
        stAllKeys.push_back(SDLK_KP_MINUS); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MINUS),
        stAllKeys.push_back(SDLK_KP_PLUS); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUS),
        stAllKeys.push_back(SDLK_KP_ENTER); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_ENTER),
        stAllKeys.push_back(SDLK_KP_1); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_1),
        stAllKeys.push_back(SDLK_KP_2); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_2),
        stAllKeys.push_back(SDLK_KP_3); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_3),
        stAllKeys.push_back(SDLK_KP_4); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_4),
        stAllKeys.push_back(SDLK_KP_5); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_5),
        stAllKeys.push_back(SDLK_KP_6); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_6),
        stAllKeys.push_back(SDLK_KP_7); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_7),
        stAllKeys.push_back(SDLK_KP_8); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_8),
        stAllKeys.push_back(SDLK_KP_9); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_9),
        stAllKeys.push_back(SDLK_KP_0); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_0),
        stAllKeys.push_back(SDLK_KP_PERIOD); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERIOD),

        stAllKeys.push_back(SDLK_APPLICATION); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_APPLICATION),
        stAllKeys.push_back(SDLK_POWER); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_POWER),
        stAllKeys.push_back(SDLK_KP_EQUALS); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALS),
        stAllKeys.push_back(SDLK_F13); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F13),
        stAllKeys.push_back(SDLK_F14); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F14),
        stAllKeys.push_back(SDLK_F15); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F15),
        stAllKeys.push_back(SDLK_F16); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F16),
        stAllKeys.push_back(SDLK_F17); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F17),
        stAllKeys.push_back(SDLK_F18); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F18),
        stAllKeys.push_back(SDLK_F19); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F19),
        stAllKeys.push_back(SDLK_F20); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F20),
        stAllKeys.push_back(SDLK_F21); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F21),
        stAllKeys.push_back(SDLK_F22); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F22),
        stAllKeys.push_back(SDLK_F23); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F23),
        stAllKeys.push_back(SDLK_F24); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F24),
        stAllKeys.push_back(SDLK_EXECUTE); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXECUTE),
        stAllKeys.push_back(SDLK_HELP); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HELP),
        stAllKeys.push_back(SDLK_MENU); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MENU),
        stAllKeys.push_back(SDLK_SELECT); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SELECT),
        stAllKeys.push_back(SDLK_STOP); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_STOP),
        stAllKeys.push_back(SDLK_AGAIN); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AGAIN),
        stAllKeys.push_back(SDLK_UNDO); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UNDO),
        stAllKeys.push_back(SDLK_CUT); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CUT),
        stAllKeys.push_back(SDLK_COPY); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_COPY),
        stAllKeys.push_back(SDLK_PASTE); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PASTE),
        stAllKeys.push_back(SDLK_FIND); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_FIND),
        stAllKeys.push_back(SDLK_MUTE); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MUTE),
        stAllKeys.push_back(SDLK_VOLUMEUP); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEUP),
        stAllKeys.push_back(SDLK_VOLUMEDOWN); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEDOWN),
        stAllKeys.push_back(SDLK_KP_COMMA); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COMMA),
        stAllKeys.push_back(SDLK_KP_EQUALSAS400); // That is        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALSAS400),

        stAllKeys.push_back(SDLK_ALTERASE); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_ALTERASE),
        stAllKeys.push_back(SDLK_SYSREQ); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SYSREQ),
        stAllKeys.push_back(SDLK_CANCEL); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CANCEL),
        stAllKeys.push_back(SDLK_CLEAR); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEAR),
        stAllKeys.push_back(SDLK_PRIOR); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRIOR),
        stAllKeys.push_back(SDLK_RETURN2); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RETURN2),
        stAllKeys.push_back(SDLK_SEPARATOR); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SEPARATOR),
        stAllKeys.push_back(SDLK_OUT); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OUT),
        stAllKeys.push_back(SDLK_OPER); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OPER),
        stAllKeys.push_back(SDLK_CLEARAGAIN); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEARAGAIN),
        stAllKeys.push_back(SDLK_CRSEL); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CRSEL),
        stAllKeys.push_back(SDLK_EXSEL); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXSEL),

        stAllKeys.push_back(SDLK_KP_00); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_00),
        stAllKeys.push_back(SDLK_KP_000); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_000),
        stAllKeys.push_back(SDLK_THOUSANDSSEPARATOR); // That is        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_THOUSANDSSEPARATOR),        
        stAllKeys.push_back(SDLK_DECIMALSEPARATOR); // That is        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DECIMALSEPARATOR),
        stAllKeys.push_back(SDLK_CURRENCYUNIT); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYUNIT),
        stAllKeys.push_back(SDLK_CURRENCYSUBUNIT); // That is        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYSUBUNIT),
        stAllKeys.push_back(SDLK_KP_LEFTPAREN); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTPAREN),
        stAllKeys.push_back(SDLK_KP_RIGHTPAREN); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTPAREN),
        stAllKeys.push_back(SDLK_KP_LEFTBRACE); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTBRACE),
        stAllKeys.push_back(SDLK_KP_RIGHTBRACE); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTBRACE),
        stAllKeys.push_back(SDLK_KP_TAB); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_TAB),
        stAllKeys.push_back(SDLK_KP_BACKSPACE); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BACKSPACE),
        stAllKeys.push_back(SDLK_KP_A); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_A),
        stAllKeys.push_back(SDLK_KP_B); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_B),
        stAllKeys.push_back(SDLK_KP_C); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_C),
        stAllKeys.push_back(SDLK_KP_D); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_D),
        stAllKeys.push_back(SDLK_KP_E); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_E),
        stAllKeys.push_back(SDLK_KP_F); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_F),
        stAllKeys.push_back(SDLK_KP_XOR); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_XOR),
        stAllKeys.push_back(SDLK_KP_POWER); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_POWER),
        stAllKeys.push_back(SDLK_KP_PERCENT); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERCENT),
        stAllKeys.push_back(SDLK_KP_LESS); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LESS),
        stAllKeys.push_back(SDLK_KP_GREATER); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_GREATER),
        stAllKeys.push_back(SDLK_KP_AMPERSAND); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AMPERSAND),
        stAllKeys.push_back(SDLK_KP_DBLAMPERSAND); // That is        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLAMPERSAND),
        stAllKeys.push_back(SDLK_KP_VERTICALBAR); // That is        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_VERTICALBAR),
        stAllKeys.push_back(SDLK_KP_DBLVERTICALBAR); // That is    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLVERTICALBAR),
        stAllKeys.push_back(SDLK_KP_COLON); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COLON),
        stAllKeys.push_back(SDLK_KP_HASH); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HASH),
        stAllKeys.push_back(SDLK_KP_SPACE); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_SPACE),
        stAllKeys.push_back(SDLK_KP_AT); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AT),
        stAllKeys.push_back(SDLK_KP_EXCLAM); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EXCLAM),
        stAllKeys.push_back(SDLK_KP_MEMSTORE); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSTORE),
        stAllKeys.push_back(SDLK_KP_MEMRECALL); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMRECALL),
        stAllKeys.push_back(SDLK_KP_MEMCLEAR); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMCLEAR),
        stAllKeys.push_back(SDLK_KP_MEMADD); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMADD),
        stAllKeys.push_back(SDLK_KP_MEMSUBTRACT); // That is   SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSUBTRACT),
        stAllKeys.push_back(SDLK_KP_MEMMULTIPLY); // That is       SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMMULTIPLY),
        stAllKeys.push_back(SDLK_KP_MEMDIVIDE); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMDIVIDE),
        stAllKeys.push_back(SDLK_KP_PLUSMINUS); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUSMINUS),
        stAllKeys.push_back(SDLK_KP_CLEAR); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEAR),
        stAllKeys.push_back(SDLK_KP_CLEARENTRY); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEARENTRY),
        stAllKeys.push_back(SDLK_KP_BINARY); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BINARY),
        stAllKeys.push_back(SDLK_KP_OCTAL); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_OCTAL),
        stAllKeys.push_back(SDLK_KP_DECIMAL); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DECIMAL),
        stAllKeys.push_back(SDLK_KP_HEXADECIMAL); // That is  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HEXADECIMAL),

        stAllKeys.push_back(SDLK_LCTRL); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LCTRL),
        stAllKeys.push_back(SDLK_LSHIFT); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LSHIFT),
        stAllKeys.push_back(SDLK_LALT); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LALT),
        stAllKeys.push_back(SDLK_LGUI); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LGUI),
        stAllKeys.push_back(SDLK_RCTRL); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RCTRL),
        stAllKeys.push_back(SDLK_RSHIFT); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RSHIFT),
        stAllKeys.push_back(SDLK_RALT); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RALT),
        stAllKeys.push_back(SDLK_RGUI); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RGUI),

        stAllKeys.push_back(SDLK_MODE); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MODE),

        stAllKeys.push_back(SDLK_AUDIONEXT); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIONEXT),
        stAllKeys.push_back(SDLK_AUDIOPREV); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOPREV),
        stAllKeys.push_back(SDLK_AUDIOSTOP); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOSTOP),
        stAllKeys.push_back(SDLK_AUDIOPLAY); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOPLAY),
        stAllKeys.push_back(SDLK_AUDIOMUTE); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOMUTE),
        stAllKeys.push_back(SDLK_MEDIASELECT); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIASELECT),
        stAllKeys.push_back(SDLK_WWW); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_WWW),
        stAllKeys.push_back(SDLK_MAIL); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MAIL),
        stAllKeys.push_back(SDLK_CALCULATOR); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CALCULATOR),
        stAllKeys.push_back(SDLK_COMPUTER); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_COMPUTER),
        stAllKeys.push_back(SDLK_AC_SEARCH); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_SEARCH),
        stAllKeys.push_back(SDLK_AC_HOME); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_HOME),
        stAllKeys.push_back(SDLK_AC_BACK); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BACK),
        stAllKeys.push_back(SDLK_AC_FORWARD); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_FORWARD),
        stAllKeys.push_back(SDLK_AC_STOP); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_STOP),
        stAllKeys.push_back(SDLK_AC_REFRESH); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_REFRESH),
        stAllKeys.push_back(SDLK_AC_BOOKMARKS); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BOOKMARKS),

        stAllKeys.push_back(SDLK_BRIGHTNESSDOWN); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_BRIGHTNESSDOWN),
        stAllKeys.push_back(SDLK_BRIGHTNESSUP); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_BRIGHTNESSUP),
        stAllKeys.push_back(SDLK_DISPLAYSWITCH); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DISPLAYSWITCH),
        stAllKeys.push_back(SDLK_KBDILLUMTOGGLE); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMTOGGLE),
        stAllKeys.push_back(SDLK_KBDILLUMDOWN); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMDOWN),
        stAllKeys.push_back(SDLK_KBDILLUMUP); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMUP),
        stAllKeys.push_back(SDLK_EJECT); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EJECT),
        stAllKeys.push_back(SDLK_SLEEP); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SLEEP),
        stAllKeys.push_back(SDLK_APP1); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_APP1),
        stAllKeys.push_back(SDLK_APP2); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_APP2),

        stAllKeys.push_back(SDLK_AUDIOREWIND); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOREWIND),
        stAllKeys.push_back(SDLK_AUDIOFASTFORWARD); // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOFASTFORWARD)
        // End        
        KeyClean(true);
        MouseClean(true);
        // And if you thought I typed all that stuff above, you're NUTS!
        TQSE_InitDone = true;
    }

    void TQSE_Poll(EventFunction EventCallBack) {
        InitCheck();
        KeyClean();
        MouseClean();
        stAppTerminate = false;
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            switch (e.type) {
            case SDL_KEYDOWN: {
                auto pkey = e.key.keysym.sym;
                stKeyDown[pkey] = true;
                stKeyHit[pkey] = stKeyDown[pkey] && (!stKeyOldDown[pkey]);
                //printf("DOWN: %d\n",pkey);
                break;
            }
            case SDL_KEYUP: {
                auto pkey = e.key.keysym.sym;
                //printf("UP:   %d\n", pkey);
                stKeyDown[pkey] = false;
                break;
            }
            case SDL_MOUSEBUTTONDOWN: {
                auto pbut = e.button.button;
                MsButDown[pbut] = true;
                MsButHit[pbut] = MsButDown[pbut] && (!MsButOldDown[pbut]);
                break;
            }
            case SDL_MOUSEBUTTONUP: {
                auto pbut = e.button.button;
                MsButDown[pbut] = false;
                break;
            }
            case SDL_QUIT:
                stAppTerminate = true;
                break;
            }
            if (EventCallBack) EventCallBack(&e);
        }
    }

    bool TQSE_Quit() {
        return stAppTerminate;
    }

    bool TQSE_KeyHit(SDL_KeyCode c) {
        return stKeyHit[c];
    }

    bool TQSE_KeyDown(SDL_KeyCode c) {
        return stKeyDown[c];
    }

    int TQSE_MouseX() {
        int x, y;
        SDL_GetMouseState(&x, &y);
        return x;
    }

    int TQSE_MouseY() {
        int x, y;
        SDL_GetMouseState(&x, &y);
        return y;
    }

    void HideMouse() {
        SDL_ShowCursor(SDL_DISABLE);
    }

    void ShowMouse() {
        SDL_ShowCursor(SDL_ENABLE);
    }

    bool TQSE_MouseDown(int code) {
        if (code < 0 || code >= maxmousebuttons) return false;
        return MsButDown[code];        
    }

    bool TQSE_MouseHit(int code) {
        if (code < 0 || code >= maxmousebuttons) return false;
        return MsButHit[code];
    }

    int TQSE_KeyByName(std::string name) {
        return SDL_GetKeyFromName(name.c_str());        
    }

    void TQSE_ShowKeyNames() {
        for (auto kc : stAllKeys) {
            printf("%10s = %9d\n", SDL_GetKeyName(kc), kc);
        }
    }

    bool TQSE_Yes(std::string question) {
        const SDL_MessageBoxButtonData buttons[] = {
            //{ /* .flags, .buttonid, .text */        0, 0, "no" },
            { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Yes" },
            { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "No" },
        };
        const SDL_MessageBoxColorScheme colorScheme = {
            { /* .colors (.r, .g, .b) */
                /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
                { 100,   0,   0 },
                /* [SDL_MESSAGEBOX_COLOR_TEXT] */
                {  255, 180,   0 },
                /* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
                { 255, 255,   0 },
                /* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
                {   0,   0, 255 },
                /* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
                { 255,   0, 255 }
            }
        };
        const SDL_MessageBoxData messageboxdata = {
            SDL_MESSAGEBOX_INFORMATION, /* .flags */
            NULL, /* .window */
            "Notice", /* .title */
            question.c_str(), /* .message */
            SDL_arraysize(buttons), /* .numbuttons */
            buttons, /* .buttons */
            &colorScheme /* .colorScheme */
        };
        int buttonid{ 0 };
        if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
            //SDL_Log("error displaying message box");
            std::cout << "SDL Message Box Failure\n";
            return false;

        }
        if (buttonid == -1) {
            SDL_Log("no selection");
        } else {
            SDL_Log("selection was %s", buttons[buttonid].text);
            std::cout << "User selected " << buttonid << " -> " << buttons[buttonid].text << std::endl;
        }
        switch (buttonid) {
        case -1:
        case 1:
            return false;
        case 0:
            return true;
        default:
            std::cout << "Is the input right? I don't think so!\x7\n";
            return false;
            break;
        }
    }
}