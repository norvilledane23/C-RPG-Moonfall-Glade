// WHERE I LEFT OFF: implementing chests - how to pass chest distance flag to event function

/* CONTROLS
    - W: Walk north
    - A: Walk to the left
    - S: Walk south
    - D: Walk to the right
    - E: Open crafting menu
    - Space: Interact (open containers, talk to NPCs)
*/

/* ITEM ID
   - No item: 0
   - Coal: 1
   - Stick: 2
   - Torch: 3
   - Mint: 4
*/

/* CONTAINER ID
    - Chest: 0
    - Barrel: 1
*/

/* GAMEPLAY STATES
    - Free roam (no menus open and no dialogue scenes active): 0 (Default state)
    - Crafting Menu Opened = 1
    - Container Opened = 2
*/

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
#include "sokol_debugtext.h"
#include "inventory.h"

#define INV_SIZE 8
#define	WORLD_SIZE 20
#define STACK_LIMIT 30

struct container {
    item container_items[8];
    int container_id;
    float x;
    float y;
    float z; // Displacement value from player
    bool open;
};

item world_items[WORLD_SIZE];
item inv_items[INV_SIZE];
item crafting_items[5];
struct container world_containers[2];
bool mouse;
bool crafting_open;
bool inv_bar_empty = true;
int i;
int gameplay_state = 0; // Default state: no menus open (0)
int pressed = 0;
int inv_slot = 0;
int selected_item = 0;
int show_item_obtained_msg_timer = -1; // Counter for how long item obtained message is displayed
char* item_obtained_name = ""; // Name of world item that was picked up
float mouse_x;
float mouse_y;
float target_x = 20;
float target_y = 20;
float ball_x = 24;
float ball_y = 60;
float torch_x;
float torch_y;

float crafting1_mouse_z; // Displacement (actual distance) between cursor and crafting slot 1
float crafting2_mouse_z; // Displacement (actual distance) between cursor and crafting slot 2
float crafting3_mouse_z; // Displacement (actual distance) between cursor and crafting slot 3
float crafting4_mouse_z; // Displacement (actual distance) between cursor and crafting slot 4
float crafting_output_mouse_z; // Displacement (actual distance) between cursor and crafting output slot

// UI sprites
gx_sprite cursor;
gx_sprite inventory_bar;
gx_sprite crafting_menu;

// CHARACTER sprites
gx_sprite player;

// ITEM sprites
gx_sprite coal;
gx_sprite stick;
gx_sprite torch;
gx_sprite mint;
gx_sprite chest;

uint64_t frame_start_time;

