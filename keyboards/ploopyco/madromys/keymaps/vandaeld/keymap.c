/* Copyright 2020 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
 * Copyright 2019 Sunjun Kim
 * Copyright 2020 Ploopy Corporation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H

#include "timer.h"
#include <stdlib.h>  // abs()

enum custom_keycodes {
    SCRL = QK_KB_1,   // hold: drag-scroll (both axes)
};

static bool scroll_active = false;

#ifndef SCROLL_DIVISOR_V
#    define SCROLL_DIVISOR_V 64.0
#endif
#ifndef SCROLL_DIVISOR_H
#    define SCROLL_DIVISOR_H 64.0
#endif
static float scroll_acc_v = 0;
static float scroll_acc_h = 0;

// Time to ignore movement right after activating drag-scroll,
// so click-jitter doesn't lock the OS into the wrong axis.
#ifndef SCROLL_LOCKOUT_MS
#    define SCROLL_LOCKOUT_MS 50
#endif
static uint32_t scroll_start_time = 0;

// Ignore tiny per-report deltas below this magnitude (counts jitter,
// not intentional movement). Tune to taste, e.g. 1-3.
#ifndef SCROLL_DEADZONE
#    define SCROLL_DEADZONE 1
#endif

// Axis dominance: once one axis's movement exceeds the other's by this
// ratio, treat the weaker axis as unintentional drift and suppress it
// for that report. 1.0 = always pick a single winner; higher values
// (e.g. 2.0-3.0) allow more genuinely-diagonal scrolling through.
#ifndef SCROLL_AXIS_DOMINANCE_RATIO
#    define SCROLL_AXIS_DOMINANCE_RATIO 1.0
#endif

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    if (scroll_active) {
        bool in_lockout = timer_elapsed32(scroll_start_time) < SCROLL_LOCKOUT_MS;

        int8_t dy = mouse_report.y;
        int8_t dx = mouse_report.x;

        // Decide which axis wins this report, if either.
        bool suppress_v = false;
        bool suppress_h = false;
        if (abs(dy) > abs(dx) * SCROLL_AXIS_DOMINANCE_RATIO) {
            suppress_h = true;
        } else if (abs(dx) > abs(dy) * SCROLL_AXIS_DOMINANCE_RATIO) {
            suppress_v = true;
        }
        // else: neither dominates enough - treat as intentional diagonal, let both through

        if (in_lockout || suppress_v || abs(dy) < SCROLL_DEADZONE) {
            mouse_report.v = 0;
            // don't accumulate during lockout/deadzone/suppression, so nothing
            // carries over into the first real scroll tick
        } else {
            scroll_acc_v += (float)dy / SCROLL_DIVISOR_V;
            mouse_report.v = -(int8_t)scroll_acc_v;   // drop the minus if natural scrolling is off
            scroll_acc_v  -=  (int8_t)scroll_acc_v;
        }

        if (in_lockout || suppress_h || abs(dx) < SCROLL_DEADZONE) {
            mouse_report.h = 0;
        } else {
            scroll_acc_h += (float)dx / SCROLL_DIVISOR_H;
            mouse_report.h  = (int8_t)scroll_acc_h;
            scroll_acc_h   -= (int8_t)scroll_acc_h;
        }

        mouse_report.x = 0;
        mouse_report.y = 0;
    }
    return mouse_report;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case SCRL:
            scroll_active = record->event.pressed;
            scroll_acc_v = 0;   // clear stale fraction on each press/release
            scroll_acc_h = 0;
            if (record->event.pressed) {
                scroll_start_time = timer_read32();
            }
            return false;
    }
    return true;
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [0] = LAYOUT( MS_BTN4, MS_BTN5, SCRL, MS_BTN2, MS_BTN1, MO(1) ),
  [1] = LAYOUT( DPI_CONFIG, KC_TRNS, KC_TRNS, KC_TRNS, QK_BOOT, KC_TRNS ),
  [2] = LAYOUT( SCRL, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS ),
  [3] = LAYOUT( SCRL, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS ),
  [4] = LAYOUT( SCRL, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS ),
  [5] = LAYOUT( SCRL, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS ),
  [6] = LAYOUT( SCRL, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS ),
  [7] = LAYOUT( SCRL, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS )
};
