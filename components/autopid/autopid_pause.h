/*
 * This file is part of the WiCAN project.
 *
 * Copyright (C) 2022  Meatpi Electronics.
 * Written by Ali Slim <ali@meatpi.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#ifndef __AUTOPID_PAUSE_H__
#define __AUTOPID_PAUSE_H__

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "autopid.h"

typedef struct
{
    bool paused;
    const char *reason;
    float voltage;
} autopid_pause_pid_polling_state_t;

bool autopid_pause_is_boot_pid_polling_keep_alive_active(const autopid_config_t *config);
bool autopid_pause_should_pause_pid_polling(const autopid_config_t *config, float *out_voltage, const char **out_reason);
autopid_pause_pid_polling_state_t autopid_pause_get_pid_polling_state(void);

#endif // __AUTOPID_PAUSE_H__
