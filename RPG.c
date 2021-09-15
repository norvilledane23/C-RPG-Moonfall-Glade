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

struct item {
    float x;
    float y;
    float z;
    int id;
};

bool mouse;
bool crafting_open;
bool inv_bar_empty = true;
int i;
int pressed = 0;
int inv_slot = 0;
int selected_item = 0;
struct item world_items[20];
struct item inv_items[8];
struct item crafting_items[5];
const int inv_bar_x = 198;
const int inv_bar_y = 546;
const int crafting_x = 260;
const int crafting_y = 200;
float mouse_x;
float mouse_y;
float target_x = 20;
float target_y = 20;
float ball_x = 20;
float ball_y = 20;
float torch_x;
float torch_y;
float inv_item_mouse_xsqr; // Squared horizontal distance of inventory items and cursor
float inv_item_mouse_ysqr; // Squared vertical distance of inventory items and cursor

float crafting_item_mouse_xsqr; // Squared horizontal distance between cursor and crafting slots
float crafting_item_mouse_ysqr; // Squared horizontal distance between cursor and crafting slots

float crafting1_mouse_z; // Displacement (actual distance) between cursor and crafting slot 1
float crafting2_mouse_z; // Displacement (actual distance) between cursor and crafting slot 2
float crafting3_mouse_z; // Displacement (actual distance) between cursor and crafting slot 3
float crafting4_mouse_z; // Displacement (actual distance) between cursor and crafting slot 4
float crafting_output_mouse_z; // Displacement (actual distance) between cursor and crafting output slot

gx_sprite player;
gx_sprite coal;
gx_sprite stick;
gx_sprite torch;
gx_sprite cursor;
gx_sprite inventory_bar;
gx_sprite crafting_menu;

uint64_t frame_start_time;

void init(void) {
    sg_setup(&(sg_desc) {
        .context = sapp_sgcontext()
    });
    gx_setup();
    stm_setup();
    sapp_show_mouse(false);
    player = gx_make_sprite("bitmap.png");
    coal = gx_make_sprite("coal.png");
    stick = gx_make_sprite("stick.png");
    torch = gx_make_sprite("torch.png");
    cursor = gx_make_sprite("cursor.png");
    inventory_bar = gx_make_sprite("inventory.png");
    crafting_menu = gx_make_sprite("craftingmenu.png");

    player.height = 40;
    player.width = 40;
    coal.height = 40;
    coal.width = 40;
    stick.height = 40;
    stick.width = 40;

    // Setting world item coordinates
    world_items[0].x = 100;
    world_items[0].y = 100;
    world_items[0].id = 1;

    world_items[1].x = 300;
    world_items[1].y = 300;
    world_items[1].id = 1;

    world_items[2].x = 180;
    world_items[2].y = 180;
    world_items[2].id = 2;

    // Setting inventory slot coordinates
    for (i = 0; i < 8; i++) {
        inv_items[i].x = 225 + (i * 50);
        inv_items[i].y = 573;
    }

    // Setting crafting slot coordinates
    crafting_items[0].x = 325;
    crafting_items[1].x = 375;
    crafting_items[2].x = 325;
    crafting_items[3].x = 375;
    crafting_items[4].x = 475;

    crafting_items[0].y = 287;
    crafting_items[1].y = 287;
    crafting_items[2].y = 337;
    crafting_items[3].y = 337;
    crafting_items[4].y = 312;
}

