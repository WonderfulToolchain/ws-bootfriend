/**
 * Copyright (c) 2022, 2023 Adrian Siekierka
 *
 * BootFriend is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * BootFriend is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with BootFriend. If not, see <https://www.gnu.org/licenses/>. 
 */

#include <stdbool.h>
#include <string.h>
#include <ws.h>
#include "input.h"
#include "ui.h"
#include "font_default.h"
#include "nanoprintf.h"
#include "util.h"
#include "ws/display.h"

const char __far msg_wsc_only[] = "The BootFriend installer is compatible only with Wonder-Swan Color and SwanCrystal consoles! Shutting down.";

static bool ui_is_space(char c) {
    return c == 0 || c == '\n' || c == 32;
}

static bool ui_is_divider(char c) {
    return ui_is_space(c) || c == '-';
}

void ui_clear_lines(uint8_t y_from, uint8_t y_to) {
    for (uint8_t i = y_from; i <= y_to; i++) {
        memset(SCREEN1 + (i << 5), 0, 28 * 2);
    }
}

void ui_puts(uint8_t x, uint8_t y, uint8_t color, const char __far* buf) {
    uint16_t prefix = SCR_ENTRY_PALETTE(color & 0x0F);

    const uint8_t __far* ptr = (const uint8_t __far*) buf;
    while (*ptr != 0) {
        uint8_t chars = 1;
        if (*ptr == '\n') { ptr++; goto NewLine; }
        if (ui_is_space(*ptr) && x == 0) { ptr++; continue; }
        while (!ui_is_divider(ptr[chars])) chars++;
        if (!ui_is_space(ptr[chars])) chars++;
        if ((x + chars) > 28) {
NewLine:
            x = 0; y++;
            if (chars > 28) chars = 28;
            continue;
        }
        uint16_t *dest = SCREEN1 + (((uint16_t) y) << 5) + x;
        for (uint8_t i = 0; i < chars; i++) {
            *(dest++) = prefix | *(ptr++);
        }
        x += chars;
    }
}

void ui_printf(uint8_t x, uint8_t y, uint8_t color, const char __far* format, ...) {
    char buf[128];
    va_list val;
    va_start(val, format);
    npf_vsnprintf(buf, sizeof(buf), format, val);
    va_end(val);

    ui_puts(x, y, color, buf);
}

void ui_init(void) {
    const uint8_t __far *src = _font_default_bin;
    uint16_t *dst = (uint16_t*) 0x2000;
    for (int i = 0; i < _font_default_bin_size; i++) {
        *(dst++) = *(src++);
    }

    ws_display_set_shade_lut(SHADE_LUT_DEFAULT);
    outportw(0x20, 0x5270);
    outportb(IO_SCR_BASE, SCR1_BASE(0x1800));
    ui_clear_lines(0, 17);
    outportw(IO_DISPLAY_CTRL, DISPLAY_SCR1_ENABLE);
    
    if (!ws_system_is_color()) {
        // Halt on mono WS units
        ui_puts(0, ((18-4)/2)-1, 0, msg_wsc_only);
        while(1) cpu_halt();
    }

    ws_mode_set(WS_MODE_COLOR);

    // set palettes
    MEM_COLOR_PALETTE(COLOR_BLACK)[0] = 0x0FFF;
    MEM_COLOR_PALETTE(COLOR_BLACK)[1] = 0x0000;
    MEM_COLOR_PALETTE(COLOR_GRAY)[0] = 0x0FFF;
    MEM_COLOR_PALETTE(COLOR_GRAY)[1] = 0x0BBB;
    MEM_COLOR_PALETTE(COLOR_RED)[0] = 0x0FFF;
    MEM_COLOR_PALETTE(COLOR_RED)[1] = 0x0F00;
    MEM_COLOR_PALETTE(COLOR_SELECTED)[0] = 0x0000;
    MEM_COLOR_PALETTE(COLOR_SELECTED)[1] = 0x0FFF;
    MEM_COLOR_PALETTE(COLOR_TITLE)[0] = 0x0842;
    MEM_COLOR_PALETTE(COLOR_TITLE)[1] = 0x0FD4;
}

static void ui_menu_draw_entry(menu_entry_t *entry, uint8_t y, bool selected) {
    uint8_t color = selected ? COLOR_SELECTED : (entry->flags & MENU_ENTRY_DISABLED ? COLOR_GRAY : COLOR_BLACK);
    uint16_t prefix = SCR_ENTRY_PALETTE(color);
    ws_screen_fill(SCREEN1, SCR_ENTRY_PALETTE(color), 0, y, 28, 1);
    ui_puts((28 - strlen(entry->text)) >> 1, y, color, entry->text);
}

uint8_t ui_menu_run(menu_entry_t *entries, uint8_t entry_count, uint8_t y) {
    uint8_t curr_entry = 0;
    while (curr_entry < entry_count && (entries[curr_entry].flags & MENU_ENTRY_DISABLED)) curr_entry++;
    if (curr_entry >= entry_count) return 0xFF;

    // draw all menu entries
    for (uint8_t i = 0; i < entry_count; i++) {
        ui_menu_draw_entry(entries + i, y + i, i == curr_entry);
    }

    while (true) {
        wait_for_vblank();
        input_update();
        int new_entry = curr_entry;

        if (input_pressed & KEY_A) {
            input_wait_clear();
	        ui_clear_lines(y, y + entry_count - 1);
            return curr_entry;
        } else if (input_pressed & KEY_UP) {
            new_entry = curr_entry - 1;
            while (new_entry >= 0 && (entries[new_entry].flags & MENU_ENTRY_DISABLED)) new_entry--;
        } else if (input_pressed & KEY_DOWN) {
            new_entry = curr_entry + 1;
            while (new_entry < entry_count && (entries[new_entry].flags & MENU_ENTRY_DISABLED)) new_entry++;
        }

        if (new_entry != curr_entry && new_entry >= 0 && new_entry < entry_count) {
            ui_menu_draw_entry(entries + curr_entry, y + curr_entry, false);
            curr_entry = new_entry;
            ui_menu_draw_entry(entries + curr_entry, y + curr_entry, true);
        }
    }
}