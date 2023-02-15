/**
 * BootFriend - boot splash helper code
 *
 * Copyright (c) 2022 Adrian "asie" Siekierka
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include <ws.h>
#include "boot_splash.h"

/**
 * @brief Try to detect if a valid boot splash is present, even if it's disabled.
 * 
 * @param data 
 * @return true 
 * @return false 
 */
bool ws_boot_splash_is_header_valid(ws_boot_splash_header_t __far* header) {
    // Heuristics
    if (header->size >= 2) return false;
    if (header->start_frame > header->end_frame) return false;
    if (header->sprite_count > 128) return false;
    if (header->tile_count > (0x760 / 8)) return false;

    uint32_t vblank_address = ((header->vblank_code_segment << 4) + header->vblank_code_offset) & 0x000FFFFF;
    if (vblank_address < 0x0800) return false;

    return true;
}
