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
    int x;
    int y;
    int id;
};

bool mouse;
bool crafting_open;
bool inv_bar_empty = true;
int i;
int pressed = 0;
int inv_slot = 0;
int selected_item = 0;
int crafting_slot[5] = {0}; // Contains id of item inside each crafting slot (output slot included)
int inv_bar[3][8] = {{0}, {225, 275, 325, 375, 425, 475 , 525, 575}}; // Contains id of item inside and x value of each inventory slot
struct item world_items[20];
struct item inv_items[8];
const int inv_bar_x = 198;
const int inv_bar_y = 546;
const int inv_y = 573; // Y-value of all items in the inventory bar
const int crafting_x = 260;
const int crafting_y = 200;
const int crafting1_x = 325;
const int crafting1_y = 287;
const int crafting2_x = 375;
const int crafting2_y = 287;
const int crafting3_x = 325;
const int crafting3_y = 337;
const int crafting4_x = 375;
const int crafting4_y = 337;
const int crafting_output_x = 475;
const int crafting_output_y = 312;
float mouse_x;
float mouse_y;
float target_x = 20;
float target_y = 20;
float ball_x = 20;
float ball_y = 20;
float coal_x = 50;
float coal_y = 20;
float stick_x = 180;
float stick_y = 180;
float torch_x;
float torch_y;
float inv1_mouse_z; // Displacement (actual distance) between coal and cursor
float inv2_mouse_z; // Displacement (actual distance) between stick and cursor
float torch_mouse_xsqr; // Squared horizontal distance of torch and cursor
float torch_mouse_ysqr; // Squared vertical distance of torch and cursor
float torch_mouse_z; // Displacement (actual distance) between torch and cursor
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
}

