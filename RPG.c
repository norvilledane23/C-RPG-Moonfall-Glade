#define SOKOL_D3D11
#define SOKOL_IMPL
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define GX_IMPL
#include "gx.h"
#include "sokol_time.h"

bool mouse;
int pressed = 0;
float target_x = 20;
float target_y = 20;
int mouse_x;
int mouse_y;
float ball_x = 20;
float ball_y = 20;
float coal_x = 50;
float coal_y = 20;
float stick_x = 180;
float stick_y = 180;
bool coal_pickedup;
bool stick_pickedup;

gx_sprite hi;
gx_sprite coal;
gx_sprite stick;
gx_sprite cursor;

uint64_t frame_start_time;

void init(void) {
    sg_setup(&(sg_desc) {
        .context = sapp_sgcontext()
    });
    gx_setup();
    stm_setup();
    hi = gx_make_sprite("bitmap.png");
    coal = gx_make_sprite("coal.png");
    stick = gx_make_sprite("stick.png");
    cursor = gx_make_sprite("cursor.png");
    hi.height = 57;
    hi.width = 22;
    coal.height = 40;
    coal.width = 40;
    stick.height = 40;
    stick.width = 40;
}

void frame(void) {
    float coaldelta_x;
    float coaldelta_y;
    float coaldelta_z;
    float stickdelta_x;
    float stickdelta_y;
    float stickdelta_z;
    int target_fps = 60;
    uint64_t frame_duration = stm_since(frame_start_time);
    int frame_duration_ms = stm_ms(frame_duration);
    int target_frame_duration_ms = 1000 / target_fps;
    int need_to_sleep_ms = target_frame_duration_ms - frame_duration_ms;

    if (need_to_sleep_ms > 0) {

        Sleep(need_to_sleep_ms);

    }

    frame_start_time = stm_now();

    if (pressed == 1 && ball_x < target_x) { // right
        ball_x += 4;
    }
    else if (pressed == 2 && ball_x > target_x) { // left
        ball_x -= 4;
    }
    else if (pressed == 3 && ball_y < target_y) { // down
        ball_y += 4;
    }
    else if (pressed == 4 && ball_y > target_y) { // up
        ball_y -= 4;
    }

    coaldelta_x = (ball_x  - coal_x) * (ball_x - coal_x);
    coaldelta_y = (ball_y - coal_y) * (ball_y - coal_y);
    coaldelta_z = sqrtf(coaldelta_x + coaldelta_y);

    stickdelta_x = (ball_x - stick_x) * (ball_x - stick_x);
    stickdelta_y = (ball_y - stick_y) * (ball_y - stick_y);
    stickdelta_z = sqrtf(stickdelta_x + stickdelta_y);

    if (coaldelta_z < 6) {
        coal_pickedup = true;
    }

    if (stickdelta_z < 6) {
        stick_pickedup = true;
    }

    gx_begin_drawing();
    gx_draw_rect(0, 0, 800, 600, (sg_color) { .4f, .5f, 1.0f, 1.0f });
    gx_draw_sprite(ball_x - 20, ball_y - 20, &hi);
    if (coal_pickedup != true) {
        gx_draw_sprite(coal_x - 20, coal_y - 20, &coal);
    }
    if (stick_pickedup != true) {
        gx_draw_sprite(stick_x - 20, stick_y - 20, &stick);
    }
    gx_draw_sprite(mouse_x, mouse_y, &cursor);
    gx_end_drawing();
}

void cleanup(void) {
    gx_shutdown();
    sg_shutdown();
}

static void event(const sapp_event* ev) {
    switch (ev->type) {
    case SAPP_EVENTTYPE_MOUSE_DOWN:
        mouse = 1;
        break;
    case SAPP_EVENTTYPE_MOUSE_UP:
        mouse = 0;
        break;
    case SAPP_EVENTTYPE_MOUSE_MOVE:
        mouse_x = ev->mouse_x;
        mouse_y = ev->mouse_y;
        break;
    case SAPP_EVENTTYPE_KEY_DOWN:
        switch (ev->key_code) {
        case SAPP_KEYCODE_RIGHT:
            pressed = 1;
            if (target_x < 780) {
                target_x += 20;
            }
            break;
        case SAPP_KEYCODE_LEFT:
            pressed = 2;
            if (target_x > 20) {
                target_x -= 20;
            }
            break;
        case SAPP_KEYCODE_DOWN:
            pressed = 3;
            if (target_y < 580) {
                target_y += 20;
            }
            break;
        case SAPP_KEYCODE_UP:
            pressed = 4;
            if (target_y > 20) {
                target_y -= 20;
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc) {
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = event,
        .width = 800,
        .height = 600,
        .window_title = "test1",
    };
}
