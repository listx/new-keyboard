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
#include "Mouse.h"

#include <stdint.h>
#include <string.h>
#include <system.h>

NVRAM_DATA(BASE_ZQ, KANA_ROMAJI, OS_PC, DELAY_12, MOD_DEFAULT, LED_DEFAULT, IME_MS, PAD_SENSE_1);

uint8_t os;
uint8_t mod;
uint8_t prefix_shift;
uint8_t prefix;
uint8_t prefixExtra;
uint8_t lastExtra;

#define MAX_OS_KEY_NAME     5

static uint8_t const osKeys[OS_MAX + 1][MAX_OS_KEY_NAME] =
{
    {KEY_P, KEY_C, KEY_ENTER},
    {KEY_M, KEY_A, KEY_C, KEY_ENTER},
};

#define MAX_MOD_KEY_NAME    6
#define MAX_MOD_KEYS        7

static uint8_t const modMap[MOD_MAX + 1][MAX_MOD_KEYS] =
{
    {KEY_LEFTCONTROL, KEY_LEFTSHIFT, KEY_LEFT_GUI, KEY_LEFTALT, KEY_RIGHTALT, KEY_RIGHTSHIFT, KEY_RIGHTCONTROL },
    {KEY_LEFTCONTROL, KEY_LEFTSHIFT, KEY_LEFTALT, KEY_LANG2, KEY_LANG1, KEY_RIGHTSHIFT, KEY_RIGHTCONTROL },
    {KEY_LEFTCONTROL, KEY_LEFTSHIFT, KEY_LEFT_GUI, KEY_LANG2, KEY_LANG1, KEY_RIGHTSHIFT, KEY_RIGHTCONTROL },
    {KEY_LEFTSHIFT, KEY_LEFTCONTROL, KEY_LEFT_GUI, KEY_LEFTALT, KEY_RIGHTALT, KEY_RIGHTCONTROL, KEY_RIGHTSHIFT },
    {KEY_LEFTSHIFT, KEY_LEFTCONTROL, KEY_LEFTALT, KEY_LANG2, KEY_LANG1, KEY_RIGHTCONTROL, KEY_RIGHTSHIFT },
    {KEY_LEFTSHIFT, KEY_LEFTCONTROL, KEY_LEFT_GUI, KEY_LANG2, KEY_LANG1, KEY_RIGHTCONTROL, KEY_RIGHTSHIFT },
};

static uint8_t const modKeys[MOD_MAX + 1][MAX_MOD_KEY_NAME] =
{
    {KEY_C, KEY_ENTER},
    {KEY_C, KEY_J, KEY_ENTER},
    {KEY_C, KEY_J, KEY_M, KEY_A, KEY_C, KEY_ENTER},
    {KEY_S, KEY_ENTER},
    {KEY_S, KEY_J, KEY_ENTER},
    {KEY_S, KEY_J, KEY_M, KEY_A, KEY_C, KEY_ENTER},
#ifdef ENABLE_DUAL_ROLE_FN
    {KEY_C, KEY_X, KEY_ENTER},
    {KEY_S, KEY_X, KEY_ENTER},
#endif
};

static uint8_t const matrixFn[8][12][3] =
{
    {{KEY_INSERT}, {KEY_F2}, {KEY_F3}, {KEY_F4}, {KEY_F5}, {KEY_F6}, {KEY_F7}, {KEY_F8}, {KEY_F9}, {KEY_MUTE}, {KEY_VOLUME_DOWN}, {KEY_PAUSE}},
    {{KEY_LEFTCONTROL, KEY_DELETE}, {KEY_F1}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {KEY_VOLUME_UP}, {KEY_SCROLL_LOCK}},
    {{KEY_LEFTCONTROL, KEY_LEFTSHIFT, KEY_Z}, {KEY_LEFTCONTROL, KEY_1}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {KEY_LEFTCONTROL, KEY_0}, {KEY_PRINTSCREEN}},
#ifdef WITH_HOS
    {{KEY_DELETE}, {KEY_LEFTCONTROL, KEY_2}, {KEY_LEFTCONTROL, KEY_3}, {KEY_LEFTCONTROL, KEY_4}, {KEY_LEFTCONTROL, KEY_5}, {0}, {0}, {KEY_LEFTCONTROL, KEY_6}, {KEY_LEFTCONTROL, KEY_7}, {KEY_LEFTCONTROL, KEY_8}, {KEY_LEFTCONTROL, KEY_9}, {KEYPAD_NUM_LOCK}},
#else
    {{KEY_DELETE}, {KEY_LEFTCONTROL, KEY_2}, {KEY_LEFTCONTROL, KEY_3}, {KEY_LEFTCONTROL, KEY_4}, {KEY_LEFTCONTROL, KEY_5}, {0}, {0}, {KEY_LEFTCONTROL, KEY_6}, {KEY_LEFTCONTROL, KEY_LEFTSHIFT, KEY_LEFTARROW}, {KEY_LEFTSHIFT, KEY_UPARROW}, {KEY_LEFTCONTROL, KEY_LEFTSHIFT, KEY_RIGHTARROW}, {KEYPAD_NUM_LOCK}},
#endif
    {{KEY_LEFTCONTROL, KEY_Q}, {KEY_LEFTCONTROL, KEY_W}, {KEY_PAGEUP}, {KEY_LEFTCONTROL, KEY_R}, {KEY_LEFTCONTROL, KEY_T}, {0}, {0}, {KEY_LEFTCONTROL, KEY_HOME}, {KEY_LEFTCONTROL, KEY_LEFTARROW}, {KEY_UPARROW}, {KEY_LEFTCONTROL, KEY_RIGHTARROW}, {KEY_LEFTCONTROL, KEY_END}},
    {{KEY_LEFTCONTROL, KEY_A}, {KEY_LEFTCONTROL, KEY_S}, {KEY_PAGEDOWN}, {KEY_LEFTCONTROL, KEY_F}, {KEY_LEFTCONTROL, KEY_G}, {KEY_ESCAPE}, {KEY_CAPS_LOCK}, {KEY_HOME}, {KEY_LEFTARROW}, {KEY_DOWNARROW}, {KEY_RIGHTARROW}, {KEY_END}},
    {{KEY_LEFTCONTROL, KEY_Z}, {KEY_LEFTCONTROL, KEY_X}, {KEY_LEFTCONTROL, KEY_C}, {KEY_LEFTCONTROL, KEY_V}, {KEY_LANG2}, {KEY_TAB}, {KEY_ENTER}, {KEY_LANG1}, {KEY_LEFTSHIFT, KEY_LEFTARROW}, {KEY_LEFTSHIFT, KEY_DOWNARROW}, {KEY_LEFTSHIFT, KEY_RIGHTARROW}, {KEY_LEFTSHIFT, KEY_END}},
    {{0}, {0}, {0}, {0}, {KEY_LEFTCONTROL, KEY_BACKSPACE}, {0}, {0}, {KEY_LEFTCONTROL, KEY_SPACEBAR}, {0}, {0}, {0}, {0}}
};

