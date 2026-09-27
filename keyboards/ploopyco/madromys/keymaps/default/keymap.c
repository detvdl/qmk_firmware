/* Copyright 2023 Colin Lam (Ploopy Corporation)
 * Copyright 2020 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
 * Copyright 2019 Sunjun Kim
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

static bool scroll_active = false;

#ifndef SCROLL_DIVISOR_V
#    define SCROLL_DIVISOR_V 64.0
#endif
#ifndef SCROLL_DIVISOR_H
#    define SCROLL_DIVISOR_H 64.0
#endif
static float scroll_acc_v = 0;
static float scroll_acc_h = 0;

#ifndef SCROLL_LOCKOUT_MS
#    define SCROLL_LOCKOUT_MS 50
#endif
static uint32_t scroll_start_time = 0;

#ifndef SCROLL_DEADZONE
#    define SCROLL_DEADZONE 1
#endif

#ifndef SCROLL_AXIS_DOMINANCE_RATIO
#    define SCROLL_AXIS_DOMINANCE_RATIO 1.0
#endif

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    if (scroll_active) {
        bool in_lockout = timer_elapsed32(scroll_start_time) < SCROLL_LOCKOUT_MS;

        int8_t dy = mouse_report.y;
        int8_t dx = mouse_report.x;

        bool suppress_v = false;
        bool suppress_h = false;
        if (abs(dy) > abs(dx) * SCROLL_AXIS_DOMINANCE_RATIO) {
            suppress_h = true;
        } else if (abs(dx) > abs(dy) * SCROLL_AXIS_DOMINANCE_RATIO) {
            suppress_v = true;
        }

        if (in_lockout || suppress_v || abs(dy) < SCROLL_DEADZONE) {
            mouse_report.v = 0;
        } else {
            scroll_acc_v += (float)dy / SCROLL_DIVISOR_V;
            mouse_report.v = -(int8_t)scroll_acc_v;
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
            scroll_acc_v = 0;
            scroll_acc_h = 0;
            if (record->event.pressed) {
                scroll_start_time = timer_read32();
            }
            return false;
    }
    return true;
}


