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

#include "cmd_sdcard.h"
#include "cmdline.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "sdcard.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#define SDCARD_LOG_PATH SD_CARD_MOUNT_POINT "/wican.log"
#define SDCARD_LOG_PAGE_SIZE (8 * 1024)
#define SDCARD_LOG_OUTPUT_CHUNK_SIZE 2048
#define SDCARD_LOG_OUTPUT_DELAY_MS 20

static struct {
    struct arg_lit *info;
    struct arg_lit *test;
    struct arg_lit *log;
    struct arg_lit *tail;
    struct arg_int *page;
    struct arg_end *end;
} sdcard_args;

static int print_log_range(FILE *file, long start, size_t length)
{
    if (fseek(file, start, SEEK_SET) != 0) {
        cmdline_printf("Error: Unable to seek within " SDCARD_LOG_PATH "\n");
        return 1;
    }

    char buffer[SDCARD_LOG_OUTPUT_CHUNK_SIZE + 1];
    size_t remaining = length;
    char last_char = '\n';

    while (remaining > 0) {
        size_t requested = remaining < SDCARD_LOG_OUTPUT_CHUNK_SIZE
                               ? remaining
                               : SDCARD_LOG_OUTPUT_CHUNK_SIZE;
        size_t bytes_read = fread(buffer, 1, requested, file);
        if (bytes_read == 0) {
            break;
        }

        buffer[bytes_read] = '\0';
        last_char = buffer[bytes_read - 1];
        cmdline_printf("%s", buffer);
        remaining -= bytes_read;
        vTaskDelay(pdMS_TO_TICKS(SDCARD_LOG_OUTPUT_DELAY_MS));
    }

    if (ferror(file)) {
        cmdline_printf("\nError: Failed while reading " SDCARD_LOG_PATH "\n");
        return 1;
    }

    if (last_char != '\n') {
        cmdline_printf("\n");
    }
    return 0;
}

static int print_log_tail(FILE *file, long file_size)
{
    long start = file_size > SDCARD_LOG_PAGE_SIZE
                     ? file_size - SDCARD_LOG_PAGE_SIZE
                     : 0;

    // Avoid starting the tail in the middle of a log line.
    if (start > 0 && fseek(file, start, SEEK_SET) == 0) {
        int c;
        while ((c = fgetc(file)) != EOF) {
            if (c == '\n') {
                start = ftell(file);
                break;
            }
        }
    }

    size_t length = file_size > start ? (size_t)(file_size - start) : 0;
    cmdline_printf("--- " SDCARD_LOG_PATH " tail (%u bytes) ---\n", (unsigned)length);
    return print_log_range(file, start, length);
}

static int print_log_page(FILE *file, long file_size, int page)
{
    int total_pages = file_size > 0
                          ? (int)((file_size + SDCARD_LOG_PAGE_SIZE - 1) / SDCARD_LOG_PAGE_SIZE)
                          : 1;
    if (page < 1 || page > total_pages) {
        cmdline_printf("Error: Page must be between 1 and %d\n", total_pages);
        return 1;
    }

    long start = (long)(page - 1) * SDCARD_LOG_PAGE_SIZE;
    size_t length = file_size > start ? (size_t)(file_size - start) : 0;
    if (length > SDCARD_LOG_PAGE_SIZE) {
        length = SDCARD_LOG_PAGE_SIZE;
    }

    cmdline_printf("--- " SDCARD_LOG_PATH " page %d/%d (%u bytes) ---\n",
                   page, total_pages, (unsigned)length);
    return print_log_range(file, start, length);
}

static int cmd_sdcard(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&sdcard_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, sdcard_args.end, argv[0]);
        return 1;
    }

    if (sdcard_args.info->count > 0) {
        sdmmc_card_info_t card_info;
        if (sdcard_get_info(&card_info) != ESP_OK) {
            cmdline_printf("Error: Failed to read SD card info\n");
            return 1;
        }
        cmdline_printf("SD Card Info:\n");
        cmdline_printf("Name: %s\n", card_info.name);
        cmdline_printf("Type: %s\n", card_info.type == CARD_TYPE_SDHC ? "SDHC/SDXC" : 
                            card_info.type == CARD_TYPE_MMC ? "MMC" : 
                            card_info.type == CARD_TYPE_SDIO ? "SDIO" : "SDSC");
        cmdline_printf("Capacity: %.2f GB\n", ((float)card_info.capacity/1024));
        cmdline_printf("Sector Size: %d bytes\n", card_info.sector_size);
        cmdline_printf("Speed: %lu KHz\n", card_info.speed);
        cmdline_printf("OK\n");
        return 0;
    }

    if (sdcard_args.test->count > 0) {
        if (sdcard_test_rw() != ESP_OK) {
            cmdline_printf("Error: SD card test failed\n");
            return 1;
        }
        cmdline_printf("SD card test passed successfully\n");
        cmdline_printf("OK\n");
        return 0;
    }

    if (sdcard_args.log->count > 0 ||
        sdcard_args.tail->count > 0 ||
        sdcard_args.page->count > 0) {
        FILE *file = fopen(SDCARD_LOG_PATH, "r");
        if (file == NULL) {
            cmdline_printf("Error: Unable to open " SDCARD_LOG_PATH "\n");
            return 1;
        }

        if (fseek(file, 0, SEEK_END) != 0) {
            fclose(file);
            cmdline_printf("Error: Unable to determine log size\n");
            return 1;
        }
        long file_size = ftell(file);
        if (file_size < 0) {
            fclose(file);
            cmdline_printf("Error: Unable to determine log size\n");
            return 1;
        }

        int result = sdcard_args.page->count > 0
                         ? print_log_page(file, file_size, sdcard_args.page->ival[0])
                         : print_log_tail(file, file_size);
        fclose(file);
        if (result != 0) {
            return result;
        }
        cmdline_printf("OK\n");
        return 0;
    }

    cmdline_printf("Error: No valid subcommand\n");
    return 1;
}

esp_err_t cmd_sdcard_register(void)
{
    sdcard_args.info = arg_lit0("i", "info", "Get SD card information");
    sdcard_args.test = arg_lit0("t", "test", "Test SD card read/write");
    sdcard_args.log = arg_lit0("l", "log", "Print the newest 8 KiB of the SD card log");
    sdcard_args.tail = arg_lit0(NULL, "tail", "Print the newest 8 KiB of the SD card log");
    sdcard_args.page = arg_int0("p", "page", "N", "Print 8 KiB page N (oldest page is 1)");
    sdcard_args.end = arg_end(6);

    const esp_console_cmd_t cmd = {
        .command = "sdcard",
        .help = "SD card control and status",
        .hint = "Options: -i/--info, -t/--test, -l/--log, --tail, -p/--page <N>",
        .func = &cmd_sdcard,
        .argtable = &sdcard_args
    };
    return cmdline_cmd_register(&cmd);
}
