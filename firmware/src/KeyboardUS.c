/*
 * Copyright 2013-2016 Esrille Inc.
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
#include <system.h>

static uint8_t const baseKeys[BASE_MAX + 1][5] =
{
    {KEY_U, KEY_S, KEY_MINUS, KEY_Z, KEY_ENTER},
    {KEY_U, KEY_S, KEY_MINUS, KEY_K, KEY_ENTER},
};

/* Modified Dubeolsik layout (US-K).
 * This layout is meant specifically to be used for a software IME Dubeolsik
 * layer. The basik Dubeolsik system has been modified for a nice complement to
 * the ZQ layout. Also, the most common consonants and vowels have been placed
 * on the home row, etc.
 */
static uint8_t const matrixQwerty[8][12] =
{
    KEY_LEFT_BRACKET, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_EQUAL,
    KEY_GRAVE_ACCENT, KEY_F1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_F12, KEY_BACKSLASH,
    KEY_RIGHT_BRACKET, KEY_1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_0, KEY_MINUS,
    KEY_CAPS_LOCK, KEY_2, KEY_3, KEY_4, KEY_5, 0, 0, KEY_6, KEY_7, KEY_8, KEY_9, KEY_QUOTE,
    KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, 0, 0, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P,
    KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_ESCAPE, KEY_APPLICATION, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON,
    KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_TAB, KEY_ENTER, KEY_N, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH,
    KEY_LEFTCONTROL, KEY_LEFT_GUI, KEY_LEFT_FN, KEY_LEFTSHIFT, KEY_BACKSPACE, KEY_LEFTALT, KEY_RIGHTALT, KEY_SPACEBAR, KEY_RIGHTSHIFT, KEY_RIGHT_FN, KEY_RIGHT_GUI, KEY_RIGHTCONTROL
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
    KEY_LEFTCONTROL, KEY_LEFT_GUI, KEY_LEFT_FN, KEY_LEFTSHIFT, KEY_BACKSPACE, KEY_LEFTALT, KEY_RIGHTALT, KEY_SPACEBAR, KEY_RIGHTSHIFT, KEY_RIGHT_FN, KEY_RIGHT_GUI, KEY_RIGHTCONTROL
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
    KEY_LEFTCONTROL, KEY_LEFT_GUI, KEY_LEFT_FN, KEY_LEFTSHIFT, KEY_SPACEBAR, KEY_LEFTALT, KEY_RIGHTALT, KEY_SPACEBAR, KEY_RIGHTSHIFT, KEY_RIGHT_FN, KEY_RIGHT_GUI, KEY_RIGHTCONTROL
};

static uint8_t const matrixZq[8][12] =
{
    00, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, 00,
    00, KEY_F1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_F12, 00,
    00, KEY_EQUAL, 0, 0, 0, 0, 0, 0, 0, 0, KEY_ZQ_DOLLAR, 00,
    KEY_ZQ_COLON, KEY_ZQ_TILDE, KEY_SLASH, KEY_ZQ_AMPERSAND, 00, 0, 0, 00, KEY_GRAVE_ACCENT, KEY_ZQ_EXCLAM, KEY_ZQ_QMARK, KEY_SEMICOLON,
    00,     KEY_Y, KEY_U, KEY_Q, KEY_Z,                     0, 0,               KEY_B, KEY_F, KEY_L, KEY_R, 00,
    KEY_A,  KEY_I, KEY_E, KEY_O, KEY_X,                     KEY_ENTER, KEY_ENTER,  KEY_D, KEY_H, KEY_T, KEY_N, KEY_S,
    KEY_J,  KEY_P, KEY_K, KEY_ZQ_DOUBLE_QUOTE, KEY_QUOTE,   KEY_TAB, KEY_DELETE, KEY_V, KEY_M, KEY_G, KEY_W, KEY_C,
    KEY_LEFTSHIFT, KEY_LEFT_GUI, KEY_CAPS_LOCK, KEY_SPACEBAR, KEY_ESCAPE, KEY_LEFTCONTROL, KEY_FN2, KEY_COMMA, KEY_FN, KEY_LEFTALT, KEY_RIGHTALT, KEY_RIGHTSHIFT
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
    KEY_LEFTCONTROL, KEY_LEFT_GUI, KEY_LEFT_FN, KEY_LEFTSHIFT, KEY_BACKSPACE, KEY_LEFTALT, KEY_RIGHTALT, KEY_SPACEBAR, KEY_RIGHTSHIFT, KEY_RIGHT_FN, KEY_RIGHT_GUI, KEY_RIGHTCONTROL
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
    KEY_LEFTCONTROL, KEY_LEFT_GUI, KEY_LEFT_FN, KEY_LEFTSHIFT, KEYPAD_ENTER, KEY_LEFTALT, KEY_RIGHTALT, KEY_SPACEBAR, KEY_RIGHTSHIFT, KEY_RIGHT_FN, KEY_RIGHT_GUI, KEY_RIGHTCONTROL
};

