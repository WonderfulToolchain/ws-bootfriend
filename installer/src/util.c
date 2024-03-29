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
#ifdef __WONDERFUL_WWITCH__
#include <sys/bios.h>
#else
#include <ws.h>
#endif
#include "util.h"

// it's an uint16_t but we only want the low byte
extern volatile uint8_t vbl_ticks;

void wait_for_vblank(void) {
#ifndef __WONDERFUL_WWITCH__
        uint8_t vbl_ticks_last = vbl_ticks;

        while (vbl_ticks == vbl_ticks_last) {
                cpu_halt();
        }
#else
        sys_wait(1);
#endif
}
