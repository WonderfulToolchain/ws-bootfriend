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

#ifndef __UI_H__
#define __UI_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <wonderful.h>

#ifdef __WONDERFUL_WWITCH__
#define SCREEN1 ((uint16_t __far*) MK_FP(0x0000, 0x1800))
#else
#define SCREEN1 ((uint16_t*) 0x1800)
#endif

#define COLOR_BLACK 0
#define COLOR_GRAY 1
#define COLOR_RED 4
#define COLOR_SELECTED 10
#define COLOR_TITLE 11

void ui_init(void);
void ui_clear_lines(uint8_t y_from, uint8_t y_to);
void ui_puts(uint8_t x, uint8_t y, uint8_t color, const char __far* buf);
static inline void ui_puts_centered(uint8_t y, uint8_t color, const char __far* buf) {
    ui_puts((28 - strlen(buf)) >> 1, y, color, buf);
}
void ui_printf(uint8_t x, uint8_t y, uint8_t color, const char __far* format, ...);

#define MENU_ENTRY_DISABLED 0x0001

typedef struct {
    const char __far *text;
    uint16_t flags;
} menu_entry_t;

uint8_t ui_menu_run(menu_entry_t __far* entries, uint8_t entry_count, uint8_t y);

#endif /* __UI_H__ */