static uint8_t const matrixFnZq[2][8][12][3] =
{
    /* KEY_FN */
    {
    {{00}, {KEY_F2}, {KEY_F3}, {KEY_F4}, {KEY_F5}, {KEY_F6}, {KEY_F7}, {KEY_F8}, {KEY_F9}, {KEY_F10}, {KEY_F11}, {00}},
    {{KEY_ENTER}, {KEY_F1}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {KEY_F12}, {KEY_LEFTSHIFT, KEY_SEMICOLON}},
    {{KEY_LEFTCONTROL, KEY_LEFTSHIFT, KEY_Z}, {KEY_ZQ_SOFTTAB2}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {KEY_DELETE}, {KEY_PRINTSCREEN}},
    {{KEY_SLASH}, {00}, {KEY_ZQ_HOMEDIR}, {00}, {00}, {0}, {0}, {00}, {0}, {KEY_ZQ_SOFTTAB2}, {00}, {KEY_BACKSPACE}},
    {{00}, {KEY_7}, {KEY_8}, {KEY_9}, {00}, {0}, {0}, {00}, {KEY_BACKSLASH}, {KEY_LEFTSHIFT, KEY_MINUS}, {KEY_EQUAL}, {00}},
    {{KEY_0}, {KEY_4}, {KEY_5}, {KEY_6}, {00}, {00}, {00}, {KEY_MINUS}, {KEY_LEFTSHIFT, KEY_LEFT_BRACKET}, {KEY_LEFTSHIFT, KEY_9}, {KEY_LEFTSHIFT, KEY_0}, {KEY_LEFTSHIFT, KEY_RIGHT_BRACKET}},
    {{KEY_PERIOD}, {KEY_1}, {KEY_2}, {KEY_3}, {KEY_GRAVE_ACCENT}, {KEY_END}, {KEY_HOME}, {00}, {KEY_LEFT_BRACKET}, {KEY_LEFTSHIFT, KEY_COMMA}, {KEY_LEFTSHIFT, KEY_PERIOD}, {KEY_RIGHT_BRACKET}},
    {{KEY_LEFTSHIFT}, {KEY_RIGHTALT}, {KEY_LEFT_GUI}, {KEY_SPACEBAR}, {KEY_CAPS_LOCK}, {KEY_LEFTCONTROL}, {00}, {00}, {KEY_RIGHT_FN}, {KEY_LEFTALT}, {KEY_RIGHTALT}, {KEY_RIGHTSHIFT}}
    },

    /* KEY_FN2 */
    {
    {{00}, {00}, {00}, {00}, {00}, {00}, {00}, {00}, {00}, {00}, {00}, {00}},
    {{00}, {00}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {00}, {00}},
    {{00}, {00}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {00}, {00}},
    {{00}, {00}, {00}, {00}, {00}, {0}, {0}, {00}, {00}, {00}, {00}, {00}},
    {{00}, {00}, {00}, {00}, {00}, {0}, {0}, {00}, {00}, {00}, {00}, {00}},
    {{00}, {00}, {00}, {00}, {00}, {KEY_PRINTSCREEN}, {KEY_SCROLL_LOCK}, {KEY_LEFTARROW}, {KEY_DOWNARROW}, {KEY_UPARROW}, {KEY_RIGHTARROW}, {00}},
    {{00}, {00}, {00}, {00}, {00}, {KEY_PAUSE}, {KEY_INSERT}, {00}, {00}, {00}, {00}, {KEY_LEFTSHIFT, KEY_INSERT}},
    {{00}, {KEY_RIGHTALT}, {KEY_LEFTCONTROL}, {KEY_BACKSPACE}, {KEY_CAPS_LOCK}, {KEY_LEFT_GUI}, {00}, {00}, {00}, {KEY_LEFTALT}, {KEY_RIGHTALT}, {00}}
    },
};

