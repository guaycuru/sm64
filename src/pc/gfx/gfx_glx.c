#ifdef __linux__
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <X11/extensions/Xrandr.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>

#include "gfx_window_manager_api.h"
#include "gfx_screen_config.h"

#include "src/pc/controller/controller_keyboard.h"

#include "../configfile.h"

Bool glXGetSyncValuesOML(Display* dpy,
                         GLXDrawable drawable,
                         int64_t* ust,
                         int64_t* msc,
                         int64_t* sbc);

Bool glXGetMscRateOML(Display* dpy,
                      GLXDrawable drawable,
                      int32_t* numerator,
                      int32_t* denominator);

int64_t glXSwapBuffersMscOML(Display* dpy,
                             GLXDrawable drawable,
                             int64_t target_msc,
                             int64_t divisor,
                             int64_t remainder);

Bool glXWaitForMscOML(Display* dpy,
                      GLXDrawable drawable,
                      int64_t target_msc,
                      int64_t divisor,
                      int64_t remainder,
                      int64_t* ust,
                      int64_t* msc,
                      int64_t* sbc);

Bool glXWaitForSbcOML(Display* dpy,
                      GLXDrawable drawable,
                      int64_t target_sbc,
                      int64_t* ust,
                      int64_t* msc,
                      int64_t* sbc);

const struct {
    const char *name;
    int scancode;
} keymap_name_to_scancode[] = {
    {"ESC", 0x01},
    {"AE01", 0x02 },
    {"AE02", 0x03 },
    {"AE03", 0x04 },
    {"AE04", 0x05 },
    {"AE05", 0x06 },
    {"AE06", 0x07 },
    {"AE07", 0x08 },
    {"AE08", 0x09 },
    {"AE09", 0x0a },
    {"AE10", 0x0b },
    {"AE11", 0x0c },
    {"AE12", 0x0d },
    {"BKSP", 0x0e },
    {"TAB", 0x0f },
    {"AD01", 0x10 },
    {"AD02", 0x11 },
    {"AD03", 0x12 },
    {"AD04", 0x13 },
    {"AD05", 0x14 },
    {"AD06", 0x15 },
    {"AD07", 0x16 },
    {"AD08", 0x17 },
    {"AD09", 0x18 },
    {"AD10", 0x19 },
    {"AD11", 0x1a },
    {"AD12", 0x1b },
    {"RTRN", 0x1c },
    {"LCTL", 0x1d },
    {"AC01", 0x1e },
    {"AC02", 0x1f },
    {"AC03", 0x20 },
    {"AC04", 0x21 },
    {"AC05", 0x22 },
    {"AC06", 0x23 },
    {"AC07", 0x24 },
    {"AC08", 0x25 },
    {"AC09", 0x26 },
    {"AC10", 0x27 },
    {"AC11", 0x28 },
    {"TLDE", 0x29 },
    {"LFSH", 0x2a },
    {"BKSL", 0x2b },
    {"AB01", 0x2c },
    {"AB02", 0x2d },
    {"AB03", 0x2e },
    {"AB04", 0x2f },
    {"AB05", 0x30 },
    {"AB06", 0x31 },
    {"AB07", 0x32 },
    {"AB08", 0x33 },
    {"AB09", 0x34 },
    {"AB10", 0x35 },
    {"RTSH", 0x36 },
    {"KPMU", 0x37 },
    {"LALT", 0x38 },
    {"SPCE", 0x39 },
    {"CAPS", 0x3a },
    {"FK01", 0x3b },
    {"FK02", 0x3c },
    {"FK03", 0x3d },
    {"FK04", 0x3e },
    {"FK05", 0x3f },
    {"FK06", 0x40 },
    {"FK07", 0x41 },
    {"FK08", 0x42 },
    {"FK09", 0x43 },
    {"FK10", 0x44 },
    {"NMLK", 0x45 },
    {"SCLK", 0x46 },
    {"KP7", 0x47 },
    {"KP8", 0x48 },
    {"KP9", 0x49 },
    {"KPSU", 0x4a },
    {"KP4", 0x4b },
    {"KP5", 0x4c },
    {"KP6", 0x4d },
    {"KPAD", 0x4e },
    {"KP1", 0x4f },
    {"KP2", 0x50 },
    {"KP3", 0x51 },
    {"KP0", 0x52 },
    {"KPDL", 0x53 },
    {"LVL3", 0x54 }, // correct?
    {"", 0x55 }, // not mapped?
    {"LSGT", 0x56 },
    {"FK11", 0x57 },
    {"FK12", 0x58 },
    {"AB11", 0x59 },
    {"KATA", 0 },
    {"HIRA", 0 },
    {"HENK", 0 },
    {"HKTG", 0 },
    {"MUHE", 0 },
    {"JPCM", 0 },
    {"KPEN", 0x11c },
    {"RCTL", 0x11d },
    {"KPDV", 0x135 },
    {"PRSC", 0x54 }, // ?
    {"RALT", 0x138 },
    {"LNFD", 0 },
    {"HOME", 0x147 },
    {"UP", 0x148 },
    {"PGUP", 0x149 },
    {"LEFT", 0x14b },
    {"RGHT", 0x14d },
    {"END", 0x14f },
    {"DOWN", 0x150 },
    {"PGDN", 0x151 },
    {"INS", 0x152 },
    {"DELE", 0x153 },
    {"PAUS", 0x21d },
    {"LWIN", 0x15b },
    {"RWIN", 0x15c },
    {"COMP", 0x15d },
};

