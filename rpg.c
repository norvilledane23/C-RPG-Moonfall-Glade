// WHERE I LEFT OFF: implementing chests - how to pass chest distance flag to event function

/*  CONTROLS
    - W: Walk north
    - A: Walk to the left
    - S: Walk south
    - D: Walk to the right
    - E: Interact

    MAP ID
    - Outside: 0
    - Tower: 1

    ITEM ID
    - No item: 0
    - Mint: 1
    - Lepus: 2
    - Veneberry: 3
    - Fairy Dust: 4
    - Bright Mint: 5
    - Stormbreath: 6

    POTION ID
    - Sanus: 0
    - Venenum: 1
    - Magicae: 2
    - Velocitas: 3

    CONTAINER ID
    - Chest: 0
    - Barrel: 1
 
    GAMEPLAY STATES
    - Free roam (no menus open and no dialogue scenes active): 0 (Default state)
    - Crafting Menu Opened = 1
    - Brewing Menu Opened = 2
    - Container Opened = 3
    - Dialogue with NPC
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

// Headers by me
#include "inventory.h"
#include "game_math.h"

struct container{
    item container_items[8];
    int container_id;
    float x;
    float y;
    bool open;
};

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 900
#define INV_SIZE 8
#define	WORLD_SIZE 20
#define STACK_LIMIT 30
#define CRAFT_STATION_X 650
#define CRAFT_STATION_Y 606.5
#define CAULDRON_X 800
#define CAULDRON_Y 599
#define TOWER_DOOR_X 100
#define TOWER_DOOR_Y 542

item world_items[WORLD_SIZE];
item inv_items[INV_SIZE];
item crafting_items[5];
item brewing_items[3];
struct container world_containers[2];
bool mouse;
int current_map_id = 0;
int gameplay_state = 0; // Default state: no menus open (0)
int pressed = 0;
int selected_item = 0;
int money;
int clock_second = 0;
int clock_minute;
int frame_counter = 0;

// Coordinate variables
float mouse_x;
float mouse_y;
float target_x = 20;
float target_y = 20;
float player_x = 60;
float player_y = 180;
float npc_x = 950;
float npc_y = 575;
float view_x = 0;
float view_y = 0;
float inv_bar_x;
float inv_bar_y;
float craft_menu_x;
float craft_menu_y;
float door_x = SCREEN_WIDTH - 109.5;
float door_y = 642;

// UI sprites
gx_sprite cursor;
gx_sprite inv_bar;
gx_sprite craft_menu;
gx_sprite brew_menu;
gx_sprite brew_button;

// CHARACTER sprites
gx_sprite player;
gx_sprite npc1;

// ITEM sprites
gx_sprite mint;
gx_sprite lepus;
gx_sprite veneberry;
gx_sprite fairy_dust;
gx_sprite bright_mint;
gx_sprite stormbreath;

// POTION sprites
gx_sprite sanus;
gx_sprite venenum;
gx_sprite magicae;
gx_sprite velocitas;

// CONTAINER sprites
gx_sprite chest;

// OBSTACLE sprites
gx_sprite tower;
gx_sprite tower_door;
gx_sprite craft_station;
gx_sprite cauldron;

gx_sprite item_sprites[20];
gx_sprite potion_sprites[20];

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
            .canvas_width = SCREEN_WIDTH,
            .canvas_height = SCREEN_HEIGHT,
            .tab_width = 4,
            .color_format = 0,
            .depth_format = 0,
            .sample_count = 0
        },
    });

    gx_setup();
    stm_setup();
    sapp_show_mouse(false); // Remove original computer mouse cursor

    // Entity sprites
    player = gx_make_sprite("player.png");
    npc1 = gx_make_sprite("npc1.png");

    // Item sprites (ingedients)
    mint = gx_make_sprite("mint.png");
    lepus = gx_make_sprite("lepus.png");
    veneberry = gx_make_sprite("veneberry.png");
    fairy_dust = gx_make_sprite("fairydust.png");
    bright_mint = gx_make_sprite("brightmint.png");
    stormbreath = gx_make_sprite("stormbreath.png");

    // Item sprites (potions)
    sanus = gx_make_sprite("sanus.png");
    venenum = gx_make_sprite("venenum.png");
    magicae = gx_make_sprite("magicae.png");
    velocitas = gx_make_sprite("velocitas.png");

    // Container sprites
    chest = gx_make_sprite("chest.png");

    // Obstacle sprites
    tower = gx_make_sprite("tower.png");
    tower_door = gx_make_sprite("towerdoor.png");
    craft_station = gx_make_sprite("craftstation.png");
    cauldron = gx_make_sprite("cauldron.png");

    // UI sprites
    cursor = gx_make_sprite("cursor.png");
    inv_bar = gx_make_sprite("inventory.png");
    craft_menu = gx_make_sprite("craftingmenu.png");
    brew_menu = gx_make_sprite("brewing.png");
    brew_button = gx_make_sprite("brewingbutton.png");

    item_sprites[0] = mint;
    item_sprites[1] = lepus;
    item_sprites[2] = veneberry;
    item_sprites[3] = fairy_dust;
    item_sprites[4] = bright_mint;

    craft_station.width *= 1.5;
    craft_station.height *= 1.5;
    cauldron.width *= 1.5;
    cauldron.height *= 1.5;
    tower_door.width *= 1.5;
    tower_door.height *= 1.5;
    npc1.width /= 2;
    npc1.height /= 2;

    // Setting world items and coordinates
    world_items[0].x = 100;
    world_items[0].y = 100;
    world_items[0].item_id = 1;
    world_items[0].name = "mint";

    world_items[1].x = 300;
    world_items[1].y = 300;
    world_items[1].item_id = 2;
    world_items[1].name = "lepus";

    world_items[2].x = 200;
    world_items[2].y = 750;
    world_items[2].item_id = 3;
    world_items[2].name = "veneberry";

    // Setting inventory bar coordinates
    inv_bar_x = (SCREEN_WIDTH / 2) - (inv_bar.width / 2);
    inv_bar_y = SCREEN_HEIGHT - inv_bar.height;

    // Setting inventory slot coordinates
    for (int i = 0; i < 8; i++) {
        inv_items[i].x = (225 + (i * 50)) * 1.5;
        inv_items[i].y = 573 * 1.5;
    }

    // Setting crafting menu coordinates
    craft_menu_x = (SCREEN_WIDTH / 2) - (craft_menu.width / 2);
    craft_menu_y = (SCREEN_HEIGHT / 2) - (craft_menu.height / 2);

    // Setting crafting slot coordinates
    crafting_items[0].x = 487.5;
    crafting_items[1].x = 562.5;
    crafting_items[2].x = 487.5;
    crafting_items[3].x = 562.5;
    crafting_items[4].x = 712.5;

    crafting_items[0].y = 430.5;
    crafting_items[1].y = 430.5;
    crafting_items[2].y = 505.5;
    crafting_items[3].y = 505.5;;
    crafting_items[4].y = 468;

    // Setting containers, container contents and coordinates
    world_containers[0].container_id = 0;
    world_containers[0].x = 600;
    world_containers[0].y = 160;

    world_containers[0].container_items[0].x = 225 * 1.5;
    world_containers[0].container_items[0].y = 475 * 1.5;
    world_containers[0].container_items[0].item_id = 4;
    world_containers[0].container_items[0].name = "fairy dust";
}

void frame(void) {
    // Animation FPS Calculations
    int target_fps = 48;
    uint64_t frame_duration = stm_since(frame_start_time); // Duration of the frame since sprite animation started
    int frame_duration_ms = stm_ms(frame_duration); // Converting frame duration to ms
    int target_frame_duration_ms = 1000 / target_fps;
    int need_to_sleep_ms = target_frame_duration_ms - frame_duration_ms; // For determining whether the frame played too fast
    frame_counter++;

    if (frame_counter == 48) {
        frame_counter = 0;
        clock_second++;
    }

    if (clock_second == 60) {
        clock_second = 0;
        clock_minute++;
    }
    
    if (clock_minute == 24) {
        clock_minute = 0;
    }

    if (need_to_sleep_ms > 0) { // If the actual frame duration was faster than the ideal time
        Sleep(need_to_sleep_ms); // Sleep for the remaining time
    }

    frame_start_time = stm_now(); // Current time is the new start time of the next frame

    if (gameplay_state == 1) {
        // Crafting recipes
        if ((crafting_items[0].item_id == 1 && crafting_items[2].item_id == 4) ||(crafting_items[1].item_id == 1 && crafting_items[3].item_id == 4)) {
            crafting_items[4].item_id = 5;
        }
        else {
            crafting_items[4].item_id = 0;
        }
    }

    else { // When no menus are open and dialogue scenes are active
        // Player movement
        if (pressed == 1 && player_x < target_x) { // right
            player_x += 8;
        }
        else if (pressed == 2 && player_x > target_x) { // left
            player_x -= 8;
        }
        else if (pressed == 3 && player_y < target_y) { // down
            player_y += 8;
        }
        else if (pressed == 4 && player_y > target_y) { // up
            player_y -= 8;
        }

        // Picking up world items
        for (int i = 0; i < WORLD_SIZE; i++) {
            // Distance check
            if (distance(player_x, world_items[i].x, player_y, world_items[i].y) < 48) {
                int existing_slot = item_check(inv_items, world_items[i].item_id);

                if (existing_slot == -1) {
                    int slot = empty_slot(inv_items);
                    int* picked_up_item;
                    picked_up_item = &world_items[i].item_id;

                    inv_items[slot] = add_inv(inv_items[slot], world_items[i], picked_up_item);
                }

                else {
                    inv_items[existing_slot].amount++;
                    world_items[i].item_id = 0;
                }
            }
        }
    }

    // SPRITE DRAWING
    // Spawning all sprites
    /* SPAWNING ORDER
        background > obstacles > containers > items > players > UI
    */

    gx_begin_drawing();
    sdtx_canvas(sapp_width() / 2.0f, sapp_height() / 2.0f); // Text size
    sdtx_color4b(125, 85, 46, 255); // Text color

    // Map specific spawning
    if (current_map_id == 0) {
        gx_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (sg_color) { 0.4f, 1.0f, 0.6f, 0.75f });
        gx_draw_sprite(SCREEN_WIDTH - tower.width, 0, &tower);

        // Spawning containers in the world
        gx_draw_sprite(world_containers[0].x - 32, world_containers[0].y - 32, &chest);

        // Spawning items in the world
        for (int i = 0; i < 20; i++) {
            if (world_items[i].item_id != 0) {
                gx_draw_sprite(world_items[i].x - 30, world_items[i].y - 30, &item_sprites[world_items[i].item_id - 1]);
            }
        }
        
        gx_draw_sprite(npc_x - (npc1.width / 2), npc_y - (npc1.height / 2), &npc1);
    }

    else if (current_map_id == 1) {
        gx_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (sg_color) { 0.58f, 0.62f, 0.64f, 1.0f });
        gx_draw_sprite(TOWER_DOOR_X - (tower_door.width / 2), TOWER_DOOR_Y - (tower_door.height / 2), &tower_door);
        gx_draw_sprite(CRAFT_STATION_X - (craft_station.width / 2), CRAFT_STATION_Y - (craft_station.height / 2), &craft_station);
        gx_draw_sprite(CAULDRON_X- (cauldron.width / 2), CAULDRON_Y - (cauldron.height / 2), &cauldron);
    }

    gx_draw_sprite(player_x - (player.width / 2), player_y - (player.height / 2), &player);
    gx_draw_sprite(inv_bar_x, inv_bar_y, &inv_bar);

    // Top left UI
    gx_draw_rect(0, 0, 200, 60, (sg_color) { 1.0f, 1.0f, 1.0f, 1.0f });
    sdtx_origin(0, 0);
    sdtx_printf("Time %02d:%02d\nGold %d", clock_minute, clock_second, money);

    // Spawning items in inventory bar and the count of items
    for (int i = 0; i < 8; i++) {
        if (inv_items[i].item_id != 0) {
            gx_draw_sprite(inv_items[i].x - 30, inv_items[i].y - 30, &item_sprites[inv_items[i].item_id-1]);
        }
    }

    // Spawning crafting menu
    if (gameplay_state == 1) {
        gx_draw_sprite(craft_menu_x, craft_menu_y, &craft_menu);

        // Spawning items in crafting menu
        for (int i = 0; i < 5; i++) {
            if (crafting_items[i].item_id != 0) {
                gx_draw_sprite(crafting_items[i].x - 30, crafting_items[i].y - 30, &item_sprites[crafting_items[i].item_id-1]);
            }
        }
    }
    
    else if (gameplay_state == 2) {
        gx_draw_sprite((SCREEN_WIDTH / 2) - (brew_menu.width / 2), (SCREEN_HEIGHT / 2) - (brew_menu.height / 2) , &brew_menu);
        gx_draw_sprite(360, 320, &brew_button);
        gx_draw_sprite(360, 390, &brew_button);
        gx_draw_sprite(360, 460, &brew_button);
        gx_draw_sprite(360, 530, &brew_button);
        /*sdtx_origin(24, 25);
        sdtx_printf("Sanus\n\n\n\nVenenum\n\n\n\nMagicae\n\n\n\nVelocitas");*/
    }

    // Spawning container interface
    else if (gameplay_state == 3) {
        gx_draw_sprite(inv_bar_x, 549, &inv_bar);

        // Spawning container items
        if (world_containers[0].container_items[0].item_id != 0) {
            gx_draw_sprite(world_containers[0].container_items[0].x - 30, world_containers[0].container_items[0].y - 30, &item_sprites[world_containers[0].container_items[0].item_id-1]);
        }
    }

    else if (gameplay_state == 4) {
        gx_draw_rect((SCREEN_WIDTH / 2) - 303, SCREEN_HEIGHT - 120, 606, 120, (sg_color) { 1.0f, 1.0f, 1.0f, 1.0f });
        sdtx_origin(20, 49);
        sdtx_printf("Village wizard! I was hoping\nto speak with you. My sister is\nsick, so I would like to buy a\npotion to heal her.");
    }

    // Spawning cursor
    if (selected_item == 0) {
        gx_draw_sprite(mouse_x, mouse_y, &cursor);
    }

    else {
        gx_draw_sprite(mouse_x, mouse_y, &item_sprites[selected_item-1]);
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
            if(gameplay_state == 1){
                // Crafting menu mouse interactions
                for (int i = 0; i < 5; i++) {
                    // Distance check
                    if (distance(mouse_x, crafting_items[i].x, mouse_y, crafting_items[i].y) < 48) {
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

            else if (gameplay_state == 3) {
                for (int i = 0; i < 8; i++) {
                    // Distance check
                    if (distance(mouse_x, world_containers[0].container_items[i].x, mouse_y, world_containers[0].container_items[i].y) < 48) {
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
            for (int i = 0; i < 8; i++) {
                // Distance check
                if (distance(mouse_x, inv_items[i].x, mouse_y, inv_items[i].y) < 48) {
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
            if (target_x < (SCREEN_WIDTH - (player.width / 2)) && gameplay_state == 0) {
                target_x += 16;
            }
            break;

        case SAPP_KEYCODE_LEFT:
            pressed = 2;
            if (target_x > (player.width / 2) && gameplay_state == 0) {
                target_x -= 16;
            }
            break;

        case SAPP_KEYCODE_DOWN:
            pressed = 3;
            if (target_y < (SCREEN_HEIGHT - (player.height / 2)) && gameplay_state == 0) {
                target_y += 16;
            }
            break;

        case SAPP_KEYCODE_UP:
            pressed = 4;
            if (target_y > (player.height / 2) && gameplay_state == 0) {
                target_y -= 16;
            }
            break;

        case SAPP_KEYCODE_E:
            // Distance check player to door
            if (current_map_id == 0) {
                if (distance(player_x, door_x, player_y, door_y) < 64) {
                    current_map_id = 1;
                    player.width *= 1.5;
                    player.height *= 1.5;
                    player_x = 24;
                    player_y = 560;
                }

                if (distance(player_x, npc_x, player_y, npc_y) < 64) {
                    if (gameplay_state == 0) {
                        gameplay_state = 4; 
                    }

                    else if (gameplay_state == 4) {
                        gameplay_state = 0;
                    }
                }
            }

            else if (current_map_id == 1) {
                // Exit tower
                if (distance(player_x, TOWER_DOOR_X, player_y, TOWER_DOOR_Y) < 64) {
                    current_map_id = 0;
                    player.width /= 1.5;
                    player.height /= 1.5;
                    player_x = door_x;
                    player_y = door_y;
                }

                // Crafting menu
                if (distance(player_x, CRAFT_STATION_X, player_y, CRAFT_STATION_Y) < 48) {
                    if (gameplay_state == 1) { // If crafting menu is open
                        for (int i = 0; i < 5; i++) {
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

                    else if (gameplay_state == 0) {
                        gameplay_state = 1;
                    }
                }

                // Brewing menu
                if (distance(player_x, CAULDRON_X, player_y, CAULDRON_Y) < 48) {
                    if (gameplay_state == 2) { // If brewing menu open
                        gameplay_state = 0;
                        break;
                    }

                    else if (gameplay_state == 0) {
                        gameplay_state = 2;
                    }
                }
            }
            
            break;

        case SAPP_KEYCODE_SPACE:
            if(gameplay_state == 3){ // If chest is already open    
                gameplay_state = 0;
                break;
            }

            if (distance(player_x, world_containers[0].x, player_y, world_containers[0].y) < 48) {
                gameplay_state = 3;
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
        .width = SCREEN_WIDTH,
        .height = SCREEN_HEIGHT,
        .window_title = "test1",
    };
}