static const uint8_t cmd_ls[] = {
    KEY_SPACEBAR,
    KEY_L,
    KEY_S,
    KEY_SPACEBAR,
    KEY_MINUS,
    KEY_A,
    KEY_H,
    KEY_L,
    KEY_S,
    KEY_SPACEBAR,
    KEY_MINUS,
    KEY_MINUS,
    KEY_C,
    KEY_O,
    KEY_L,
    KEY_O,
    KEY_R,
    KEY_ENTER,
    0,
};

static uint8_t const matrixFn109[4][3] =
{
    {KEY_INTERNATIONAL5},   // no-convert
    {KEY_INTERNATIONAL4},   // convert
    {KEY_INTERNATIONAL2},   // hiragana
    {KEY_GRAVE_ACCENT}      // zenkaku
};

static uint8_t const matrixNumLock[8][5] =
{
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, KEYPAD_MULTIPLY, 0,
    KEY_CALC, 0, KEYPAD_EQUAL, KEYPAD_DIVIDE, 0,
    0, KEYPAD_7, KEYPAD_8, KEYPAD_9, KEYPAD_SUBTRACT,
    0, KEYPAD_4, KEYPAD_5, KEYPAD_6, KEYPAD_ADD,
    0, KEYPAD_1, KEYPAD_2, KEYPAD_3, KEY_ENTER,
    0, KEYPAD_0, 0, KEYPAD_DOT, 0,
};

#define MAX_DELAY_KEY_NAME  4

static uint8_t const delayKeyNames[DELAY_MAX + 1][MAX_DELAY_KEY_NAME] =
{
    {KEY_D, KEY_0, KEY_ENTER},
    {KEY_D, KEY_1, KEY_2, KEY_ENTER},
    {KEY_D, KEY_2, KEY_4, KEY_ENTER},
    {KEY_D, KEY_3, KEY_6, KEY_ENTER},
    {KEY_D, KEY_4, KEY_8, KEY_ENTER},
};

#define MAX_PREFIX_KEY_NAME  4

static uint8_t const prefixKeyNames[PREFIXSHIFT_MAX + 1][MAX_PREFIX_KEY_NAME] =
{
    {KEY_O, KEY_F, KEY_F, KEY_ENTER},
    {KEY_O, KEY_N, KEY_ENTER},
    {KEY_L, KEY_E, KEY_D, KEY_ENTER},
};

static uint8_t const codeRev2[8][12] =
{
    13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 22,
    36, 0, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY, 11, 47,
    84, 24, VOID_KEY, VOID_KEY, VOID_KEY, 65, 66, VOID_KEY, VOID_KEY, VOID_KEY, 35, 95,
    85, 12, VOID_KEY, VOID_KEY, VOID_KEY, 77, 78, VOID_KEY, VOID_KEY, VOID_KEY, 23, 94,
    86, 25, 37, 38, 39, 40, 43, 44, 45, 46, 34, 93,
    87, 48, 49, 50, 51, 52, 55, 56, 57, 58, 59, 92,
    88, 60, 61, 62, 63, 64, 67, 68, 69, 70, 71, 91,
    89, 72, 73, 74, 75, 76, 79, 80, 81, 82, 83, 90,
};

typedef struct Keys {
    uint8_t keys[6];
} Keys;

static uint8_t ordered_keys[MAX_MACRO_SIZE];
static uint8_t ordered_pos = 0;
static uint8_t ordered_max;

static uint8_t currentDelay;
static Keys keys[DELAY_MAX + 2];
static int8_t currentKey = 0;

static uint8_t tick;
static uint8_t processed[8];

static uint8_t modifiers;
static uint8_t modifiersPrev;
uint8_t modifiersExtra;
static uint8_t modifiersExtraPrev;
static uint8_t current[8];
static int8_t count;
static uint8_t rowCount[8];
static uint8_t columnCount[12];

static uint8_t led;

#ifdef ENABLE_DUAL_ROLE_FN
static uint8_t modFn;
static uint8_t dualFn;  // Used for dual-role FN keys
#endif

void initKeyboard(void)
{
    memset(keys, VOID_KEY, sizeof keys);
    currentKey = 0;
    memset(current, 0, 8);
    memset(processed, 0, 2);
    memset(processed + 2, VOID_KEY, 6);
    modifiers = modifiersPrev = 0;
    lastExtra = modifiersExtra = modifiersExtraPrev = 0;
    count = 2;
    loadKeyboardSettings();
}

void loadKeyboardSettings(void)
{
    os = ReadNvram(EEPROM_OS);
    if (OS_MAX < os)
        os = 0;
    mod = ReadNvram(EEPROM_MOD);
    if (MOD_MAX < mod)
        mod = 0;
    currentDelay = ReadNvram(EEPROM_DELAY);
    if (DELAY_MAX < currentDelay)
        currentDelay = 0;
    prefix_shift = ReadNvram(EEPROM_PREFIX);
    if (PREFIXSHIFT_MAX < prefix_shift)
        prefix_shift = 0;
    loadBaseSettings();
    loadKanaSettings();
}

void emitOSName(void)
{
    emitStringN(osKeys[os], MAX_OS_KEY_NAME);
}

