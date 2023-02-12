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
#include <ws.h>
#include "bootfriend.h"
#include "boot_splash.h"
#include "font_default.h"
#include "input.h"
#include "nanoprintf.h"
#include "ui.h"
#include "util.h"
#include "wonderful-asm-common.h"
#include "ws/display.h"
#include "ws/eeprom.h"

volatile uint16_t vbl_ticks;

__attribute__((interrupt))
void vblank_int_handler(void) {
	vbl_ticks++;
	vblank_input_update();
	ws_hwint_ack(HWINT_VBLANK);
}

static const char __far bfi_title[] = "BootFriend Installer v. %02X";
static const char __far bfi_eeprom_locked[] = "EEP locked";
static const char __far bfi_eeprom_unlocked[] = "EEP unlocked";
static const char __far bfi_no_splash[] = "no splash";
static const char __far bfi_invalid_splash[] = "invalid splash";
static const char __far bfi_no_bf[] = "non-BF splash";
static const char __far bfi_bf_found[] = "BF v.%02X";

ws_boot_splash_header_t boot_header_data;
static bool boot_header_update_required;
static bool boot_header_splash_valid;

void boot_header_mark_changed(void) {
	boot_header_update_required = true;
}

void boot_header_refresh(void) {
	if (!boot_header_update_required) return;
	ws_eeprom_read_data(ws_eeprom_handle_internal(), 0x80, (uint8_t*) &boot_header_data, sizeof(ws_boot_splash_header_t));
	boot_header_splash_valid = ws_boot_splash_is_header_valid(&boot_header_data);
	boot_header_update_required = false;
}

static void statusbar_update(void) {
	boot_header_refresh();
	char buf[29];

	SCREEN1[0] = SCR_ENTRY_PALETTE(COLOR_TITLE);
	SCREEN1[27] = SCR_ENTRY_PALETTE(COLOR_TITLE);
	ui_printf(1, 0, COLOR_TITLE, bfi_title, 0);

	ui_clear_lines(1, 1);

	const char __far *eeprom_status = ws_ieep_protect_check() ? bfi_eeprom_locked : bfi_eeprom_unlocked;
	ui_puts(0, 1, COLOR_BLACK, eeprom_status);

	// detect BootFriend
	bool splash_active = boot_header_data.options1 & IEEP_C_OPTIONS1_CUSTOM_SPLASH;
	bool splash_bf = boot_header_data.pad5 == 'b' && boot_header_data.pad6 == 'F';
	bool splash_valid = boot_header_splash_valid;
	if (!splash_valid && !splash_active) {
		strcpy(buf, bfi_no_splash);
	} else if (!splash_valid && splash_active) {
		strcpy(buf, bfi_invalid_splash);
	} else if (splash_valid && !splash_bf) {
		strcpy(buf, bfi_no_bf);
	} else if (splash_valid && splash_bf) {
		npf_snprintf(buf, sizeof(buf), bfi_bf_found, boot_header_data.pad4);
	} else {
		buf[0] = '?'; buf[1] = 0;
	}
	ui_puts(28 - strlen(buf), 1, splash_active ? COLOR_BLACK : COLOR_GRAY, buf);
}

static void toggle_boot_splash(void) {
	uint16_t word_0x82 = ws_eeprom_read_word(ws_eeprom_handle_internal(), 0x82);
	word_0x82 ^= (IEEP_C_OPTIONS1_CUSTOM_SPLASH << 8);
	ws_eeprom_write_word(ws_eeprom_handle_internal(), 0x82, word_0x82);

	boot_header_mark_changed();
	statusbar_update();
}

extern const char __far ws_ieep_internal_owner_to_ascii_map[];
extern void bootfriend_vblank_handler(void) __attribute__((interrupt));