void frame(void) {
    /* ITEM ID
       - no item: 0
       - coal: 1
       - stick: 2
       - torch: 3
    */

    // Player distances
    float coal_player_xsqr; // Squared horizontal distance of coal and player
    float coal_player_ysqr; // Squared vertical distance of coal and player
    float coal_player_z; // Displacement (actual distance) between coal and player
    float stick_player_xsqr; // Squared horizontal distance of stick and player
    float stick_player_ysqr; // Squared vertical distance of stick and player
    float stick_player_z; // Displacement (actual distance) between stick and player
    
    // Inventory mouse distances
    float inv1_mouse_xsqr; // Squared horizontal distance of coal and cursor
    float inv1_mouse_ysqr; // Squared vertical distance of coal and cursor
    float inv2_mouse_xsqr; // Squared horizontal distance of stick and cursor
    float inv2_mouse_ysqr; // Squared vertical distance of stick and cursor

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

    int i;
    int temp;
    // Frame
    int target_fps = 60;
    uint64_t frame_duration = stm_since(frame_start_time); // Duration of the frame since sprite animation started
    int frame_duration_ms = stm_ms(frame_duration); // Converting frame duration to ms
    int target_frame_duration_ms = 1000 / target_fps;
    int need_to_sleep_ms = target_frame_duration_ms - frame_duration_ms; // For determining whether the frame played too fast

    if (need_to_sleep_ms > 0) { // If the actual frame duration was faster than the ideal time
        Sleep(need_to_sleep_ms); // Sleep for the remaining time
    }

    frame_start_time = stm_now(); // Current time is the new start time of the next frame

    /*
    // Check if inventory is empty
    for (i = 0; i < 8; i++) {
        if (inv_bar[0][i] != 0) {
            inv_bar_empty = false;
            break;
        }
    }
    */

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

        // Calculating coal and player displacement
        coal_player_xsqr = (ball_x - coal_x) * (ball_x - coal_x);
        coal_player_ysqr = (ball_y - coal_y) * (ball_y - coal_y);
        coal_player_z = sqrtf(coal_player_xsqr + coal_player_ysqr);

        // Calculating stick and player displacement
        stick_player_xsqr = (ball_x - stick_x) * (ball_x - stick_x);
        stick_player_ysqr = (ball_y - stick_y) * (ball_y - stick_y);
        stick_player_z = sqrtf(stick_player_xsqr + stick_player_ysqr);
        
        if (coal_player_z < 12) {
            coal_x = inv_bar[1][inv_slot];
            coal_y = inv_y;
            inv_bar[0][inv_slot] = 1;
            inv_slot++;
        }

        if (stick_player_z < 12) {
            stick_x = inv_bar[1][inv_slot];
            stick_y = inv_y;
            inv_bar[0][inv_slot] = 2;
            inv_slot++;
        }
    }

    else {
        // Cursor and inventory slots displacement calculations
        inv1_mouse_xsqr = (mouse_x - inv_bar[1][0]) * (mouse_x - inv_bar[1][0]);
        inv1_mouse_ysqr = (mouse_y - inv_y) * (mouse_y - inv_y);
        inv1_mouse_z = sqrtf(inv1_mouse_xsqr + inv1_mouse_ysqr);

        inv2_mouse_xsqr = (mouse_x - inv_bar[1][1]) * (mouse_x - inv_bar[1][1]);
        inv2_mouse_ysqr = (mouse_y - inv_y) * (mouse_y - inv_y);
        inv2_mouse_z = sqrtf(inv2_mouse_xsqr + inv2_mouse_ysqr);

        // Cursor and crafting slot displacement calculations
        crafting1_mouse_xsqr = (mouse_x - crafting1_x) * (mouse_x - crafting1_x);
        crafting1_mouse_ysqr = (mouse_y - crafting1_y) * (mouse_y - crafting1_y);
        crafting1_mouse_z = sqrtf(crafting1_mouse_xsqr + crafting1_mouse_ysqr);

        crafting2_mouse_xsqr = (mouse_x - crafting2_x) * (mouse_x - crafting2_x);
        crafting2_mouse_ysqr = (mouse_y - crafting2_y) * (mouse_y - crafting2_y);
        crafting2_mouse_z = sqrtf(crafting2_mouse_xsqr + crafting2_mouse_ysqr);

        crafting3_mouse_xsqr = (mouse_x - crafting3_x) * (mouse_x - crafting3_x);
        crafting3_mouse_ysqr = (mouse_y - crafting3_y) * (mouse_y - crafting3_y);
        crafting3_mouse_z = sqrtf(crafting3_mouse_xsqr + crafting3_mouse_ysqr);

        crafting4_mouse_xsqr = (mouse_x - crafting4_x) * (mouse_x - crafting4_x);
        crafting4_mouse_ysqr = (mouse_y - crafting4_y) * (mouse_y - crafting4_y);
        crafting4_mouse_z = sqrtf(crafting4_mouse_xsqr + crafting4_mouse_ysqr);

        crafting_output_mouse_xsqr = (mouse_x - crafting_output_x) * (mouse_x - crafting_output_x);
        crafting_output_mouse_ysqr = (mouse_y - crafting_output_y) * (mouse_y - crafting_output_y);
        crafting_output_mouse_z = sqrtf(crafting_output_mouse_xsqr + crafting_output_mouse_ysqr);
        
        if (selected_item == 1) {
            coal_x = mouse_x;
            coal_y = mouse_y;
        }

        else if (selected_item == 2) {
            stick_x = mouse_x; 
            stick_y = mouse_y;
        }

        else if (selected_item == 3) {
            torch_x = mouse_x;
            torch_y = mouse_y;
        }

        // Item in crafting slot 1
        if (crafting_slot[0] == 1) {
            coal_x = crafting1_x;
            coal_y = crafting1_y;
        }

        else if (crafting_slot[0] == 2) {
            stick_x = crafting1_x;
            stick_y = crafting1_y;
        }

        // Item in crafting slot 2
        if (crafting_slot[1] == 1) {
            coal_x = crafting2_x;
            coal_y = crafting2_y;
        }

        else if (crafting_slot[1] == 2) {
            stick_x = crafting2_x;
            stick_y = crafting2_y;
        }

        // Item in crafting slot 3
        if (crafting_slot[2] == 1) {
            coal_x = crafting3_x;
            coal_y = crafting3_y;
        }

        else if (crafting_slot[2] == 2) {
            stick_x = crafting3_x;
            stick_y = crafting3_y;
        }

        // Item in crafting slot 4
        if (crafting_slot[3] == 1) {
            coal_x = crafting4_x;
            coal_y = crafting4_y;
        }

        else if (crafting_slot[3] == 2) {
            stick_x = crafting4_x;
            stick_y = crafting4_y;
        }

        // Crafting recipes

        // Torch recipe
        if ((crafting_slot[0] == 1 && crafting_slot[2] == 2) || (crafting_slot[1] == 1 && crafting_slot[3] == 2)) {
            torch_x = crafting_output_x;
            torch_y = crafting_output_y;
            crafting_slot[4] = 3;
        }
    }

    gx_begin_drawing();
    gx_draw_rect(0, 0, 800, 600, (sg_color) { .4f, .5f, 1.0f, 1.0f });
    gx_draw_sprite(inv_bar_x, inv_bar_y, &inventory_bar);
    gx_draw_sprite(ball_x - 20, ball_y - 20, &player);

    if (crafting_open == true) {
        gx_draw_sprite(crafting_x, crafting_y, &crafting_menu);
    }

    gx_draw_sprite(coal_x - 20, coal_y - 20, &coal);
    gx_draw_sprite(stick_x - 20, stick_y - 20, &stick);

    if(crafting_slot[4] == 3){
        gx_draw_sprite(torch_x - 20, torch_y - 20, &torch);
    }

    if (selected_item == 0) {
        gx_draw_sprite(mouse_x, mouse_y, &cursor);
    }

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
       switch(ev->mouse_button) {
       case SAPP_MOUSEBUTTON_LEFT:
           if (crafting_open == true) {
               if (selected_item == 0) { // No item has been selected
                   // Selecting inventory bar items
                   if (inv1_mouse_z < 20) {
                       selected_item = inv_bar[0][0];
                   }

                   if (inv2_mouse_z < 20) {
                       selected_item = inv_bar[0][1];
                   }

                   // Selecting crafting slot items
                   if (crafting1_mouse_z < 20) {
                       selected_item = crafting_slot[0];
                   }

                   if (crafting2_mouse_z < 20) {
                       selected_item = crafting_slot[1];
                   }

                   if (crafting3_mouse_z < 20) {
                       selected_item = crafting_slot[2];
                   }

                   if (crafting4_mouse_z < 20) {
                       selected_item = crafting_slot[3];
                   }

                   if (crafting_output_mouse_z < 20 && crafting_slot[4] != 0) {
                       selected_item = crafting_slot[4];

                       for (i = 0; i < 4; i++) {
                           if (crafting_slot[i] != 0) {
                               crafting_slot[i] = 0;
                           }
                       }
                   }
               }

               else { // An item has been selected
                   // Dropping item in inventory bar
                   if (inv1_mouse_z < 18) {
                       crafting_slot[3] = selected_item;
                       selected_item = 0;
                   }

                   // Dropping item in crafting slots
                   if (crafting1_mouse_z < 18) {
                       if (selected_item == 1) {
                           selected_item = 0;
                           crafting_slot[0] = 1;
                       }
                       
                       else if (selected_item == 2) {
                           selected_item = 0;
                           crafting_slot[0] = 2;
                       }
                   }

                   if (crafting2_mouse_z < 18) {
                       if (selected_item == 1) {
                           selected_item = 0;
                           crafting_slot[1] = 1;
                       }

                       else if (selected_item == 2) {
                           selected_item = 0;
                           crafting_slot[1] = 2;
                       }
                   }

                   if (crafting3_mouse_z < 18) {
                       if (selected_item == 1) {
                           selected_item = 0;
                           crafting_slot[2] = 1;
                       }

                       else if (selected_item == 2) {
                           selected_item = 0;
                           crafting_slot[2] = 2;
                       }
                   }

                   if (crafting4_mouse_z < 18) {
                       if (selected_item == 1) {
                           selected_item = 0;
                           crafting_slot[3] = 1;
                       }

                       else if (selected_item == 2) {
                           selected_item = 0;
                           crafting_slot[3] = 2;
                       }
                   }
               }
           }
           break;
       default:
           break;
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
                target_x += 18;
            }
            break;
        case SAPP_KEYCODE_LEFT:
            pressed = 2;
            if (target_x > 20) {
                target_x -= 18;
            }
            break;
        case SAPP_KEYCODE_DOWN:
            pressed = 3;
            if (target_y < 580) {
                target_y += 18;
            }
            break;
        case SAPP_KEYCODE_UP:
            pressed = 4;
            if (target_y > 20) {
                target_y -= 18;
            }
            break;
        case SAPP_KEYCODE_E:
            if (crafting_open == true) {
                crafting_slot[0] = 0;
                crafting_slot[1] = 0;
                crafting_slot[2] = 0;
                crafting_slot[3] = 0;
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