static struct {
    Display *dpy;
    Window root;
    Window win;
    
    Atom atom_wm_state;
    Atom atom_wm_state_fullscreen;
    
    bool is_fullscreen;
    
    int keymap[256];
    
    uint64_t ust0;
    int64_t last_msc;
    uint64_t wanted_ust; // multiplied by 3
    uint64_t vsync_interval;
    uint64_t last_ust;
    int64_t target_msc;
    bool dropped_frame;
} glx;

static void init_keymap(void) {
    XkbDescPtr desc = XkbGetMap(glx.dpy, 0, XkbUseCoreKbd);
    XkbGetNames(glx.dpy, XkbKeyNamesMask, desc);
    
    for (int i = desc->min_key_code; i <= desc->max_key_code && i < 256; i++) {
        char name[XkbKeyNameLength + 1];
        memcpy(name, desc->names->keys[i].name, XkbKeyNameLength);
        name[XkbKeyNameLength] = '\0';
        for (size_t j = 0; j < sizeof(keymap_name_to_scancode) / sizeof(keymap_name_to_scancode[0]); j++) {
            if (strcmp(keymap_name_to_scancode[j].name, name) == 0) {
                glx.keymap[i] = keymap_name_to_scancode[j].scancode;
                break;
            }
        }
    }
    
    XkbFreeNames(desc, XkbKeyNamesMask, True);
    XkbFreeKeyboard(desc, 0, True);
}

static void gfx_glx_set_fullscreen(bool on);