void frame(void) {
    /* ITEM ID
       - no item: 0
       - coal: 1
       - stick: 2
       - torch: 3
    */

    float item_player_xsqr; // Squared horizontal distance of item and player
    float item_player_ysqr; // Squared vertical distance of item and player

    // Crafting menu distances
    float crafting1_mouse_xsqr; // Squared horizontal distance between cursor and crafting slot 1
    float crafting1_mouse_ysqr; // Squared vertical distance between cursor and crafting slot 1
    float crafting2_mouse_xsqr; // Squared horizontal distance between cursor and crafting slot 2
    float crafting2_mouse_ysqr; // Squared vertical distance between cursor and crafting slot 2
    float crafting3_mouse_xsqr; // Squared horizontal distance between cursor and crafting slot 3
    float crafting3_mouse_ysqr; // Squared vertical distance between cursor and crafting slot 3
    float crafting4_mouse_xsqr; // Squared horizontal distance between cursor and crafting slot 4
    float crafting4_mouse_ysqr; // Squared vertical distance between cursor and crafting slot 4
    float crafting_output_mouse_xsqr; // Squared horizontal distance of cursor and crafting output slot
    float crafting_output_mouse_ysqr; // Squared vertical distance between cursor and crafting output slot

    // FPS Animation Calculations
    int target_fps = 60;
    uint64_t frame_duration = stm_since(frame_start_time); // Duration of the frame since sprite animation started
    int frame_duration_ms = stm_ms(frame_duration); // Converting frame duration to ms
    int target_frame_duration_ms = 1000 / target_fps;
    int need_to_sleep_ms = target_frame_duration_ms - frame_duration_ms; // For determining whether the frame played too fast

    if (need_to_sleep_ms > 0) { // If the actual frame duration was faster than the ideal time
        Sleep(need_to_sleep_ms); // Sleep for the remaining time
    }

    frame_start_time = stm_now(); // Current time is the new start time of the next frame
    
    // Checking for empty inventory slot for various uses (i.e. returning unused crafting items, picking up new world items)
    for (i = 0; i < 8; i++) {
        if (inv_items[i].id == 0) {
            inv_slot = i;
            break;
        }
    }

    if (crafting_open != true) { // When crafting menu is not open
        // Player movement
        if (pressed == 1 && ball_x < target_x) { // right
            ball_x += 8;
        }
        else if (pressed == 2 && ball_x > target_x) { // left
            ball_x -= 8;
        }
        else if (pressed == 3 && ball_y < target_y) { // down
            ball_y += 8;
        }
        else if (pressed == 4 && ball_y > target_y) { // up
            ball_y -= 8;
        }

        // Picking up world items
        for (i = 0; i < 20; i++) {
            if(world_items[i].id != 0){
                // Calculating world items and player displacement
                item_player_xsqr = (ball_x - world_items[i].x) * (ball_x - world_items[i].x);
                item_player_ysqr = (ball_y - world_items[i].y) * (ball_y - world_items[i].y);
                world_items[i].z = sqrtf(item_player_xsqr + item_player_ysqr);

                // Distance check
                if (world_items[i].z < 24 && inv_slot < 8) {
                    inv_items[inv_slot].id = world_items[i].id;
                    world_items[i].id = 0;
                }
            }
        }
    }

    else {
        // Inventory bar mouse interactions
        for (i = 0; i < 8; i++) {
            // Calculating inventory items and cursor displacement
            inv_item_mouse_xsqr = (mouse_x - inv_items[i].x) * (mouse_x - inv_items[i].x);
            inv_item_mouse_ysqr = (mouse_y - inv_items[i].y) * (mouse_y - inv_items[i].y);
            inv_items[i].z = sqrtf(inv_item_mouse_xsqr + inv_item_mouse_ysqr);
        }

        for (i = 0; i < 5; i++) {
            // Calculating crafting items and cursor displacement
            crafting_item_mouse_xsqr = (mouse_x - crafting_items[i].x) * (mouse_x - crafting_items[i].x);
            crafting_item_mouse_ysqr = (mouse_y - crafting_items[i].y) * (mouse_y - crafting_items[i].y);
            crafting_items[i].z = sqrtf(crafting_item_mouse_xsqr + crafting_item_mouse_ysqr);
        }

        // Crafting recipes
        // Torch recipe
        if ((crafting_items[0].id == 1 && crafting_items[2].id == 2) || (crafting_items[1].id == 1 && crafting_items[3].id == 2)) {
            crafting_items[4].id = 3;
        }
    }

    gx_begin_drawing();
    gx_draw_rect(0, 0, 800, 600, (sg_color) { .4f, .5f, 1.0f, 1.0f });
    gx_draw_sprite(ball_x - 20, ball_y - 20, &player);
    gx_draw_sprite(inv_bar_x, inv_bar_y, &inventory_bar);

    // Spawning items in the world
    for (i = 0; i < 20; i++) {
        if (world_items[i].id == 1) {
            gx_draw_sprite(world_items[i].x - 20, world_items[i].y - 20, &coal);
        }

        else if (world_items[i].id == 2) {
            gx_draw_sprite(world_items[i].x - 20, world_items[i].y - 20, &stick);
        }
    }

    // Spawning crafting menu
    if (crafting_open == true) {
        gx_draw_sprite(crafting_x, crafting_y, &crafting_menu);
    }

    // Spawning items in inventory bar
    for (i = 0; i < 8; i++) {
        if (inv_items[i].id == 1) {
            gx_draw_sprite(inv_items[i].x - 20, inv_items[i].y - 20, &coal);
        }

        else if (inv_items[i].id == 2) {
            gx_draw_sprite(inv_items[i].x - 20, inv_items[i].y - 20, &stick);
        }

        else if (inv_items[i].id == 3) {
            gx_draw_sprite(inv_items[i].x - 20, inv_items[i].y - 20, &torch);
        }
    }

    // Spawning items in crafting menu
    for (i = 0; i < 5; i++) {
        if (crafting_items[i].id == 1) {
            gx_draw_sprite(crafting_items[i].x - 20, crafting_items[i].y - 20, &coal);
        }

        else if (crafting_items[i].id == 2) {
            gx_draw_sprite(crafting_items[i].x - 20, crafting_items[i].y - 20, &stick);
        }

        else if (crafting_items[i].id == 3) {
            gx_draw_sprite(crafting_items[i].x - 20, crafting_items[i].y - 20, &torch);
        }
    }

    // Spawning cursor
    if (selected_item == 0) {
        gx_draw_sprite(mouse_x, mouse_y, &cursor);
    }

    else if(selected_item == 1) {
        gx_draw_sprite(mouse_x, mouse_y, &coal);
    }

    else if (selected_item == 2) {
        gx_draw_sprite(mouse_x, mouse_y, &stick);
    }

    else if (selected_item == 3) {
        gx_draw_sprite(mouse_x, mouse_y, &torch);
    }

    gx_end_drawing();
}

