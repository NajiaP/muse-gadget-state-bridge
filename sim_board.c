/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "sim_board.h"
#include "sim_platform.h"

#include <stdlib.h>
#include <string.h>

#include "SDL.h"
#include "src/drivers/sdl/lv_sdl_mouse.h"
#include "src/drivers/sdl/lv_sdl_window.h"

#define WATCHER_RESOLUTION 412

static lv_display_t *s_display;

/* muse_ui.c reads the selected board through this production global. */
const muse_board_t *muse_board;

static esp_err_t sim_init(void)
{
    return ESP_OK;
}

static lv_display_t *sim_display_start(lv_indev_t **touch)
{
    s_display = lv_sdl_window_create(WATCHER_RESOLUTION, WATCHER_RESOLUTION);
    if (!s_display) {
        return NULL;
    }
    /* Optional desktop presentation modes for the 412x412 device UI:
     * MUSE_SIM_ZOOM=2.5 scales it explicitly; MUSE_SIM_PET=1 makes a
     * borderless, always-on-top desktop pet in the corner of the screen;
     * with neither, the window auto-fits ~92% of the current display;
     * MUSE_SIM_FULLSCREEN=1 fills the screen (best with an explicit
     * zoom, otherwise the UI stays device-size, centered).
     * The gadget UI itself always renders at device resolution. */
    const char *pet_env = getenv("MUSE_SIM_PET");
    bool pet = pet_env && pet_env[0] == '1';
    const char *zoom_env = getenv("MUSE_SIM_ZOOM");
    if (zoom_env && zoom_env[0]) {
        double zoom = atof(zoom_env);
        if (zoom >= 0.5 && zoom <= 8.0) {
            lv_sdl_window_set_zoom(s_display, (float)zoom);
        }
    } else if (pet) {
        lv_sdl_window_set_zoom(s_display, 0.55f);
    } else {
        SDL_Window *window = lv_sdl_window_get_window(s_display);
        SDL_DisplayMode mode;
        if (window && SDL_GetCurrentDisplayMode(SDL_GetWindowDisplayIndex(window), &mode) == 0) {
            double zx = (mode.w * 0.92) / WATCHER_RESOLUTION;
            double zy = (mode.h * 0.92) / WATCHER_RESOLUTION;
            double zoom = zx < zy ? zx : zy;
            if (zoom > 4.0) {
                zoom = 4.0;
            }
            if (zoom > 1.05) {
                lv_sdl_window_set_zoom(s_display, (float)zoom);
            }
        }
    }
    if (pet) {
        SDL_Window *window = lv_sdl_window_get_window(s_display);
        if (window) {
            int win_w = 0, win_h = 0;
            SDL_DisplayMode mode;
            SDL_SetWindowBordered(window, SDL_FALSE);
            SDL_SetWindowAlwaysOnTop(window, SDL_TRUE);
            SDL_GetWindowSize(window, &win_w, &win_h);
            if (SDL_GetCurrentDisplayMode(SDL_GetWindowDisplayIndex(window), &mode) == 0 && win_w > 0) {
                SDL_SetWindowPosition(window, mode.w - win_w - 24, mode.h - win_h - 24);
            }
        }
    }
    const char *fs_env = getenv("MUSE_SIM_FULLSCREEN");
    if (fs_env && fs_env[0] == '1') {
        SDL_Window *window = lv_sdl_window_get_window(s_display);
        if (window) {
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
    }
    /* The SDL driver installs SDL_GetTicks. Use the simulator clock instead so
     * scripted runs can advance time without sleeping and render repeatably. */
    lv_tick_set_cb(sim_time_tick_ms);
    lv_sdl_window_set_title(s_display, "Muse Gadget Simulator");
    lv_sdl_window_set_resizeable(s_display, false);
    if (touch) {
        *touch = lv_sdl_mouse_create();
    }
    return s_display;
}

static bool sim_display_lock(int timeout_ms)
{
    (void)timeout_ms;
    return true;
}

static void sim_display_unlock(void)
{
}

static void sim_set_brightness(int pct)
{
    (void)pct;
}

static void sim_panel_sleep(bool sleep)
{
    (void)sleep;
}

static esp_err_t sim_power_off(void)
{
    return ESP_FAIL;
}

static const muse_board_t s_sim_board = {
    .name = "SenseCAP Watcher Simulator",
    .width = WATCHER_RESOLUTION,
    .height = WATCHER_RESOLUTION,
    .round = true,
    .touch = true,
    .diagonal_in = 1.45f,
    .talk_button = "wheel",
    .aux_button = "scroll",
    .talk_hint = { LV_ALIGN_CENTER, 100, -143 },
    .frame_ms = 40,
    .init = sim_init,
    .display_start = sim_display_start,
    .display_lock = sim_display_lock,
    .display_unlock = sim_display_unlock,
    .set_brightness = sim_set_brightness,
    .panel_sleep = sim_panel_sleep,
    .power_off = sim_power_off,
};

const muse_board_t *sim_board_get(void)
{
    return &s_sim_board;
}

lv_display_t *sim_board_display(void)
{
    return s_display;
}
