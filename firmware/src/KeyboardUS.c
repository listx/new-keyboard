/*
 * Copyright 2013-2014 Esrille Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Keyboard.h"

#include <string.h>

#ifdef __XC8
#include <xc.h>
#endif

static uint8_t const baseKeys[BASE_MAX + 1][5] =
{
    {KEY_U, KEY_S, KEY_ENTER},
    {KEY_U, KEY_S, KEY_MINUS, KEY_D, KEY_ENTER},
    {KEY_U, KEY_S, KEY_MINUS, KEY_C, KEY_ENTER},
    {KEY_J, KEY_P, KEY_ENTER},
    {KEY_J, KEY_P, KEY_MINUS, KEY_N, KEY_ENTER},
    {KEY_U, KEY_S, KEY_MINUS, KEY_Z, KEY_ENTER},
};

static uint8_t const matrixQwerty[8][12] =
{
    KEY_LEFT_BRACKET, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_EQUAL,
    KEY_GRAVE_ACCENT, KEY_F1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_F12, KEY_BACKSLASH,
    KEY_RIGHT_BRACKET, KEY_1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_0, KEY_MINUS,
    KEY_CAPS_LOCK, KEY_2, KEY_3, KEY_4, KEY_5, 0, 0, KEY_6, KEY_7, KEY_8, KEY_9, KEY_QUOTE,
    KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, 0, 0, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P,
    KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_ESCAPE, KEY_APPLICATION, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON,
    KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_TAB, KEY_ENTER, KEY_N, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH,
    KEY_LEFTCONTROL, KEY_LEFT_GUI, KEY_FN, KEY_LEFTSHIFT, KEY_BACKSPACE, KEY_LEFTALT, KEY_RIGHTALT, KEY_SPACEBAR, KEY_RIGHTSHIFT, KEY_FN, KEY_RIGHT_GUI, KEY_RIGHTCONTROL
};

static uint8_t const matrixDvorak[8][12] =
{
    KEY_LEFT_BRACKET, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_BACKSLASH,
    KEY_GRAVE_ACCENT, KEY_F1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_F12, KEY_EQUAL,
    KEY_RIGHT_BRACKET, KEY_1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_0, KEY_SLASH,
    KEY_CAPS_LOCK, KEY_2, KEY_3, KEY_4, KEY_5, 0, 0, KEY_6, KEY_7, KEY_8, KEY_9, KEY_MINUS,
    KEY_QUOTE, KEY_COMMA, KEY_PERIOD, KEY_P, KEY_Y, 0, 0, KEY_F, KEY_G, KEY_C, KEY_R, KEY_L,
    KEY_A, KEY_O, KEY_E, KEY_U, KEY_I, KEY_ESCAPE, KEY_APPLICATION, KEY_D, KEY_H, KEY_T, KEY_N, KEY_S,
    KEY_SEMICOLON, KEY_Q, KEY_J, KEY_K, KEY_X, KEY_TAB, KEY_ENTER, KEY_B, KEY_M, KEY_W, KEY_V, KEY_Z,
    KEY_LEFTCONTROL, KEY_LEFT_GUI, KEY_FN, KEY_LEFTSHIFT, KEY_BACKSPACE, KEY_LEFTALT, KEY_RIGHTALT, KEY_SPACEBAR, KEY_RIGHTSHIFT, KEY_FN, KEY_RIGHT_GUI, KEY_RIGHTCONTROL
};

static uint8_t const matrixColemak[8][12] =
{
    KEY_LEFT_BRACKET, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_EQUAL,
    KEY_GRAVE_ACCENT, KEY_F1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_F12, KEY_BACKSLASH,
    KEY_RIGHT_BRACKET, KEY_1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_0, KEY_MINUS,
    KEY_BACKSPACE, KEY_2, KEY_3, KEY_4, KEY_5, 0, 0, KEY_6, KEY_7, KEY_8, KEY_9, KEY_QUOTE,
    KEY_Q, KEY_W, KEY_F, KEY_P, KEY_G, 0, 0, KEY_J, KEY_L, KEY_U, KEY_Y, KEY_SEMICOLON,
    KEY_A, KEY_R, KEY_S, KEY_T, KEY_D, KEY_ESCAPE, KEY_APPLICATION, KEY_H, KEY_N, KEY_E, KEY_I, KEY_O,
    KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_TAB, KEY_ENTER, KEY_K, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH,
    KEY_LEFTCONTROL, KEY_LEFT_GUI, KEY_FN, KEY_LEFTSHIFT, KEY_SPACEBAR, KEY_LEFTALT, KEY_RIGHTALT, KEY_SPACEBAR, KEY_RIGHTSHIFT, KEY_FN, KEY_RIGHT_GUI, KEY_RIGHTCONTROL
};

static uint8_t const matrixZq[8][12] =
{
    KEY_ENTER, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, 00,
    00, KEY_F1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_F12, 00,
    00, KEY_EQUAL, 0, 0, 0, 0, 0, 0, 0, 0, KEY_ZQ_DOLLAR, 00,
    KEY_LEFTSHIFT, KEY_ZQ_TILDE, KEY_SLASH, KEY_ZQ_QMARK, KEY_ZQ_AMPERSAND, 0, 0, KEY_GRAVE_ACCENT, KEY_ZQ_UNDERSCORE, KEY_MINUS, KEY_ZQ_EXCLAM, KEY_RIGHTSHIFT,
    KEY_Z, KEY_R, KEY_D, KEY_F, KEY_PERIOD,     0, 0,                           KEY_ZQ_DOUBLE_QUOTE,    KEY_Y, KEY_U, KEY_P, KEY_Q,
    KEY_S, KEY_N, KEY_T, KEY_H, KEY_L,          KEY_HOME, KEY_END,              KEY_X,                  KEY_A, KEY_E, KEY_I, KEY_O,
    KEY_C, KEY_W, KEY_G, KEY_M, KEY_SEMICOLON,  KEY_ZQ_COLON, KEY_ZQ_ASTERISK,  KEY_QUOTE,              KEY_J, KEY_K, KEY_B, KEY_V,
    KEY_ESCAPE, KEY_LEFT_GUI, KEY_CAPS_LOCK, KEY_SPACEBAR, KEY_ENTER, KEY_LEFTCONTROL, KEY_FN2, KEY_COMMA, KEY_FN, KEY_LEFTALT, KEY_RIGHTALT, KEY_TAB
};

//
// Japanese layouts
//
// [{   KEY_RIGHT_BRACKET
// ]}   KEY_NON_US_HASH
// \|   KEY_INTERNATIONAL3
// @`   KEY_LEFT_BRACKET
// -=   KEY_MINUS
// :*   KEY_QUOTE
// ^~   KEY_EQUAL
//  _   KEY_INTERNATIONAL1
// no-convert   KEY_INTERNATIONAL5
// convert      KEY_INTERNATIONAL4
// hiragana     KEY_INTERNATIONAL2
// zenkaku      KEY_GRAVE_ACCENT
//

static uint8_t const matrixJIS[8][12] =
{
    KEY_RIGHT_BRACKET, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_EQUAL,
    KEY_INTERNATIONAL3, KEY_F1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_F12, KEY_LEFT_BRACKET,
    KEY_NON_US_HASH, KEY_1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_0, KEY_MINUS,
    KEY_CAPS_LOCK, KEY_2, KEY_3, KEY_4, KEY_5, 0, 0, KEY_6, KEY_7, KEY_8, KEY_9, KEY_QUOTE,
    KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, 0, 0, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P,
    KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_ESCAPE, KEY_APPLICATION, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON,
    KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_TAB, KEY_ENTER, KEY_N, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH,
    KEY_LEFTCONTROL, KEY_LEFT_GUI, KEY_FN, KEY_LEFTSHIFT, KEY_BACKSPACE, KEY_LEFTALT, KEY_RIGHTALT, KEY_SPACEBAR, KEY_RIGHTSHIFT, KEY_FN, KEY_RIGHT_GUI, KEY_RIGHTCONTROL
};

static uint8_t const matrixNicolaF[8][12] =
{
    KEY_RIGHT_BRACKET, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_MINUS,
    KEY_INTERNATIONAL3, KEY_F1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_F12, KEY_LEFT_BRACKET,
    KEY_NON_US_HASH, KEY_1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_0, KEY_QUOTE,
    KEY_EQUAL, KEY_2, KEY_3, KEY_4, KEY_5, 0, 0, KEY_6, KEY_7, KEY_8, KEY_9, KEY_BACKSPACE,
    KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, 0, 0, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P,
    KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_ESCAPE, KEY_APPLICATION, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON,
    KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_TAB, KEY_ENTER, KEY_N, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH,
    KEY_LEFTCONTROL, KEY_LEFT_GUI, KEY_FN, KEY_LEFTSHIFT, KEYPAD_ENTER, KEY_LEFTALT, KEY_RIGHTALT, KEY_SPACEBAR, KEY_RIGHTSHIFT, KEY_FN, KEY_RIGHT_GUI, KEY_RIGHTCONTROL
};

static uint8_t mode;
static uint8_t lastShift;

void initKeyboardBase(void)
{
    mode = eeprom_read(EEPROM_BASE);
    if (BASE_MAX < mode)
        mode = 0;
}

void emitBaseName(void)
{
    emitStringN(baseKeys[mode], 5);
}

void switchBase(void)
{
    ++mode;
    if (BASE_MAX < mode)
        mode = 0;
    eeprom_write(EEPROM_BASE, mode);
    emitBaseName();
}

int8_t isDigit(uint8_t code)
{
    return code == 25 || code == 34 || (37 <= code && code <= 46);
}

int8_t isJP(void)
{
    return mode == BASE_JIS || mode == BASE_NICOLA_F;
}

int8_t processKeysBase(const uint8_t* current, const uint8_t* processed, uint8_t* report)
{
    uint8_t modifiers;

    /*
     * current[0] can hold the GUI (Super), CTRL, ALT, and SHIFT modifiers.
     * current[1] can hold the MOUSE, FN, and FN2 modifiers.
     *
     * We steal the concept of "sticky" shift keys from the JIS X6004 layout
     * where the shift case can be selected by pressing and releasing a shift
     * key alone before pressing the next key, AKA prefix-shift layout.
     *
     * In our case, we have 3 sticky modifers: SHIFT, FN, and FN2. SHIFT is
     * unique in that it can be generated from 2 unique keys, either the left or
     * right SHIFT key. Otherwise the Sticky behavior of the 3 modifiers listed
     * above are treated the same way with the use of *lastShift* and *lastExtra*.
     * Normally we would have a single byte to store the SHIFT/FN/FN2 modifer
     * info (as we only need 3 bits in our use case --- one for each modifier),
     * but because FN2 is defined as 2, it overlaps with the LEFTSHIFT key.
     */


    /* We check all non-Shift key modifiers, and save it into *modifiers*. */
    modifiers = current[0] & ~MOD_SHIFT;
    report[0] = modifiers;

    if (!(current[1] & MOD_PAD)) {
        uint8_t count = 2;
        /* We loop 6 times, presumably because of 6-key rollover. */
        for (int8_t i = 2; i < 8; ++i) {
            uint8_t code = current[i];
            uint8_t key = getKeyNumLock(code);
            if (!key)
                key = getKeyBase(code);
            key = toggleKanaMode(key, modifiers, !memchr(processed + 2, key, 6));

            /* Process special keys that are private to ZQ layout. */
            switch (key) {
            case KEY_ZQ_QMARK:
            case KEY_ZQ_UNDERSCORE:
            case KEY_ZQ_ASTERISK:
            case KEY_ZQ_DOUBLE_QUOTE:
            case KEY_ZQ_TILDE:
            case KEY_ZQ_DOLLAR:
            case KEY_ZQ_AMPERSAND:
            case KEY_ZQ_EXCLAM:
            case KEY_ZQ_COLON:

                modifiers |= MOD_LEFTSHIFT;
                switch (key) {
                    case KEY_ZQ_QMARK:
                        report[count++] = KEY_SLASH;
                        break;
                    case KEY_ZQ_UNDERSCORE:
                        report[count++] = KEY_MINUS;
                        break;
                    case KEY_ZQ_ASTERISK:
                        report[count++] = KEY_8;
                        break;
                    case KEY_ZQ_DOUBLE_QUOTE:
                        report[count++] = KEY_QUOTE;
                        break;
                    case KEY_ZQ_TILDE:
                        report[count++] = KEY_GRAVE_ACCENT;
                        break;
                    case KEY_ZQ_DOLLAR:
                        report[count++] = KEY_4;
                        break;
                    case KEY_ZQ_AMPERSAND:
                        report[count++] = KEY_7;
                        break;
                    case KEY_ZQ_EXCLAM:
                        report[count++] = KEY_1;
                        break;
                    case KEY_ZQ_COLON:
                        report[count++] = KEY_SEMICOLON;
                        break;
                }
                break;
            default:
                /* Take into consideration the lastShift and lastExtra keys, if
                 * they are set. */
                if (lastShift & MOD_SHIFT)
                    /*
                     * We do not use MOD_SHIFT, because by convention (as with
                     * the ZQ keys above) we always use MOD_LEFTSHIFT to mean
                     * "SHIFT". There is no need to set both the LEFTSHIFT and
                     * RIGHTSHIFT bits anyway. We prefer minimal behavior.
                     */
                    modifiers |= MOD_LEFTSHIFT;

                report[count++] = key;
                break;
            }
        }
    }
    report[0] = modifiers;
    lastShift = current[0];
    lastExtra = modifiersExtra;
    return XMIT_NORMAL;
}