void switchOS(void)
{
    ++os;
    if (OS_MAX < os)
        os = 0;
    WriteNvram(EEPROM_OS, os);
    emitOSName();
}

void emitModName(void)
{
    emitStringN(modKeys[mod], MAX_MOD_KEY_NAME);
}

void switchMod(void)
{
    ++mod;
    if (MOD_MAX < mod)
        mod = 0;
    WriteNvram(EEPROM_MOD, mod);
    emitModName();
}

void emitDelayName(void)
{
    emitStringN(delayKeyNames[currentDelay], MAX_DELAY_KEY_NAME);
}

void switchDelay(void)
{
    ++currentDelay;
    if (DELAY_MAX < currentDelay)
        currentDelay = 0;
    WriteNvram(EEPROM_DELAY, currentDelay);
    emitDelayName();
}

void emitPrefixShift(void)
{
    emitStringN(prefixKeyNames[prefix_shift], MAX_PREFIX_KEY_NAME);
}

void switchPrefixShift(void)
{
    ++prefix_shift;
    if (PREFIXSHIFT_MAX < prefix_shift)
        prefix_shift = 0;
    WriteNvram(EEPROM_PREFIX, prefix_shift);
    emitPrefixShift();
}

#define CODE_A      (5*12+0)

void onPressed(int8_t row, uint8_t column)
{
    uint8_t key;
    uint8_t code;

    if (2 <= BOARD_REV_VALUE)
        code = codeRev2[row][column];
    else
        code = 12 * row + column;
    ++columnCount[column];
    ++rowCount[row];
    key = getKeyBase(code);
    if (KEY_LEFTCONTROL <= key && key <= KEY_RIGHT_GUI) {
        modifiers |= 1u << (key - KEY_LEFTCONTROL);
        return;
    }
    if (KEY_RIGHT_FN == key) {
        modifiersExtra |= MOD_FN;
        return;
    }
    if (KEY_FN2 == key) {
        modifiersExtra |= MOD_FN2;
        return;
    }
    if (KEY_LEFT_FN <= key && key <= KEY_RIGHT_FN) {
        current[1] |= 1u << (key - KEY_LEFT_FN);
        return;
    }
    if (count < 8)
        current[count++] = code;
}

static int8_t detectGhost(void)
{
    uint8_t i;
    int8_t detected;
    uint8_t rx = 0;
    uint8_t cx = 0;

    for (i = 0; i < sizeof rowCount; ++i) {
        if (2 <= rowCount[i])
            ++rx;
    }
    for (i = 0; i < sizeof columnCount; ++i) {
        if (2 <= columnCount[i])
            ++cx;
    }
    detected = (2 <= rx && 2 <= cx);
    memset(rowCount, 0, sizeof rowCount);
    memset(columnCount, 0, sizeof columnCount);
    return detected;
}

uint8_t beginMacro(uint8_t max)
{
    ordered_pos = 1;
    ordered_max = max;
    return ordered_keys[0];
}

uint8_t peekMacro(void)
{
    if (ordered_pos < ordered_max)
        return ordered_keys[ordered_pos];
    return 0;
}

uint8_t getMacro(void)
{
    if (ordered_pos < ordered_max) {
        uint8_t key = ordered_keys[ordered_pos++];
        if (key)
            return key;
    }
    ordered_pos = ordered_max = 0;
    return 0;
}

void emitKey(uint8_t c)
{
    if (ordered_pos < sizeof ordered_keys)
        ordered_keys[ordered_pos++] = c;
    if (ordered_pos < sizeof ordered_keys)
        ordered_keys[ordered_pos] = 0;
}

void emitString(const uint8_t s[])
{
    uint8_t i = 0;
    uint8_t c;
    for (c = s[i]; c; c = s[++i])
        emitKey(c);
}

void emitStringN(const uint8_t s[], uint8_t len)
{
    uint8_t i = 0;
    uint8_t c;
    for (c = s[i]; i < len && c; c = s[++i])
        emitKey(c);
}

static uint8_t getNumKeycode(unsigned int n)
{
    if (n == 0)
        return KEY_0;
    if (1 <= n && n <= 9)
        return KEY_1 - 1 + n;
    return KEY_SPACEBAR;
}

#if APP_MACHINE_VALUE != 0x4550
void emitNumber(uint16_t n)
{
    int8_t zero = 0;

    for (uint16_t i = 10000;;) {
        uint8_t d = n / i;
        if (d || zero) {
            zero = 1;
            emitKey(getNumKeycode(d));
        }
        n %= i;
        i /= 10;
        if (i == 1)
            zero = 1;
        else if (i == 0)
            break;
    }
}
#endif

