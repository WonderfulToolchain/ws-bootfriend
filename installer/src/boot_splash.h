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

#ifndef __BOOT_SPLASH_H__
#define __BOOT_SPLASH_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t pad1, pad2, pad3; // 0
    uint8_t options1; // 3
    uint8_t name_color; // 4
    uint8_t pad4; // 5
    uint8_t size; // 6
    uint8_t start_frame; // 7
    uint8_t end_frame; // 8
    uint8_t sprite_count; // 9
    uint8_t palette_flags; // A
    uint8_t tile_count; // B
    uint16_t palette_offset; // C
    uint16_t tile_offset; // E
    uint16_t map_offset; // 10
    uint16_t screen_dest_horizontal; // 12
    uint16_t screen_dest_vertical; // 14
    uint8_t map_width; // 16
    uint8_t map_height; // 17
    uint16_t vblank_code_offset; // 18
    uint16_t vblank_code_segment; // 1A
    uint8_t name_x_horizontal; // 1C
    uint8_t name_y_horizontal; // 1D
    uint8_t name_x_vertical; // 1E
    uint8_t name_y_vertical; // 1F
    uint8_t pad5, pad6; // 20
    uint16_t sound_wavetable_offset; // 22
    uint16_t sound_channel_offset[4]; // 24
    uint8_t swancrystal_init_data[10]; // 2C
    // 36
} ws_boot_splash_header_t;

typedef struct {
    union {
        ws_boot_splash_header_t header;
        uint8_t data[0];
    };
} ws_boot_splash_t;

#define BOOT_SPLASH_SIZE_445 0x00
#define BOOT_SPLASH_SIZE_957 0x01

#define BOOT_SPLASH_PALETTE_1BPP 0x00
#define BOOT_SPLASH_PALETTE_2BPP 0x80

bool ws_boot_splash_is_header_valid(ws_boot_splash_header_t *header);

#endif /* __BOOT_SPLASH_H__ */