void cleanup(void) {
    gx_shutdown();
    sg_shutdown();
}

static void event(const sapp_event* ev) {
    int j;
    switch (ev->type) {
    case SAPP_EVENTTYPE_MOUSE_DOWN:
        mouse = 1;
        switch (ev->mouse_button) {
        case SAPP_MOUSEBUTTON_LEFT:
            if (crafting_open == true) {
                // Inventory bar mouse interactions
                for (i = 0; i < 8; i++) {
                    // Distance check
                    if (inv_items[i].z < 32) {
                        // Select items from inventory bar
                        if (selected_item == 0 && inv_items[i].id != 0) {
                                selected_item = inv_items[i].id;
                                inv_items[i].id = 0;
                        }

                        // Drop items in inventory bar
                        else if(selected_item != 0 && inv_items[i].id == 0) {
                                inv_items[i].id = selected_item;
                                selected_item = 0;
                        }

                        break;
                    }
                }

                // Crafting menu mouse interactions
                for (i = 0; i < 5; i++) {
                    // Distance check
                    if (crafting_items[i].z < 32) {
                        // Select items from crafting menu
                        if (selected_item == 0 && crafting_items[i].id != 0) {
                            selected_item = crafting_items[i].id;
                            crafting_items[i].id = 0;

                            if (i == 4) {
                                for (j = 0; j < 4; j++) {
                                    crafting_items[j].id = 0;
                                }
                            }
                        }

                        // Drop items in crafting menu
                        else if (selected_item != 0 && crafting_items[i].id == 0) {
                            crafting_items[i].id = selected_item;
                            selected_item = 0;
                        }

                        break;
                    }
                }
            }
        }
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
        case SAPP_KEYCODE_E:
            if (crafting_open == true) {
                for (i = 0; i < 5; i++) {
                    if (crafting_items[i].id != 0) {
                        for (j = 0; j < 8; j++) {
                            if (inv_items[j].id == 0 && i != 4) {
                                inv_items[j].id = crafting_items[i].id;
                                break;
                            }
                        }
                        crafting_items[i].id = 0;
                    }
                }

                crafting_open = false;
                break;
            }
            crafting_open = true;
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