static void test_bootfriend(void) {
	// copy BootFriend to IRAM
	memcpy((uint8_t*) 0x6000, _bootfriend_bin, _bootfriend_bin_size);

	uint8_t vbl_bootstrap[12];
	vbl_bootstrap[0] = 0x50; // PUSH AX
	vbl_bootstrap[1] = 0x9A; // CALL far
	vbl_bootstrap[2] = _bootfriend_bin[0x18];
	vbl_bootstrap[3] = _bootfriend_bin[0x19];
	vbl_bootstrap[4] = _bootfriend_bin[0x1A];
	vbl_bootstrap[5] = _bootfriend_bin[0x1B];
	vbl_bootstrap[6] = 0xB0; // MOV AL,
	vbl_bootstrap[7] = 0xFF; // [ack value]
	vbl_bootstrap[8] = 0xE6; // OUT IO_HWINT_ACK, AL
	vbl_bootstrap[9] = 0xB6;
	vbl_bootstrap[10] = 0x58; // OP AX
	vbl_bootstrap[11] = 0xCF; // IRET

	// prepare UI
    outportb(IO_SCR_BASE, SCR1_BASE(0x0800));
	for (uint8_t i = 0; i <= 42; i++) {
		const uint8_t __far *src = _font_default_bin + (ws_ieep_internal_owner_to_ascii_map[i] * 8);
		uint16_t *dst = (uint16_t*) (0x2000 + (i * 16));
		for (int i = 0; i < _font_default_bin_size; i++) {
			*(dst++) = *(src++);
		}
	}

	cpu_irq_disable();
	ws_hwint_set_handler(HWINT_IDX_VBLANK, vbl_bootstrap);
	ws_hwint_enable(HWINT_VBLANK);
	cpu_irq_enable();

	while (true) {
		cpu_halt();
		cpu_irq_disable();
		uint16_t keys = ws_keypad_scan();
		cpu_irq_enable();
		if (keys & KEY_B) break;
	}

	cpu_irq_disable();
	ws_hwint_set_handler(HWINT_IDX_VBLANK, vblank_int_handler);
	ws_hwint_enable(HWINT_VBLANK);
	cpu_irq_enable();

	// restore UI
	ui_init();
}

static const char __far msg_are_you_sure_install[] = "THIS SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND.\n\nWould you like to install BootFriend?";
static const char __far msg_installing_eeprom_data[] = "Installing IEEPROM data...";
static const char __far msg_verifying_eeprom_data[] =  "Verifying IEEPROM data....";
static const char __far msg_verify_error[] =  "Verify error @ %03X";
static const char __far msg_do_not_turn_off[] = "Do not turn off the console!";

static void install_bootfriend(void) {
	ws_eeprom_handle_t ieep_handle = ws_eeprom_handle_internal();

	ui_puts(1, 3, COLOR_BLACK, msg_installing_eeprom_data);
	ui_puts(0, 5, COLOR_RED, msg_do_not_turn_off);

	cpu_irq_disable();

	// Disable the custom splash, if enabled.
	uint16_t word_0x82 = ws_eeprom_read_word(ieep_handle, 0x82);
	if (word_0x82 & (IEEP_C_OPTIONS1_CUSTOM_SPLASH << 8)) {
		word_0x82 ^= (IEEP_C_OPTIONS1_CUSTOM_SPLASH << 8);
		ws_eeprom_write_word(ieep_handle, 0x82, word_0x82);
	}

	// Write BootFriend data; skip sensitive/user-configurable areas
	uint16_t word_0x84 = ws_eeprom_read_word(ieep_handle, 0x84);
	word_0x84 = (word_0x84 & 0xFF) | (_bootfriend_bin[5] << 8);
	ws_eeprom_write_word(ieep_handle, 0x84, word_0x84);

	uint16_t steps_per_progress = (_bootfriend_bin_size - 6) / (26 * 2);
	uint8_t step_counter = 0;
	uint16_t step_counter_min = 0;

	for (uint16_t i = 0x06; i < _bootfriend_bin_size; i += 2) {
		uint16_t w = _bootfriend_bin[i] | (_bootfriend_bin[i + 1] << 8);
		uint16_t w2 = ws_eeprom_read_word(ieep_handle, i + 0x80);
		// skip SwanCrystal data block
		if (!(i >= 0x2C && i < 0x38)) {
			if (w != w2) {
				ws_eeprom_write_word(ieep_handle, i + 0x80, w);
			}
		}
		
		if (step_counter < 26 && (++step_counter_min) == steps_per_progress) {
			ws_screen_put(SCREEN1, SCR_ENTRY_PALETTE(COLOR_SELECTED), 1 + (step_counter++), 15);
			step_counter_min = 0;
		}
	}

	// Verify read.
	ui_puts(1, 3, COLOR_BLACK, msg_verifying_eeprom_data);
	ui_clear_lines(15, 15);

	step_counter = 0;
	step_counter_min = 0;

	for (uint16_t i = 0x06; i < _bootfriend_bin_size; i += 2) {
		uint16_t w = _bootfriend_bin[i] | (_bootfriend_bin[i + 1] << 8);
		// skip SwanCrystal data block
		if (!(i >= 0x2C && i < 0x38)) {
			uint16_t w2 = ws_eeprom_read_word(ieep_handle, i + 0x80);
			if (w != w2) {
				ui_clear_lines(15, 15);
				
				ui_printf(1, 15, COLOR_RED, msg_verify_error, i);
				cpu_irq_enable();
				input_wait_clear();
				while (input_pressed == 0) {
					wait_for_vblank(); input_update();
				}
				input_wait_clear();

				goto EndInstall;
				return;
			}
		}
		
		if (step_counter < 26 && (++step_counter_min) == steps_per_progress) {
			ws_screen_put(SCREEN1, SCR_ENTRY_PALETTE(COLOR_SELECTED), 1 + (step_counter++), 15);
			step_counter_min = 0;
		}
	}

	// Enable the custom splash.
	word_0x82 |= (IEEP_C_OPTIONS1_CUSTOM_SPLASH << 8);
	ws_eeprom_write_word(ieep_handle, 0x82, word_0x82);
	
EndInstall:
	cpu_irq_enable();
	ui_clear_lines(3, 16);

	boot_header_mark_changed();
	statusbar_update();
}