static const uint8_t about_title[] = {
    KEY_E, KEY_S, KEY_R, KEY_I, KEY_L, KEY_L, KEY_E, KEY_SPACEBAR, KEY_N, KEY_I, KEY_S, KEY_S, KEY_E,
    KEY_ENTER, 0
};
static const uint8_t about_rev[] = {
    KEY_R, KEY_E, KEY_V, KEY_PERIOD, KEY_SPACEBAR, 0
};
static const uint8_t about_ver[] = {
    KEY_V, KEY_E, KEY_R, KEY_PERIOD, KEY_SPACEBAR, 0
};
static const uint8_t about_copyright[] = {
    KEY_C, KEY_O, KEY_P, KEY_Y, KEY_R, KEY_I, KEY_G, KEY_H, KEY_T, KEY_SPACEBAR, KEY_2, KEY_0, KEY_1, KEY_3, KEY_MINUS, KEY_2, KEY_0, KEY_1, KEY_6, KEY_SPACEBAR,
    KEY_E, KEY_S, KEY_R, KEY_I, KEY_L, KEY_L, KEY_E, KEY_SPACEBAR, KEY_I, KEY_N, KEY_C, KEY_PERIOD, KEY_ENTER, 0
};
static const uint8_t about_f2[] = {
    KEY_F, KEY_2, KEY_SPACEBAR, 0
};
static const uint8_t about_f3[] = {
    KEY_F, KEY_3, KEY_SPACEBAR, 0
};
static const uint8_t about_f4[] = {
    KEY_F, KEY_4, KEY_SPACEBAR, 0
};
static const uint8_t about_f5[] = {
    KEY_F, KEY_5, KEY_SPACEBAR, 0
};
static const uint8_t about_f6[] = {
    KEY_F, KEY_6, KEY_SPACEBAR, 0
};
static const uint8_t about_f7[] = {
    KEY_F, KEY_7, KEY_SPACEBAR, 0
};
static const uint8_t about_f8[] = {
    KEY_F, KEY_8, KEY_SPACEBAR, 0
};
static const uint8_t about_f9[] = {
    KEY_F, KEY_9, KEY_SPACEBAR, 0
};

#ifdef WITH_HOS
static const uint8_t about_ble[] = {
    KEY_B, KEY_L, KEY_E, KEY_SPACEBAR, KEY_M, KEY_O, KEY_D, KEY_U, KEY_L, KEY_E,
    KEY_ENTER, 0
};

static const uint8_t about_kvm[] = {
    KEY_K, KEY_V, KEY_M, KEY_SPACEBAR, 0
};

static const uint8_t about_lesc[] = {
    KEY_L, KEY_E, KEY_S, KEY_C, KEY_SPACEBAR, 0
};

#endif

static void about(void)
{
    emitString(about_title);

    // REV.
    emitString(about_rev);
    emitKey(getNumKeycode(BOARD_REV_VALUE));
    emitKey(KEY_ENTER);

    // VER.
    emitString(about_ver);
    emitKey(getNumKeycode((APP_VERSION_VALUE >> 8) & 0xf));
    emitKey(KEY_PERIOD);
    emitKey(getNumKeycode((APP_VERSION_VALUE >> 4) & 0xf));
    emitKey(getNumKeycode(APP_VERSION_VALUE & 0xf));
    emitKey(KEY_ENTER);

#ifdef WITH_HOS
    emitString(about_ble);
    emitString(about_rev);
    emitKey(getNumKeycode(HosGetRevision() & 0xf));
    emitKey(KEY_ENTER);

    emitString(about_ver);
    emitKey(getNumKeycode((HosGetVersion() >> 8) & 0xf));
    emitKey(KEY_PERIOD);
    emitKey(getNumKeycode((HosGetVersion() >> 4) & 0xf));
    emitKey(getNumKeycode(HosGetVersion() & 0xf));
    emitKey(KEY_ENTER);

    emitString(about_copyright);

    emitString(about_kvm);
    emitKey(getNumKeycode(CurrentProfile()));
    emitKey(KEY_ENTER);

    if (!isUSBMode()) {
        emitString(about_lesc);
        emitKey(getNumKeycode(HosGetLESC()));
        emitKey(KEY_ENTER);
    }
#else
    emitString(about_copyright);
#endif

    // F2 OS
    emitString(about_f2);
    emitOSName();

    // F3 Layout
    emitString(about_f3);
    emitBaseName();

    // F4 Kana Layout
    emitString(about_f4);
    emitKanaName();

    // F5 Delay
    emitString(about_f5);
    emitDelayName();

    // F6 Modifiers
    emitString(about_f6);
    emitModName();

    // F7 IME
    emitString(about_f7);
    emitIMEName();

    // F8 LED
    emitString(about_f8);
    emitLEDName();

    // F9 Prefix Shift
    emitString(about_f9);
    emitPrefixShift();

#ifdef ENABLE_MOUSE
    emitMouse();
#endif

#ifdef WITH_HOS
    if (!isBusPowered()) {
        uint16_t voltage = HosGetBatteryVoltage();
        uint8_t level = HosGetBatteryLevel();
        if (HOS_BATTERY_VOLTAGE_OFFSET < voltage) {
            emitKey(getNumKeycode(voltage / 100));
            emitKey(KEY_PERIOD);
            voltage %= 100;
            emitKey(getNumKeycode(voltage / 10));
            emitKey(getNumKeycode(voltage % 10));
            emitKey(KEY_V);
            emitKey(KEY_SPACEBAR);
            emitNumber(level);
            emitKey(KEYPAD_PERCENT);
            emitKey(KEY_ENTER);
        }
    }
#endif
}

static const uint8_t* getKeyFn(uint8_t code, uint8_t matrixNumber)
{
    if (is109()) {
        if (12 * 6 + 8 <= code && code <= 12 * 6 + 11)
            return matrixFn109[code - (12 * 6 + 8)];
    }
    return matrixFnZq[matrixNumber][code / 12][code % 12];
}

#ifdef WITH_HOS
static void switchProfile(uint8_t profile)
{
    SelectProfile(profile);
    loadKeyboardSettings();
#ifdef ENABLE_MOUSE
    loadMouseSettings();
#endif
}
#endif