uint8_t controlZQLED(uint8_t report)
{
    if (mode == BASE_ZQ) {
        if (prefix & MOD_SHIFT)
            report |= LED_SCROLL_LOCK;
        if (prefixExtra & MOD_FN)
            report |= LED_CAPS_LOCK;
        if (prefixExtra & MOD_FN2)
            report |= LED_NUM_LOCK;

    }
    return report;
}


int8_t isZQMode(const uint8_t* current)
{
    return !(current[0] & (MOD_ALT | MOD_CONTROL | MOD_GUI)) && !(current[1] & MOD_PAD) && mode == BASE_ZQ;
}

uint8_t getKeyBase(uint8_t code)
{
    uint8_t key = getKeyNumLock(code);
    uint8_t row = code / 12;
    uint8_t column = code % 12;
    if (key)
        return key;
    switch (mode) {
    case BASE_QWERTY:
        key = matrixQwerty[row][column];
        break;
    case BASE_DVORAK:
        key = matrixDvorak[row][column];
        break;
    case BASE_COLEMAK:
        key = matrixColemak[row][column];
        break;
    case BASE_JIS:
        key = matrixJIS[row][column];
        break;
    case BASE_NICOLA_F:
        key = matrixNicolaF[row][column];
        break;
    case BASE_ZQ:
        key = matrixZq[row][column];
        break;
    default:
        key = matrixQwerty[row][column];
        break;
    }
    return processModKey(key);
}