static const char __far msg_are_you_sure_recovery[] = "THIS SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND.\n\nThis option tries to restore factory TFT configuration for SwanCrystal consoles.\n\nWould you like to continue?";
static const uint8_t __far swancrystal_factory_tft_data[] = {
	0xD0, 0x77, 0xF7, 0x06, 0xE2, 0x0A, 0xEA, 0xEE
};

static void recovery_swancrystal(void) {
	ws_eeprom_handle_t ieep_handle = ws_eeprom_handle_internal();

	for (uint8_t i = 0; i < 8; i += 2) {
		uint16_t w = *((uint16_t*) (swancrystal_factory_tft_data + i));
		uint16_t w2 = ws_eeprom_read_word(ieep_handle, 0xAE + i);
		if (w != w2) {
			ws_eeprom_write_word(ieep_handle, 0xAE + i, w);
		}
	}

	boot_header_mark_changed();
	statusbar_update();
}

static const char __far msg_are_you_sure[] = "Are you sure?";
static const char __far msg_yes[] = "Yes";
static const char __far msg_no[] = "No";

bool menu_confirm(const char __far *text, uint8_t text_height, bool centered) {
	menu_entry_t entries[2];
	uint8_t height = text_height + 3;
	uint8_t y_text = 3 + ((14 - height) >> 1);
	uint8_t y_menu = y_text + text_height + 1;

	ui_puts(centered ? (28 - strlen(text)) >> 1 : 0, y_text, 0, text);

	entries[0].text = msg_no;  entries[0].flags = 0;
	entries[1].text = msg_yes; entries[1].flags = 0;
	uint8_t result = ui_menu_run(entries, 2, y_menu);

	ui_clear_lines(y_text, y_text + text_height - 1);
	return result == 1;
}

static const char __far msg_test_bootfriend[] = "Test BootFriend";
static const char __far msg_install_bootfriend[] = "Install BootFriend";
static const char __far msg_disable_splash[] = "Disable boot splash";
static const char __far msg_enable_splash[] = "Enable boot splash";
static const char __far msg_recover_swancrystal[] = "SwanCrystal TFT recovery";

uint8_t menu_show_main(void) {
	boot_header_refresh();
	bool splash_active = boot_header_data.options1 & IEEP_C_OPTIONS1_CUSTOM_SPLASH;
	menu_entry_t entries[4];
	uint8_t entry_count = 0;

	entries[entry_count].text = msg_test_bootfriend;
	entries[entry_count++].flags = 0;
	entries[entry_count].text = msg_install_bootfriend;
	entries[entry_count++].flags = ws_ieep_protect_check() ? MENU_ENTRY_DISABLED : 0; 
	entries[entry_count].text = splash_active ? msg_disable_splash : msg_enable_splash;
	entries[entry_count++].flags = (ws_ieep_protect_check() || (!splash_active && !boot_header_splash_valid)) ? MENU_ENTRY_DISABLED : 0;
	entries[entry_count].text = msg_recover_swancrystal;
	entries[entry_count++].flags = ws_ieep_protect_check() ? MENU_ENTRY_DISABLED : 0; 

	ui_menu_run(entries, entry_count, 3 + ((14 - entry_count) >> 1));
}

void menu_main(void) {
	input_wait_clear();
	switch (menu_show_main()) {
	case 0: // Test BootFriend	
		test_bootfriend();
		break;
	case 1: // Install BootFriend
		if (menu_confirm(msg_are_you_sure_install, 6, false)) install_bootfriend();
		break;
	case 2: // Disable/Enable boot splash
		if (menu_confirm(msg_are_you_sure, 1, true)) toggle_boot_splash();
		break;
	case 3: // SwanCrystal recovery
		if (menu_confirm(msg_are_you_sure_recovery, 9, false)) recovery_swancrystal();
		break;
	}
}

int main(void) {
	cpu_irq_disable();

	boot_header_mark_changed();
	ui_init();
	statusbar_update();

	ws_hwint_set_handler(HWINT_IDX_VBLANK, vblank_int_handler);
	ws_hwint_enable(HWINT_VBLANK);
	cpu_irq_enable();

	while(1) {
		menu_main();
	}
}