static int8_t processKeys(const uint8_t* current, uint8_t* processed, uint8_t* report)
{
    int8_t xmit;

    if (!memcmp(current, processed, 8))
        return XMIT_NONE;
    memset(report, 0, 8);

    if (lastExtra & MOD_FN)
        modifiersExtra |= MOD_FN;
    if (lastExtra & MOD_FN2)
        modifiersExtra |= MOD_FN2;
    /*
     * In the MOD_FN layer, some keys take on special meaning. E.g., KEY_F1
     * calls about() to rapidly type out some information about the current
     * keyboard settings.
     */
    if (current[1] & MOD_FN) {
        uint8_t modifiers = current[0];
        uint8_t count = 2;
        xmit = XMIT_NORMAL;

        for (int8_t i = 2; i < 8 && xmit != XMIT_MACRO; ++i) {
            uint8_t code = current[i];
            const uint8_t* a = getKeyFn(code, MOD_FN - 1);
            for (int8_t j = 0; j < 3 && count < 8; ++j) {
                uint8_t key = a[j];
                int8_t make = !memchr(processed + 2, code, 6);

                switch (key) {
                case 0:
                    break;
                case KEY_F1:
                    if (make) {
#ifdef WITH_HOS
                        if (current[0] & MOD_SHIFT) {
                            switchProfile(1);
                            modifiers &= ~(MOD_CONTROL | MOD_SHIFT);
                            xmit = XMIT_BRK;
                        }
                        else
#endif
                        {
                            about();
                            xmit = XMIT_MACRO;
                        }
                    }
                    break;
                case KEY_F2:
                    if (make) {
#ifdef WITH_HOS
                        if (current[0] & MOD_SHIFT) {
                            switchProfile(2);
                            modifiers &= ~(MOD_CONTROL | MOD_SHIFT);
                            xmit = XMIT_BRK;
                        }
                        else
#endif
                        {
                            switchOS();
                            xmit = XMIT_MACRO;
                        }
                    }
                    break;
                case KEY_F3:
                    if (make) {
#ifdef WITH_HOS
                        if (current[0] & MOD_SHIFT) {
                            switchProfile(3);
                            modifiers &= ~(MOD_CONTROL | MOD_SHIFT);
                            xmit = XMIT_BRK;
                        }
                        else
#endif
                        {
                            switchBase();
                            xmit = XMIT_MACRO;
                        }
                    }
                    break;
                case KEY_F4:
                    if (make) {
#ifdef WITH_HOS
                        if (current[0] & MOD_SHIFT) {
                            switchProfile(0);
                            modifiers &= ~(MOD_CONTROL | MOD_SHIFT);
                            xmit = XMIT_BRK;
                        }
                        else
#endif
                        {
                            switchKana();
                            xmit = XMIT_MACRO;
                        }
                    }
                    break;
                case KEY_F5:
                    if (make) {
                        switchDelay();
                        xmit = XMIT_MACRO;
                    }
                    break;
                case KEY_F6:
                    if (make) {
                        switchMod();
                        xmit = XMIT_MACRO;
                    }
                    break;
                case KEY_F7:
                    if (make) {
                        switchIME();
                        xmit = XMIT_MACRO;
                    }
                    break;
                case KEY_F8:
                    if (make) {
                        switchLED();
                        xmit = XMIT_MACRO;
                    }
                    break;
                case KEY_F9:
                    if (make) {
                        switchPrefixShift();
                        xmit = XMIT_MACRO;
                    }
                    break;
                case KEY_LEFTCONTROL:
                    modifiers |= MOD_LEFTCONTROL;
                    break;
                case KEY_RIGHTCONTROL:
                    modifiers |= MOD_RIGHTCONTROL;
                    break;
                case KEY_LEFTSHIFT:
                    modifiers |= MOD_LEFTSHIFT;
                    break;
                case KEY_RIGHTSHIFT:
                    modifiers |= MOD_RIGHTSHIFT;
                    break;
#ifdef WITH_HOS
                case KEY_ESCAPE:
                    if (make) {
                        if (!isUSBMode() && (current[0] & MOD_SHIFT)) {
                            HosSetEvent(HOS_TYPE_DEFAULT, HOS_EVENT_CLEAR_BONDING_DATA);
                            modifiers &= ~(MOD_CONTROL | MOD_SHIFT);
                            xmit = XMIT_BRK;
                        } else {
                            key = toggleKanaMode(key, current[0], make);
                            report[count++] = key;
                        }
                    }
                    break;
#endif
                case KEYPAD_00:
                    if (make) {
                        emitKey(KEY_0);
                        emitKey(KEY_0);
                        xmit = XMIT_MACRO;
                    }
                    break;
                case KEYPAD_000:
                    if (make) {
                        emitKey(KEY_0);
                        emitKey(KEY_0);
                        emitKey(KEY_0);
                        xmit = XMIT_MACRO;
                    }
                    break;
                case KEY_ZQ_HOMEDIR:
                    if (make) {
                        emitKey(0);
                        emitKey(KEY_ZQ_MACRO_TILDE);
                        emitKey(KEY_SLASH);
                        xmit = XMIT_IN_ORDER;
                    }
                    break;
                case KEY_ZQ_SOFTTAB2:
                    if (make) {
                        emitKey(KEY_SPACEBAR);
                        emitKey(KEY_SPACEBAR);
                        xmit = XMIT_MACRO;
                    }
                    break;
                default:
                    key = toggleKanaMode(key, current[0], make);
                    report[count++] = key;
                    break;
                }
            }
        }
        report[0] = modifiers;
    } else if (current[1] & MOD_FN2) {
        uint8_t modifiers = current[0];
        uint8_t count = 2;
        xmit = XMIT_NORMAL;
        for (int8_t i = 2; i < 8 && xmit != XMIT_MACRO; ++i) {
            uint8_t code = current[i];
            const uint8_t* a = getKeyFn(code, MOD_FN2 - 1);
            for (int8_t j = 0; j < 3 && count < 8; ++j) {
                uint8_t key = a[j];
                int8_t make = !memchr(processed + 2, code, 6);
                switch (key) {
                case 0:
                    break;
                case KEY_LEFTCONTROL:
                    modifiers |= MOD_LEFTCONTROL;
                    break;
                case KEY_RIGHTCONTROL:
                    modifiers |= MOD_RIGHTCONTROL;
                    break;
                case KEY_LEFTSHIFT:
                    modifiers |= MOD_LEFTSHIFT;
                    break;
                case KEY_RIGHTSHIFT:
                    modifiers |= MOD_RIGHTSHIFT;
                    break;
                default:
                    key = toggleKanaMode(key, current[0], make);
                    report[count++] = key;
                    break;
                }
            }
        }
#ifdef WITH_HOS
        if (count == 2) {
            modifiers &= ~MOD_SHIFT;
        }
#endif
        report[0] = modifiers;
    } else if (isKanaMode(current))
        xmit = processKeysKana(current, processed, report);
    else
        xmit = processKeysBase(current, processed, report);

#ifdef ENABLE_DUAL_ROLE_FN
    if (isDualRoleFnMod()) {
        if ((current[1] ^ processed[1]) & MOD_FN) {
            modFn = (current[1] & MOD_FN);
            if (modFn) {
                dualFn = modFn;
            } else if (dualFn && xmit == XMIT_NORMAL && !report[2]) {
                uint8_t key = (dualFn & MOD_RIGHTFN) ? KEY_LANG1 : KEY_LANG2;
                key = toggleKanaMode(key, current[0], 1);
                report[2] = key;
                memmove(processed, current, 8);
                processed[1] |= dualFn;
                dualFn = 0;
                return xmit;
            }
        }
        if (dualFn && (xmit != XMIT_NORMAL || report[2])) {
            dualFn = 0;
        }
    }
#endif

    if (xmit == XMIT_NORMAL || xmit == XMIT_IN_ORDER || xmit == XMIT_MACRO)
        memmove(processed, current, 8);

    return xmit;
}

