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

#include <wonderful.h>
#include "input.h"
#include "util.h"

#ifdef __WONDERFUL_WWITCH__
#include <sys/bios.h>

uint16_t input_pressed;

void input_update(void) {
	input_pressed = key_hit_check_with_repeat();
}

void input_wait_clear(void) {
	while (key_press_check() != 0) sys_wait(1);
	input_pressed = 0;
}
#else
#include <ws.h>

// it's an uint16_t but we only want the low byte
extern volatile uint8_t vbl_ticks;

uint16_t input_keys = 0;
uint16_t input_keys_repressed = 0;
uint16_t input_keys_released = 0;
uint16_t input_pressed, input_held;

void vblank_input_update(void) {
	uint16_t keys = ws_keypad_scan();
	input_keys |= keys;
	input_keys_repressed |= (keys & input_keys_released);
	input_keys_released |= (input_held & (~keys));
}

void input_reset(void) {
	cpu_irq_disable();
	input_keys = 0;
	input_keys_repressed = 0;
	input_keys_released = 0;
	cpu_irq_enable();
}

#define JOY_REPEAT_DELAY 15
#define JOY_REPEAT_DELAY_NEXT 3

static uint8_t input_vbls_next[11];

void input_update(void) {
	uint16_t keys_pressed;
	uint16_t keys_repressed;
	uint16_t keys_released;

	cpu_irq_disable();
	keys_pressed = input_keys;
	keys_repressed = input_keys_repressed;
	keys_released = input_keys_released;

	input_pressed = 0;
	uint16_t input_mask = 2;
	for (uint8_t i = 0; i < 11; i++, input_mask <<= 1) {
		if (keys_pressed & input_mask) {
			if (keys_repressed & input_mask) {
				goto KeyRepressed;
			} else if (input_held & input_mask) {
				if (keys_released & input_mask) {
					input_held &= ~input_mask;
				}
				if (((uint8_t) (input_vbls_next[i] - vbl_ticks)) < 0x80) continue;
				if (!(keys_released & input_mask)) {
					input_pressed |= input_mask;
					input_vbls_next[i] = vbl_ticks + JOY_REPEAT_DELAY_NEXT;
				}
			} else {
				if (!(keys_released & input_mask)) {
KeyRepressed:
					input_pressed |= input_mask;
					input_held |= input_mask;
					input_vbls_next[i] = vbl_ticks + JOY_REPEAT_DELAY;
				}
			}
			break;
		} else {
			input_held &= ~input_mask;
		}
	}

	input_reset();
}

void input_wait_clear(void) {
	do {
		input_reset();
		wait_for_vblank();
	} while (input_keys != 0);
	input_update();
}
#endif

void wait_for_keypress(void) {
	input_wait_clear(); while (input_pressed == 0) { wait_for_vblank(); input_update(); } input_wait_clear();
}