static uint8_t mode;
static uint8_t lastShift;

void loadBaseSettings(void)
{
    mode = ReadNvram(EEPROM_BASE);
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
    WriteNvram(EEPROM_BASE, mode);
//    emitBaseName();
}

int8_t isDigit(uint8_t code)
{
    return code == 25 || code == 34 || (37 <= code && code <= 46);
}

int8_t isJP(void)
{
    return mode == BASE_JIS || mode == BASE_NICOLA_F;
}

int8_t pressed(const uint8_t* current, const uint8_t* processed, uint8_t modifiers, uint8_t k)
{
    for (int8_t i = 2; i < 8; ++i) {
        uint8_t code = current[i];
        uint8_t key = getKeyNumLock(code);
        if (!key)
            key = getKeyBase(code);
        key = toggleKanaMode(key, modifiers, !memchr(processed + 2, key, 6));
        if (k == key)
            return 1;
    }
    return 0;
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
            case KEY_ZQ_DOUBLE_QUOTE:
            case KEY_ZQ_TILDE:
            case KEY_ZQ_DOLLAR:
            case KEY_ZQ_AMPERSAND:
            case KEY_ZQ_EXCLAM:
            case KEY_ZQ_COLON:

                modifiers |= MOD_LEFTSHIFT;
                i = 8;
                switch (key) {
                    case KEY_ZQ_QMARK:
                        report[count++] = KEY_SLASH;
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
            case KEY_GRAVE_ACCENT:
            case KEY_MINUS:
            case KEY_EQUAL:
            case KEY_LEFT_BRACKET:
            case KEY_RIGHT_BRACKET:
            case KEY_BACKSLASH:
            case KEY_SEMICOLON:
            case KEY_QUOTE:
            case KEY_COMMA:
            case KEY_PERIOD:
            case KEY_SLASH:
                report[count++] = key;
                if (mode == BASE_ZQ || mode == BASE_QWERTY)
                    i = 8;
                break;
            default:
                /* Take into consideration the lastShift and lastExtra keys, if
                 * they are set. */
                if ((lastShift | current[0]) & MOD_SHIFT) {
                    /*
                     * We do not use MOD_SHIFT, because by convention (as with
                     * the ZQ keys above) we always use MOD_LEFTSHIFT to mean
                     * "SHIFT". There is no need to set both the LEFTSHIFT and
                     * RIGHTSHIFT bits anyway. We prefer minimal behavior.
                     */
                    modifiers |= MOD_LEFTSHIFT;
                    report[count++] = key;
                    /* NOTE: The following discussion is only an educated guess.
                     *
                     * If we are typing the keys 'SHIFT, T, H' in rapid
                     * sequence, then we don't want the H to be capitalized like
                     * the T. We catch this case with a memcmp, which returns 0
                     * if there is no change between the previous ("processed")
                     * and "current" bytes. If there is a change in the bytes
                     * (going from 'T' to 'H'), then exit the loop and make 'H'
                     * become processed on its own in the next "report".
                     *
                     * The call to pressed() is necessary to prevent the
                     * capslock key (remapped to Hyper key with xmodmap) from
                     * triggering the memcmp() boolean; the capslock key should
                     * be treated as a modifier key that does not count as being
                     * "pressed" in the usual sense. Without this, we are unable
                     * to press SHIFT+CAPS_LOCK as part of one keypress
                     * ("report").
                     */
                    if (memcmp(current, processed, 8)
                        && !pressed(current, processed, modifiers, KEY_CAPS_LOCK)) {
                        lastShift = 0;
                        goto exit_loop;
                    }
                } else {
                    report[count++] = key;
                }
                break;
            }
        }
    }
    lastShift = current[0];
exit_loop:
    report[0] = modifiers;
    lastExtra = modifiersExtra;
    return XMIT_NORMAL;
}

uint8_t controlZQLED(uint8_t report)
{
    if (mode == BASE_ZQ || mode == BASE_QWERTY) {
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
    return !(current[0] & (MOD_ALT | MOD_CONTROL | MOD_GUI)) && !(current[1] & MOD_PAD) && (mode == BASE_ZQ || mode == BASE_QWERTY);
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