static void processOSMode(uint8_t* report)
{
    for (int8_t i = 2; i < 8; ++i) {
        uint8_t key = report[i];
        switch (os) {
        case OS_PC:
            switch (key) {
            case KEY_LANG1:
                report[i] = KEY_F13;
                break;
            case KEY_LANG2:
                report[i] = KEY_F14;
                break;
            case KEY_INTERNATIONAL4:
            case KEY_INTERNATIONAL5:
                report[i] = KEY_SPACEBAR;
                break;
            default:
                break;
            }
            break;
        case OS_MAC:
            switch (key) {
            case KEY_INTERNATIONAL4:
            case KEY_INTERNATIONAL5:
                report[i] = KEY_SPACEBAR;
                break;
            case KEY_APPLICATION:
                if (isMacMod()) {
                    report[0] |= MOD_LEFTALT;
                    memmove(report + i, report + i + 1, 7 - i);
                    report[7] = 0;
                    --i;
                }
                break;
#ifdef WITH_HOS
            case KEYPAD_ENTER:
                report[i] = KEY_ENTER;
                break;
#endif
            default:
                break;
            }
            break;
        case OS_104A:
            switch (key) {
            case KEY_LANG1:
                report[i] = KEY_SPACEBAR;
                report[0] |= MOD_LEFTSHIFT | MOD_LEFTCONTROL;
                break;
            case KEY_LANG2:
                report[i] = KEY_BACKSPACE;
                report[0] |= MOD_LEFTSHIFT | MOD_LEFTCONTROL;
                break;
            case KEY_INTERNATIONAL4:
            case KEY_INTERNATIONAL5:
                report[i] = KEY_SPACEBAR;
                break;
            default:
                break;
            }
            break;
        case OS_104B:
            switch (key) {
            case KEY_LANG1:
            case KEY_LANG2:
                report[i] = KEY_GRAVE_ACCENT;
                report[0] |= MOD_LEFTALT;
                break;
            case KEY_INTERNATIONAL4:
            case KEY_INTERNATIONAL5:
                report[i] = KEY_SPACEBAR;
                break;
            default:
                break;
            }
            break;
        case OS_109A:
            switch (key) {
            case KEY_LANG1:
                report[i] = KEY_INTERNATIONAL4;
                report[0] |= MOD_LEFTSHIFT | MOD_LEFTCONTROL;
                break;
            case KEY_LANG2:
                report[i] = KEY_INTERNATIONAL5;
                report[0] |= MOD_LEFTSHIFT | MOD_LEFTCONTROL;
                break;
            default:
                break;
            }
            break;
        case OS_109B:
            switch (key) {
            case KEY_LANG1:
            case KEY_LANG2:
                report[i] = KEY_GRAVE_ACCENT;
                break;
            default:
                break;
            }
            break;
        case OS_ALT_SP:
            switch (key) {
            case KEY_LANG1:
            case KEY_LANG2:
                report[i] = KEY_SPACEBAR;
                report[0] |= MOD_LEFTALT;
                break;
            default:
                break;
            }
            break;
        case OS_SHIFT_SP:
            switch (key) {
            case KEY_LANG1:
            case KEY_LANG2:
                report[i] = KEY_SPACEBAR;
                report[0] |= MOD_LEFTSHIFT;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}

uint8_t processModKey(uint8_t key)
{
    const uint8_t* map = modMap[0];
    uint8_t xfer = mod;
#ifdef ENABLE_DUAL_ROLE_FN
    switch (mod) {
    case MOD_CX:
        xfer = MOD_C;
        break;
    case MOD_SX:
        xfer = MOD_S;
        break;
    default:
        break;
    }
#endif
    for (int8_t i = 0; i < MAX_MOD_KEYS; ++i) {
        if (key == map[i])
            return modMap[xfer][i];
    }
    return key;
}

int8_t makeReport(uint8_t* report)
{
    int8_t xmit = XMIT_NONE;
    int8_t at;
    int8_t prev;

    if (!detectGhost()) {
        while (count < 8)
            current[count++] = VOID_KEY;
        memmove(keys[currentKey].keys, current + 2, 6);
        current[0] = modifiers;
//        if (led & LED_SCROLL_LOCK)
//            current[1] |= MOD_LEFTFN;
        current[1] = modifiersExtra;
#ifdef ENABLE_MOUSE
        if (isMouseTouched())
            current[1] |= MOD_PAD;
#endif

        if ((prefix_shift && isKanaMode(current)) || isZQMode(current)) {
            current[0] |= prefix;
            current[1] |= prefixExtra;
            if (!(modifiersPrev & MOD_LEFTSHIFT) && (modifiers & MOD_LEFTSHIFT))
                prefix ^= MOD_LEFTSHIFT;
            if (!(modifiersPrev & MOD_RIGHTSHIFT) && (modifiers & MOD_RIGHTSHIFT))
                prefix ^= MOD_RIGHTSHIFT;
            if (!(modifiersExtraPrev & MOD_FN) && (modifiersExtra & MOD_FN))
                prefixExtra ^= MOD_FN;
            if (!(modifiersExtraPrev & MOD_FN2) && (modifiersExtra & MOD_FN2))
                prefixExtra ^= MOD_FN2;
        }
        modifiersPrev = modifiers;
        modifiersExtraPrev = modifiersExtra;

        // Copy keys that exist in both keys[prev] and keys[at] for debouncing.
        at = currentKey + DELAY_MAX + 2 - currentDelay;
        if (DELAY_MAX + 1 < at)
                at -= DELAY_MAX + 2;
        prev = at + DELAY_MAX + 1;
        if (DELAY_MAX + 1 < prev)
                prev -= DELAY_MAX + 2;
        count = 2;
        for (int8_t i = 0; i < 6; ++i) {
            uint8_t key = keys[at].keys[i];
            if (memchr(keys[prev].keys, key, 6))
                current[count++] = key;
        }
        while (count < 8)
            current[count++] = VOID_KEY;

#ifdef ENABLE_MOUSE
        if (current[1] == MOD_PAD)
            processMouseKeys(current, processed);
#endif

        if (memcmp(current, processed, 8)) {
            if (memcmp(current + 2, processed + 2, 6) || current[2] == VOID_KEY || current[1] || (current[0] & MOD_SHIFT)) {
                if (current[2] != VOID_KEY) {
                    prefix = 0;
                    prefixExtra = 0;
                }
                xmit = processKeys(current, processed, report);
            } else if (processed[1] && !current[1] ||
                     (processed[0] & MOD_LEFTSHIFT) && !(current[0] & MOD_LEFTSHIFT) ||
                     (processed[0] & MOD_RIGHTSHIFT) && !(current[0] & MOD_RIGHTSHIFT))
            {
                /* empty */
            } else
                xmit = processKeys(current, processed, report);
        }

        processOSMode(report);
    } else {
        prev = currentKey + DELAY_MAX + 1;
        if (DELAY_MAX + 1 < prev)
                prev -= DELAY_MAX + 2;
        memmove(keys[currentKey].keys, keys[prev].keys, 6);
    }

    if (DELAY_MAX + 1 < ++currentKey)
        currentKey = 0;
    count = 2;
    modifiers = 0;
    modifiersExtra = 0;

    return xmit;
}

uint8_t controlLED(uint8_t report)
{
    led = report;
    report = controlKanaLED(report);
    report = controlZQLED(report);
#ifdef ENABLE_MOUSE
    if (isMouseTouched())
        report |= LED_SCROLL_LOCK;
#endif
    if (BOARD_REV_VALUE < 3) {
        static int8_t tick;

        if (4 <= ++tick)
            tick = 0;
        else
            report &= ~LED_USB_DEVICE_HID_KEYBOARD_CAPS_LOCK;
    }
    return report;
}

uint8_t getKeyNumLock(uint8_t code)
{
    uint8_t col = code % 12;

    if ((led & LED_NUM_LOCK) && 7 <= col) {
        col -= 7;
        return matrixNumLock[code / 12][col];
    }
    return 0;
}