void init(void) {
    sg_setup(&(sg_desc) {
        .context = sapp_sgcontext()
    });

    // Text setup
    sdtx_setup(&(sdtx_desc_t) {
        .context_pool_size = 8,
        .printf_buf_size = 4096,
        .fonts = {
            [0] = sdtx_font_kc853(),
            [1] = sdtx_font_kc854(),
            [2] = sdtx_font_z1013(),
            [3] = sdtx_font_cpc(),
            [4] = sdtx_font_c64(),
            [5] = sdtx_font_oric()
        },
        .context = {
            .char_buf_size = 4096,
            .canvas_width = 800,
            .canvas_height = 600,
            .tab_width = 4,
            .color_format = 0,
            .depth_format = 0,
            .sample_count = 0
        },
    });

    gx_setup();
    stm_setup();
    sapp_show_mouse(false); // Remove original computer mouse cursor

    // Sprite creation
    player = gx_make_sprite("player.png");
    coal = gx_make_sprite("coal.png");
    stick = gx_make_sprite("stick.png");
    torch = gx_make_sprite("torch.png");
    mint = gx_make_sprite("mint.png");
    cursor = gx_make_sprite("cursor.png");
    inventory_bar = gx_make_sprite("inventory.png");
    crafting_menu = gx_make_sprite("craftingmenu.png");
    chest = gx_make_sprite("chest.png");

    stick.height = 40;
    stick.width = 40;

    // Setting containers, container contents and coordinates
    world_containers[0].container_id = 0;
    world_containers[0].x = 600;
    world_containers[0].y = 160;

    world_containers[0].container_items[0].x = 225;
    world_containers[0].container_items[0].y = 475;

    // Setting world items and coordinates
    world_items[0].x = 100;
    world_items[0].y = 100;
    world_items[0].item_id = 1;
    world_items[0].name = "coal";

    world_items[1].x = 300;
    world_items[1].y = 300;
    world_items[1].item_id = 2;
    world_items[1].name = "stick";

    world_items[2].x = 180;
    world_items[2].y = 180;
    world_items[2].item_id = 1;
    world_items[2].name = "coal";

    // Spawning random item in container
    world_containers[0].container_items[0].item_id = 1;

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
    // Animation FPS Calculations
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
        if (inv_items[i].item_id == 0) {
            inv_slot = i;
            break;
        }
    }
    
    // Inventory bar mouse interactions
    for (i = 0; i < 8; i++) {
        // Calculating inventory items and cursor displacement
        inv_items[i].z = sqrtf(pow((mouse_x - inv_items[i].x), 2) + pow((mouse_y - inv_items[i].y), 2));
    }

    if (gameplay_state == 1) { // Crafting menu is opened
        for (i = 0; i < 5; i++) {
            // Calculating crafting items and cursor displacement
            crafting_items[i].z = sqrtf(pow((mouse_x - crafting_items[i].x), 2) + pow((mouse_y - crafting_items[i].y), 2));
        }

        // Crafting recipes
        // Torch recipe
        if ((crafting_items[0].item_id == 1 && crafting_items[2].item_id == 2) || (crafting_items[1].item_id == 1 && crafting_items[3].item_id == 2)) {
            crafting_items[4].item_id = 3;
        }
    }

    else { // When no menus are open and dialogue scenes are active
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
            if (world_items[i].item_id != 0) {
                // Calculating world items and player displacement
                world_items[i].z = sqrtf(pow((ball_x - world_items[i].x),2) + pow((ball_y - world_items[i].y), 2));

                // Distance check
                if (world_items[i].z < 24 && inv_slot < 8) {
                    if (inv_item_stack(inv_items, world_items[i].item_id) == -1) {
                        inv_items[inv_slot].item_id = world_items[i].item_id;
                        inv_items[inv_slot].amount = 1;
                        world_items[i].item_id = 0;
                        show_item_obtained_msg_timer = 0;
                        item_obtained_name = world_items[i].name;
                    }

                    else {
                        int matched_slot = inv_item_stack(inv_items, world_items[i].item_id);

                        inv_items[matched_slot].amount++;
                        world_items[i].item_id = 0;
                        show_item_obtained_msg_timer = 0;
                        item_obtained_name = world_items[i].name;
                    }
                }
            }
        }

        // Opening a container
        // Calculating world containers and player displacement
        world_containers[0].z = sqrtf(pow((ball_x - world_containers[0].x), 2) + pow((ball_y - world_containers[0].y), 2));
    }
    
    
    // Spawning all sprites
    /* SPAWNING ORDER
        world containers > player > inventory bar > inventory items > world items > text messages > crafting menu/container interface > crafting menu/container items > cursor
    */

    gx_begin_drawing();
    gx_draw_rect(0, 0, 800, 600, (sg_color) { .4f, .5f, 1.0f, 1.0f });

    // Spawning containers in the world
    gx_draw_sprite(world_containers[0].x - 32, world_containers[0].y - 32, &chest);

    gx_draw_sprite(ball_x - 24, ball_y - 60, &player);
    gx_draw_sprite(198, 546, &inventory_bar);

    // Spawning items in inventory bar and the count of items
    for (i = 0; i < 8; i++) {
        if (inv_items[i].item_id == 1) {
            gx_draw_sprite(inv_items[i].x - 20, inv_items[i].y - 20, &coal);
            sdtx_canvas(sapp_width() / 1.6f, sapp_height() / 1.6f);
            sdtx_origin(16, 45);
            sdtx_color3b(250, 191, 118);
            sdtx_printf("%d", inv_items[i].amount);
        }

        else if (inv_items[i].item_id == 2) {
            gx_draw_sprite(inv_items[i].x - 20, inv_items[i].y - 20, &stick);
        }

        else if (inv_items[i].item_id == 3) {
            gx_draw_sprite(inv_items[i].x - 20, inv_items[i].y - 20, &torch);
        }

        else if (inv_items[i].item_id == 4) {
            gx_draw_sprite(inv_items[i].x - 20, inv_items[i].y - 20, &mint);
        }
    }

    // Spawning items in the world
    for (i = 0; i < 20; i++) {
        if (world_items[i].item_id == 1) {
            gx_draw_sprite(world_items[i].x - 20, world_items[i].y - 20, &coal);
        }

        else if (world_items[i].item_id == 2) {
            gx_draw_sprite(world_items[i].x - 20, world_items[i].y - 20, &stick);
        }

        else if (world_items[i].item_id == 3) {
            gx_draw_sprite(world_items[i].x - 20, world_items[i].y - 20, &torch);
        }

        else if (world_items[i].item_id == 4) {
            gx_draw_sprite(world_items[i].x - 20, world_items[i].y - 20, &mint);
        }
    }

    // Displaying new item obtained notification (duration: 48 frames)

    sdtx_canvas(sapp_width() / 1.6f, sapp_height() / 1.6f);
    sdtx_origin(16, 41);
    sdtx_color3b(250, 191, 118);

    if (show_item_obtained_msg_timer != -1) {
        if (show_item_obtained_msg_timer == 48) {
            show_item_obtained_msg_timer = -1;
        }
        else{
            sdtx_printf("Item obtained: %s", item_obtained_name);
            show_item_obtained_msg_timer++;
        }
    }

    // Spawning crafting menu
    if (gameplay_state == 1) {
        gx_draw_sprite(260, 200, &crafting_menu);

        // Spawning items in crafting menu
        for (i = 0; i < 5; i++) {
            if (crafting_items[i].item_id == 1) {
                gx_draw_sprite(crafting_items[i].x - 20, crafting_items[i].y - 20, &coal);
            }

            else if (crafting_items[i].item_id == 2) {
                gx_draw_sprite(crafting_items[i].x - 20, crafting_items[i].y - 20, &stick);
            }

            else if (crafting_items[i].item_id == 3) {
                gx_draw_sprite(crafting_items[i].x - 20, crafting_items[i].y - 20, &torch);
            }

            else if (crafting_items[i].item_id == 4) {
                gx_draw_sprite(crafting_items[i].x - 20, crafting_items[i].y - 20, &mint);
            }
        }
    }

    // Spawning container interface
    if (gameplay_state == 2) {
        sdtx_origin(16, 33);
        sdtx_printf("Chest", item_obtained_name);
        gx_draw_sprite(198, 448, &inventory_bar);

        // Spawning container items
        if (world_containers[0].container_items[0].item_id == 1) {
            gx_draw_sprite(world_containers[0].container_items[0].x - 20, world_containers[0].container_items[0].y - 20, &coal);
        }

        else if (world_containers[0].container_items[0].item_id == 2) {
            gx_draw_sprite(world_containers[0].container_items[0].x, world_containers[0].container_items[0].y - 20, &stick);
        }

        else if (world_containers[0].container_items[0].item_id == 3) {
            gx_draw_sprite(world_containers[0].container_items[0].x - 20, world_containers[0].container_items[0].y - 20, &torch);
        }

        else if (world_containers[0].container_items[0].item_id == 4) {
            gx_draw_sprite(world_containers[0].container_items[0].x - 20, world_containers[0].container_items[0].y - 20, &mint);
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

    else {
        gx_draw_sprite(mouse_x, mouse_y, &mint);
    }

    sdtx_draw();
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
            // Drop items in world
            if(gameplay_state == 0){
                if (selected_item != 0) {
                    for (i = 0; i < 20; i++) {
                        if (world_items[i].item_id == 0) {
                            world_items[i].item_id = selected_item;
                            selected_item = 0;
                            world_items[i].x = mouse_x;
                            world_items[i].y = mouse_y;
                            selected_item = 0;
                            break;
                        }
                    }
                }
            }

            else if(gameplay_state == 1){
                // Crafting menu mouse interactions
                for (i = 0; i < 5; i++) {
                    // Distance check
                    if (crafting_items[i].z < 32) {
                        // Select items from crafting menu
                        if (selected_item == 0 && crafting_items[i].item_id != 0) {
                            selected_item = crafting_items[i].item_id;
                            crafting_items[i].item_id = 0;

                            if (i == 4) {
                                for (j = 0; j < 4; j++) {
                                    crafting_items[j].item_id = 0;
                                }
                            }
                        }

                        // Drop items in crafting menu
                        else if (selected_item != 0 && crafting_items[i].item_id == 0) {
                            crafting_items[i].item_id = selected_item;
                            selected_item = 0;
                        }

                        break;
                    }
                }
            }

            else if (gameplay_state == 2) {
                for (i = 0; i < 8; i++) {
                    // Calculating container items and cursor displacement
                    world_containers[0].container_items[i].z = sqrtf(pow((mouse_x - world_containers[0].container_items[i].x), 2) + pow((mouse_y - world_containers[0].container_items[i].y), 2));

                    // Distance check
                    if (world_containers[0].container_items[i].z < 32) {
                        // Select items from container
                        if (selected_item == 0 && world_containers[0].container_items[i].item_id != 0) {
                            selected_item = world_containers[0].container_items[i].item_id;
                            world_containers[0].container_items[i].item_id = 0;
                        }

                        // Drop items in container
                        else if (selected_item != 0 && world_containers[0].container_items[i].item_id == 0) {
                            world_containers[0].container_items[i].item_id = selected_item;
                            selected_item = 0;
                        }
                        break;
                    }
                }
            }

            // Inventory bar interactions
            for (i = 0; i < 8; i++) {
                // Distance check
                if (inv_items[i].z < 32) {
                    // Select items from inventory bar
                    if (selected_item == 0 && inv_items[i].item_id != 0) {
                        selected_item = inv_items[i].item_id;
                        inv_items[i].item_id = 0;
                    }

                    // Drop items in inventory bar
                    else if (selected_item != 0 && inv_items[i].item_id == 0) {
                        inv_items[i].item_id = selected_item;
                        selected_item = 0;
                    }
                    break;
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
            if (target_x < 780 && gameplay_state == 0) {
                target_x += 20;
            }
            break;

        case SAPP_KEYCODE_LEFT:
            pressed = 2;
            if (target_x > 20 && gameplay_state == 0) {
                target_x -= 20;
            }
            break;

        case SAPP_KEYCODE_DOWN:
            pressed = 3;
            if (target_y < 580 && gameplay_state == 0) {
                target_y += 20;
            }
            break;

        case SAPP_KEYCODE_UP:
            pressed = 4;
            if (target_y > 20 && gameplay_state == 0) {
                target_y -= 20;
            }
            break;

        case SAPP_KEYCODE_E:
            if (gameplay_state == 1) { // If crafting menu is open
                for (i = 0; i < 5; i++) {
                    if (crafting_items[i].item_id != 0) {
                        for (j = 0; j < 8; j++) {
                            if (inv_items[j].item_id == 0 && i != 4) {
                                inv_items[j].item_id = crafting_items[i].item_id;
                                break;
                            }
                        }
                        crafting_items[i].item_id = 0;
                    }
                }

                gameplay_state = 0;
                break;
            }
            if (gameplay_state  == 0) {
                gameplay_state = 1;
            }
            break;

        case SAPP_KEYCODE_SPACE:
            if(gameplay_state == 2){ // If chest is already open    
                gameplay_state = 0;
                break;
            }

            if (world_containers[0].z < 48) {
                gameplay_state = 2;
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
        .height = 608,
        .window_title = "test1",
    };
}