static void gfx_glx_init(void) {
    glx.dpy = XOpenDisplay(NULL);
    if (glx.dpy == NULL) {
        fprintf(stderr, "Cannot connect to X server\n");
        exit(1);
    }
    glx.root = DefaultRootWindow(glx.dpy);
    
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    XVisualInfo *vi = glXChooseVisual(glx.dpy, 0, att);
    if (vi == NULL) {
        fprintf(stderr, "No appropriate GLX visual found\n");
        exit(1);
    }
    Colormap cmap = XCreateColormap(glx.dpy, glx.root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | FocusChangeMask;
    glx.win = XCreateWindow(glx.dpy, glx.root, 0, 0, DESIRED_SCREEN_WIDTH, DESIRED_SCREEN_HEIGHT, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    
    glx.atom_wm_state = XInternAtom(glx.dpy, "_NET_WM_STATE", False);
    glx.atom_wm_state_fullscreen = XInternAtom(glx.dpy, "_NET_WM_STATE_FULLSCREEN", False);
    XMapWindow(glx.dpy, glx.win);

    if (configFullscreen)
 gfx_glx_set_fullscreen(true);

    XStoreName(glx.dpy, glx.win, "Super Mario 64 PC-port GLX");
    GLXContext glc = glXCreateContext(glx.dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(glx.dpy, glx.win, glc);
    
    init_keymap();
    
    int64_t ust, msc, sbc;
    glXGetSyncValuesOML(glx.dpy, glx.win, &ust, &msc, &sbc);
    glx.ust0 = (uint64_t)ust;
    glx.vsync_interval = 16666;
}

static void gfx_glx_main_loop(void (*run_one_game_iter)(void)) {
    while (1) {
        run_one_game_iter();
    }
}

static void gfx_glx_get_dimensions(uint32_t *width, uint32_t *height) {
    XWindowAttributes attributes;
    XGetWindowAttributes(glx.dpy, glx.win, &attributes);
    *width = attributes.width;
    *height = attributes.height;
}

static void gfx_glx_set_fullscreen(bool on) {
    XEvent xev;
    configFullscreen = on;
    xev.xany.type = ClientMessage;
    xev.xclient.message_type = glx.atom_wm_state;
    xev.xclient.format = 32;
    xev.xclient.window = glx.win;
    xev.xclient.data.l[0] = on;
    xev.xclient.data.l[1] = glx.atom_wm_state_fullscreen;
    xev.xclient.data.l[2] = 0;
    xev.xclient.data.l[3] = 0;
    XSendEvent(glx.dpy, glx.root, 0, SubstructureNotifyMask | SubstructureRedirectMask, &xev);
}

static void gfx_glx_handle_events(void) {
    while (XPending(glx.dpy)) {
        XEvent xev;
        XNextEvent(glx.dpy, &xev);
        if (xev.type == FocusOut) {
            keyboard_on_all_keys_up();
        }
        if (xev.type == KeyPress || xev.type == KeyRelease) {
            if (xev.xkey.keycode < 256) {
                int scancode = glx.keymap[xev.xkey.keycode];
                if (scancode != 0) {
                    if (xev.type == KeyPress) {
                        if (scancode == 0x44) { // F10
                            glx.is_fullscreen = !glx.is_fullscreen;
                            gfx_glx_set_fullscreen(glx.is_fullscreen);
                        }
                        keyboard_on_key_down(scancode);
                    } else {
                        keyboard_on_key_up(scancode);
                    }
                }
            }
        }
    }
}

static bool gfx_glx_start_frame(void) {
    return true;
}

static void gfx_glx_swap_buffers_begin(void) {
    glx.wanted_ust += 100000; // advance 1/30 second
    
    double vsyncs_to_wait = (int64_t)(glx.wanted_ust / 3 - glx.last_ust) / (double)glx.vsync_interval;
    if (vsyncs_to_wait <= 0) {
        printf("Dropping frame\n");
        // Drop frame
        glx.dropped_frame = true;
        return;
    }
    if (floor(vsyncs_to_wait) != vsyncs_to_wait) {
        uint64_t left_ust = glx.last_ust + floor(vsyncs_to_wait) * glx.vsync_interval;
        uint64_t right_ust = glx.last_ust + ceil(vsyncs_to_wait) * glx.vsync_interval;
        uint64_t adjusted_wanted_ust = glx.wanted_ust / 3 + (glx.last_ust + 33333 > glx.wanted_ust / 3 ? 2000 : -2000);
        int64_t diff_left = adjusted_wanted_ust - left_ust;
        int64_t diff_right = right_ust - adjusted_wanted_ust;
        if (diff_left < 0) {
            diff_left = -diff_left;
        }
        if (diff_right < 0) {
            diff_right = -diff_right;
        }
        if (diff_left < diff_right) {
            vsyncs_to_wait = floor(vsyncs_to_wait);
        } else {
            vsyncs_to_wait = ceil(vsyncs_to_wait);
        }
        if (vsyncs_to_wait == 0) {
            printf("vsyncs_to_wait became 0 so dropping frame\n");
            glx.dropped_frame = true;
            return;
        }
    }
    glx.dropped_frame = false;
    //printf("Vsyncs to wait: %d, diff: %d\n", (int)vsyncs_to_wait, (int)(glx.last_ust + (int64_t)vsyncs_to_wait * glx.vsync_interval - glx.wanted_ust / 3));
    if (vsyncs_to_wait > 30) {
        // Unreasonable, so change to 2
        vsyncs_to_wait = 2;
    }
    glx.target_msc = glx.last_msc + vsyncs_to_wait;
    
    glXSwapBuffersMscOML(glx.dpy, glx.win, glx.target_msc, 0, 0);
}

static void gfx_glx_swap_buffers_end(void) {
    if (glx.dropped_frame) {
        return;
    }
    
    int64_t ust, msc, sbc;
    if (!glXWaitForSbcOML(glx.dpy, glx.win, 0, &ust, &msc, &sbc)) {
        // X connection broke or something?
        glx.last_ust += (glx.target_msc - glx.last_msc) * glx.vsync_interval;
        glx.last_msc = glx.target_msc;
        return;
    }
    uint64_t this_ust = ust - glx.ust0;
    uint64_t vsyncs_passed = msc - glx.last_msc;
    if (glx.last_ust != 0 && vsyncs_passed != 0) {
        glx.vsync_interval = (this_ust - glx.last_ust) / vsyncs_passed;
        //printf("glx.vsync_interval: %d\n", (int)glx.vsync_interval);
    }
    glx.last_ust = this_ust;
    glx.last_msc = msc;
    if (msc != glx.target_msc) {
        printf("Frame too late by %d vsyncs\n", (int)(msc - glx.target_msc));
    }
    if (msc - glx.target_msc >= 2) {
        // Frame arrived way too late, so reset timer from here
        printf("Reseting timer\n");
        glx.wanted_ust = this_ust * 3;
    }
}

static double gfx_glx_get_time(void) {
    return 0.0;
}

struct GfxWindowManagerAPI gfx_glx = {
    gfx_glx_init,
    gfx_glx_main_loop,
    gfx_glx_get_dimensions,
    gfx_glx_handle_events,
    gfx_glx_start_frame,
    gfx_glx_swap_buffers_begin,
    gfx_glx_swap_buffers_end,
    gfx_glx_get_time
};

#endif
