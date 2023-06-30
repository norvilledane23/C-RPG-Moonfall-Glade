/*  MOONFALL GLADE: A SIMULATION ROLE-PLAYING GAME
    Student Name: Norville Amao

    CONTROLS
    - W: Walk north
    - A: Walk to the left
    - S: Walk south
    - D: Walk to the right
    - E: Interact
    - Space: Open container

    MAP ID
    - Outside (Forest): 0
    - Tower: 1

    ITEM ID
    - No item: 0
    - Mint: 1
    - Lepus: 2
    - Veneberry: 3
    - Fairy Dust: 4
    - Bright Mint: 5
    - Stormbreath: 6
    - Sanus: 7
    - Venenum: 8
    - Magicae: 9
    - Velocitas: 10

    CONTAINER ID
    - Chest: 0
    - Barrel: 1
 
    GAMEPLAY STATES
    - Free roam (no menus open and no dialogue scenes active): 0 (Default state)
    - Crafting Menu Opened = 1
    - Brewing Menu Opened = 2
    - Container Opened = 3
    - Dialogue with NPC = 4
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
#include "game_math.h"
#include "inventory.h"
#include "brewing.h"

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
#define BOOKSHELF_X 450
#define BOOKSHELF_Y 542
#define TOWER_DOOR_X 100
#define TOWER_DOOR_Y 542

item items[20];
item world_items[WORLD_SIZE];
item inv_items[INV_SIZE];
item crafting_items[5];
item brewing_items[3];
item potion_liquids[3];
struct container world_containers[2];

item selected_item;
item potion_result;
item no_item;
item mint;
item lepus;
item veneberry;
item fairy_dust;
item bright_mint;
item stormbreath;
item sanus;
item venenum;
item magicae;
item velocitas;
item water;
item liquid_sap;
item lemon_juice;

bool mouse;
bool show_continue = false;
bool continue_clicked = false;
int temp = 0;
int speed = 0;
int current_map_id = 0; // Initialized to 0 (Forest)
int gameplay_state = 0; // Default state: no menus open (0)
int left_pressed = 0;
int right_pressed = 0;
int up_pressed = 0;
int down_pressed = 0;
int money;
int clock_second = 0;
int clock_minute;
int frame_counter = 0;
int near_button = 0;
int brew_page = 0;
int brew_potion;

// Coordinate variables
float mouse_x;
float mouse_y;
float player_x = 750;
float player_y = 700;
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
gx_sprite brew_box;
gx_sprite brew_menu;
gx_sprite brew_menu2;
gx_sprite brew_menu3;

gx_sprite pbutton0;
gx_sprite pbutton1;
gx_sprite pbutton2;
gx_sprite pbutton3;
gx_sprite pbutton30;
gx_sprite pbutton31;
gx_sprite pbutton32;
gx_sprite pbutton33;
gx_sprite pbutton34;
gx_sprite pbutton35;
gx_sprite brew_continue;

gx_sprite pbutton0_h;
gx_sprite pbutton1_h;
gx_sprite pbutton2_h;
gx_sprite pbutton3_h;
gx_sprite pbutton30_h;
gx_sprite pbutton31_h;
gx_sprite pbutton32_h;
gx_sprite pbutton33_h;
gx_sprite pbutton34_h;
gx_sprite pbutton35_h;
gx_sprite brew_continue_h;

// CHARACTER sprites
gx_sprite player;
gx_sprite npc1;

// CONTAINER sprites
gx_sprite chest;

// OBSTACLE sprites
gx_sprite tower;
gx_sprite tower_door;
gx_sprite craft_station;
gx_sprite cauldron;
gx_sprite bookshelf;

// Sprite arrays
gx_sprite brew_UI_sprites[20];
gx_sprite brew_UI_sprites_h[20]; // highlighted UI sprites

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

    // Container sprites
    chest = gx_make_sprite("chest.png");

    // Obstacle sprites
    tower = gx_make_sprite("tower.png");
    tower_door = gx_make_sprite("towerdoor.png");
    craft_station = gx_make_sprite("craftstation.png");
    cauldron = gx_make_sprite("cauldron.png");
    bookshelf = gx_make_sprite("bookshelf.png");

    // UI sprites
    cursor = gx_make_sprite("cursor.png");
    inv_bar = gx_make_sprite("inventory.png");
    craft_menu = gx_make_sprite("craftingmenu.png");
    brew_box = gx_make_sprite("brewbox.png");
    brew_menu = gx_make_sprite("brewing.png");
    brew_menu2 = gx_make_sprite("brewing2.png");
    brew_menu3 = gx_make_sprite("brewing3.png");
    pbutton0 = gx_make_sprite("pbutton0.png");
    pbutton1 = gx_make_sprite("pbutton1.png");
    pbutton2 = gx_make_sprite("pbutton2.png");
    pbutton3 = gx_make_sprite("pbutton3.png");
    pbutton30 = gx_make_sprite("pbutton30.png");
    pbutton31 = gx_make_sprite("pbutton31.png");
    pbutton32 = gx_make_sprite("pbutton32.png");
    pbutton33 = gx_make_sprite("pbutton33.png");
    pbutton34 = gx_make_sprite("pbutton34.png");
    pbutton35 = gx_make_sprite("pbutton35.png");
    brew_continue = gx_make_sprite("continue.png");

    // Highlighted UI
    pbutton0_h = gx_make_sprite("pbutton0_h.png");
    pbutton1_h = gx_make_sprite("pbutton1_h.png");
    pbutton2_h = gx_make_sprite("pbutton2_h.png");
    pbutton3_h = gx_make_sprite("pbutton3_h.png");
    pbutton30_h = gx_make_sprite("pbutton30_h.png");
    pbutton31_h = gx_make_sprite("pbutton31_h.png");
    pbutton32_h = gx_make_sprite("pbutton32_h.png");
    pbutton33_h = gx_make_sprite("pbutton33_h.png");
    pbutton34_h = gx_make_sprite("pbutton34_h.png");
    pbutton35_h = gx_make_sprite("pbutton35_h.png");
    brew_continue_h = gx_make_sprite("continue_h.png");

    // Sprite arrays
    brew_UI_sprites[0] = pbutton0;
    brew_UI_sprites[1] = pbutton1;
    brew_UI_sprites[2] = pbutton2;
    brew_UI_sprites[3] = pbutton3;
    brew_UI_sprites[4] = pbutton30;
    brew_UI_sprites[5] = pbutton31;
    brew_UI_sprites[6] = pbutton32;
    brew_UI_sprites[7] = pbutton33;
    brew_UI_sprites[8] = pbutton34;
    brew_UI_sprites[9] = pbutton35;
    brew_UI_sprites[10] = brew_menu;
    brew_UI_sprites[11] = brew_menu2;
    brew_UI_sprites[12] = brew_menu3;
    brew_UI_sprites[13] = brew_box;
    brew_UI_sprites[14] = brew_continue;

    brew_UI_sprites_h[0] = pbutton0_h;
    brew_UI_sprites_h[1] = pbutton1_h;
    brew_UI_sprites_h[2] = pbutton2_h;
    brew_UI_sprites_h[3] = pbutton3_h;
    brew_UI_sprites_h[4] = pbutton30_h;
    brew_UI_sprites_h[5] = pbutton31_h;
    brew_UI_sprites_h[6] = pbutton32_h;
    brew_UI_sprites_h[7] = pbutton33_h;
    brew_UI_sprites_h[8] = pbutton34_h;
    brew_UI_sprites_h[9] = pbutton35_h;
    brew_UI_sprites_h[10] = brew_continue_h;

    // Setting up items
    no_item.item_id = 0;
    no_item.name = "";
    no_item.sprite = gx_make_sprite("");
    items[0] = no_item;


    mint.item_id = 1;
    mint.name = "Mint";
    mint.sprite = gx_make_sprite("mint.png");
    items[1] = mint;


    lepus.item_id = 2;
    lepus.name = "Lepus";
    lepus.sprite = gx_make_sprite("lepus.png");
    items[2] = lepus;


    veneberry.item_id = 3;
    veneberry.name = "Veneberry";
    veneberry.sprite = gx_make_sprite("veneberry.png");
    items[3] = veneberry;


    fairy_dust.item_id = 4;
    fairy_dust.name = "Fairy Dust";
    fairy_dust.sprite = gx_make_sprite("fairydust.png");
    items[4] = fairy_dust;


    bright_mint.item_id = 5;
    bright_mint.name = "Bright Mint";
    bright_mint.sprite = gx_make_sprite("brightmint.png");
    items[5] = bright_mint;


    stormbreath.item_id = 6;
    stormbreath.name = "Stormbreath";
    stormbreath.sprite = gx_make_sprite("stormbreath.png");
    items[6] = stormbreath;


    sanus.item_id = 7;
    sanus.name = "Sanus";
    sanus.sprite = gx_make_sprite("sanus.png");
    items[7] = sanus;


    venenum.item_id = 8;
    venenum.name = "Venenum";
    venenum.sprite = gx_make_sprite("venenum.png");
    items[8] = venenum;


    magicae.item_id = 9;
    magicae.name = "Magicae";
    magicae.sprite = gx_make_sprite("magicae.png");
    items[9] = magicae;

    velocitas.item_id = 10;
    velocitas.name = "Velocitas";
    velocitas.sprite = gx_make_sprite("velocitas.png");
    items[10] = velocitas;

    // Potion liquid
    water.item_id = 20;
    water.name = "Water";
    water.sprite = gx_make_sprite("water.png");
    potion_liquids[0] = water;

    liquid_sap.item_id = 21;
    liquid_sap.name = "Liquid Sap";
    liquid_sap.sprite = gx_make_sprite("liquidsap.png");
    potion_liquids[1] = liquid_sap;

    lemon_juice.item_id = 22;
    lemon_juice.name = "Lemon Juice";
    lemon_juice.sprite = gx_make_sprite("lemonjuice.png");
    potion_liquids[2] = lemon_juice;

    potion_result.x = 600;
    potion_result.y = 640;
    potion_result.item_id = 0;
    potion_result.name = "";
    potion_result.sprite = gx_make_sprite("");

    // Objects in Tower map are 1.5 larger than in the forest
    craft_station.width *= 1.5;
    craft_station.height *= 1.5;
    cauldron.width *= 1.5;
    cauldron.height *= 1.5;
    bookshelf.width *= 1.5;
    bookshelf.height *= 1.5;
    tower_door.width *= 1.5;
    tower_door.height *= 1.5;
    npc1.width /= 2;
    npc1.height /= 2;

    // Setting world items and coordinates
    world_items[0] = mint;
    world_items[0].x = 100;
    world_items[0].y = 100;

    world_items[1] = lepus;
    world_items[1].x = 300;
    world_items[1].y = 300;

    world_items[2] = veneberry;
    world_items[2].x = 650;
    world_items[2].y = 750;

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

    for (int i = 0; i < 3; i++) {
        brewing_items[i].x = 424;
    }

    brewing_items[0].y = 376;
    brewing_items[1].y = 476;
    brewing_items[2].y = 576;

    potion_liquids[0].x = 650;
    potion_liquids[1].x = 750;
    potion_liquids[2].x = 700;

    potion_liquids[0].y = 376;
    potion_liquids[1].y = 376;
    potion_liquids[2].y = 476;

    // Setting containers, container contents and coordinates
    world_containers[0].container_id = 0;
    world_containers[0].x = 600;
    world_containers[0].y = 160;

    world_containers[0].container_items[0].x = 225 * 1.5;
    world_containers[0].container_items[0].y = 475 * 1.5;
    world_containers[0].container_items[0].item_id = items[4].item_id;
    world_containers[0].container_items[0].name = items[4].name;
    world_containers[0].container_items[0].sprite = items[4].sprite;

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

    if (gameplay_state == 0) { // Free roam
        // Player movement only occurs in free roam
        if (right_pressed == 1) { // right
            player_x += right_border_check(player_x, player.width / 2);
        }

        if (left_pressed == 1) { // left
            player_x -= left_border_check(player_x, player.width / 2);;
        }

        if (down_pressed == 1) { // down
            player_y += bottom_border_check(player_y, player.height / 2);
        }

        if (up_pressed == 1) { // up
            player_y -= top_border_check(player_y, player.height / 2);
        }

        // Picking up world items
        for (int i = 0; i < WORLD_SIZE; i++) {
            // Distance check
            if (distance(player_x, world_items[i].x, player_y, world_items[i].y) < 48) {
                int existing_slot = item_check(inv_items, world_items[i].item_id);

                if (existing_slot == -1) {
                    int slot = empty_slot(inv_items);
                    item* picked_up_item;
                    picked_up_item = &world_items[i];

                    inv_items[slot] = add_inv(inv_items[slot].x, inv_items[slot].y, inv_items[slot], world_items[i], picked_up_item);
                }

                else {
                    inv_items[existing_slot].amount++;
                    world_items[i].item_id = 0;
                }
            }
        }
    }

    else if (gameplay_state == 1) {
        // Crafting recipes
        if ((crafting_items[0].item_id == 1 && crafting_items[2].item_id == 4) ||(crafting_items[1].item_id == 1 && crafting_items[3].item_id == 4)) {
            crafting_items[4].item_id = 5;
            crafting_items[4].name = items[5].name;
            crafting_items[4].sprite = items[5].sprite;
        }
        else {
            crafting_items[4].item_id = 0;
        }
    }

    else if (gameplay_state == 2) {
        if ((brewing_items[0].item_id == 2 || brewing_items[1].item_id == 2) && (brewing_items[0].item_id == 3 || brewing_items[1].item_id == 3) && brewing_items[2].item_id == 21) {
            show_continue = true;
        }

        if (brew_page == 2) {
            if (temp == 3 && speed == 1) {
                potion_result.item_id = items[8].item_id;
                potion_result.name = items[8].name;
                potion_result.sprite = items[8].sprite;
            }
        }
    }

    /*  SPRITE DRAWING
    * 
        SPAWNING ORDER
        background > obstacles > containers > items > players > UI
    */

    gx_begin_drawing();
    sdtx_canvas(sapp_width() / 2.0f, sapp_height() / 2.0f); // Text size
    sdtx_color4b(125, 85, 46, 255); // Text color

    // Map specific spawning
    // Outside (Forest)
    if (current_map_id == 0) {
        gx_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (sg_color) { 0.4f, 1.0f, 0.6f, 0.75f });
        gx_draw_sprite(SCREEN_WIDTH - tower.width, 0, &tower);

        // Spawning containers in the world
        gx_draw_sprite(world_containers[0].x - 32, world_containers[0].y - 32, &chest);

        // Spawning items in the world
        for (int i = 0; i < 20; i++) {
            if (world_items[i].item_id != 0) {
                gx_draw_sprite(world_items[i].x - 30, world_items[i].y - 30, &world_items[i].sprite);
            }
        }
        
        gx_draw_sprite(npc_x - (npc1.width / 2), npc_y - (npc1.height / 2), &npc1);
    }

    //Inside tower
    else if (current_map_id == 1) {
        gx_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (sg_color) { 0.58f, 0.62f, 0.64f, 1.0f });
        gx_draw_sprite(TOWER_DOOR_X - (tower_door.width / 2), TOWER_DOOR_Y - (tower_door.height / 2), &tower_door);
        gx_draw_sprite(CRAFT_STATION_X - (craft_station.width / 2), CRAFT_STATION_Y - (craft_station.height / 2), &craft_station);
        gx_draw_sprite(CAULDRON_X - (cauldron.width / 2), CAULDRON_Y - (cauldron.height / 2), &cauldron);
        gx_draw_sprite(BOOKSHELF_X - (bookshelf.width / 2), BOOKSHELF_Y - (bookshelf.height / 2), &bookshelf);
    }

    gx_draw_sprite(player_x - (player.width / 2), player_y - (player.height / 2), &player);
    gx_draw_sprite(inv_bar_x, inv_bar_y, &inv_bar);

    // Top left UI
    gx_draw_rect(0, 0, 200, 60, (sg_color) { 1.0f, 1.0f, 1.0f, 1.0f });
    sdtx_origin(0, 0);
    sdtx_printf("Time %02d:%02d\nGold %d\n", clock_minute, clock_second, money, near_button, temp, speed);

    // Spawning items in inventory bar and the count of items
    for (int i = 0; i < 8; i++) {
        if (inv_items[i].item_id != 0) {
            gx_draw_sprite(inv_items[i].x - 30, inv_items[i].y - 30, &inv_items[i].sprite);
        }
    }

    // Crafting menu
    if (gameplay_state == 1) {
        gx_draw_sprite(craft_menu_x, craft_menu_y, &craft_menu);

        // Spawning items in crafting menu
        for (int i = 0; i < 5; i++) {
            if (crafting_items[i].item_id != 0) {
                gx_draw_sprite(crafting_items[i].x - 30, crafting_items[i].y - 30, &crafting_items[i].sprite);
            }
        }
    }
    
    // Brewing menu
    else if (gameplay_state == 2) {
        int* button = &near_button;

        if (brew_page == 0) {
            potion_selection(mouse_x, mouse_y, button, brew_menu, pbutton1, pbutton1_h);
        }

        else if (brew_page == 1) {
            ingredient_selection(mouse_x, mouse_y, show_continue, button, brew_UI_sprites, brew_continue_h, brewing_items, potion_liquids);
        }

        else if (brew_page == 2) {
            cooking_selection(mouse_x, mouse_y, button, potion_result, brew_UI_sprites, brew_UI_sprites_h);
        }
    }

    // Spawning container interface
    else if (gameplay_state == 3) {
        gx_draw_sprite(inv_bar_x, 672, &inv_bar);

        // Spawning container items
        if (world_containers[0].container_items[0].item_id != 0) {
            gx_draw_sprite(world_containers[0].container_items[0].x - 30, world_containers[0].container_items[0].y - 30, &world_containers[0].container_items[0].sprite);
        }
    }

    else if (gameplay_state == 4) {
        gx_draw_rect((SCREEN_WIDTH / 2) - 303, SCREEN_HEIGHT - 120, 606, 120, (sg_color) { 1.0f, 1.0f, 1.0f, 1.0f });
        sdtx_origin(20, 49);
        sdtx_printf("Village wizard! I was hoping\nto speak with you. My sister is\nsick, so I would like to buy a\npotion to heal her.");
    }

    // Spawning cursor
     gx_draw_sprite(mouse_x, mouse_y, &cursor);

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
                // crafting menu mouse interactions
                for (int i = 0; i < 5; i++) {
                    // distance check
                    if (distance(mouse_x, crafting_items[i].x, mouse_y, crafting_items[i].y) < 48) {
                        // select items from crafting menu
                        if (selected_item.item_id == 0 && crafting_items[i].item_id != 0) {
                            selected_item = crafting_items[i];
                            cursor = selected_item.sprite;
                            crafting_items[i].item_id = items[0].item_id;
                            crafting_items[i].name = items[0].name;
                            crafting_items[i].sprite = items[0].sprite;

                            if (i == 4) {
                                for (j = 0; j < 4; j++) {
                                    crafting_items[j].item_id = 0;
                                    crafting_items[j].name = items[0].name;
                                    crafting_items[j].sprite = items[0].sprite;
                                }
                            }
                        }

                        // drop items in crafting menu
                        else if (selected_item.item_id != 0 && crafting_items[i].item_id == 0) {
                            crafting_items[i].item_id = selected_item.item_id;
                            crafting_items[i].name = selected_item.name;
                            crafting_items[i].sprite = selected_item.sprite;
                            cursor = gx_make_sprite("cursor.png");
                            selected_item = items[0];
                        }

                        break;
                    }
                }
            }

            // Brewing
            else if (gameplay_state == 2) {
                if (brew_page == 0){
                    if (near_button == 1) {
                        brew_page = 1;
                        near_button = 0;
                    }
                }

                else if (brew_page == 1) {
                    // Checking brewing items and potion liquids slots
                    for (int i = 0; i < 3; i++) {
                        // Distance check
                        if (distance(mouse_x, brewing_items[i].x, mouse_y, brewing_items[i].y) < 48) {
                            // Select items from brewing menu
                            if (selected_item.item_id == 0 && brewing_items[i].item_id != 0) {
                                selected_item = brewing_items[i];
                                cursor = selected_item.sprite;
                                brewing_items[i].item_id = items[0].item_id;
                                brewing_items[i].name = items[0].name;
                                brewing_items[i].sprite = items[0].sprite;
                            }

                            // Drop items in brewing menu
                            else if (selected_item.item_id != 0 && brewing_items[i].item_id == 0) {
                                brewing_items[i].item_id = selected_item.item_id;
                                brewing_items[i].name = selected_item.name;
                                brewing_items[i].sprite = selected_item.sprite;
                                cursor = gx_make_sprite("cursor.png");
                                selected_item = items[0];
                            }

                            break;
                        }

                        if (distance(mouse_x, potion_liquids[i].x, mouse_y, potion_liquids[i].y) < 48) {
                            // Select items from potion liquids
                            if (selected_item.item_id == 0 && potion_liquids[i].item_id != 0) {
                                selected_item = potion_liquids[i];
                                cursor = selected_item.sprite;
                                potion_liquids[i].item_id = items[0].item_id;
                                potion_liquids[i].name = items[0].name;
                                potion_liquids[i].sprite = items[0].sprite;
                            }

                            // Drop items in potion liquids
                            else if (selected_item.item_id != 0 && potion_liquids[i].item_id == 0) {
                                potion_liquids[i].item_id = selected_item.item_id;
                                potion_liquids[i].name = selected_item.name;
                                potion_liquids[i].sprite = selected_item.sprite;
                                cursor = gx_make_sprite("cursor.png");
                                selected_item = items[0];
                            }

                            break;
                        }

                        if (near_button == 1) {
                            brew_page = 2;
                            near_button = 0;
                        }
                    }
                }


                else if (brew_page == 2) {
                    if (near_button != 0) {
                        if (near_button < 4) {
                            temp = near_button;
                        }
                        else {
                            speed = near_button - 3;
                        }
                    }

                    if (distance(mouse_x, 600, mouse_y, 640 < 48)) {
                        // Select items from result
                        if (selected_item.item_id == 0 && potion_result.item_id != 0) {
                            selected_item = potion_result;
                            cursor = selected_item.sprite;
                            potion_result.item_id = items[0].item_id;
                            potion_result.name = items[0].name;
                            potion_result.sprite = items[0].sprite;
                            temp = 0;
                            speed = 0;
                        }

                        // Drop items in result
                        else if (selected_item.item_id == 8 && potion_result.item_id == 0) {
                            potion_result.item_id = selected_item.item_id;
                            potion_result.name = selected_item.name;
                            potion_result.sprite = selected_item.sprite;
                            cursor = gx_make_sprite("cursor.png");
                            selected_item = items[0];
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
                        if (selected_item.item_id == 0 && world_containers[0].container_items[i].item_id != 0) {
                            selected_item = world_containers[0].container_items[i];
                            cursor = selected_item.sprite;
                            world_containers[0].container_items[i].item_id = items[0].item_id;
                            world_containers[0].container_items[i].name = items[0].name;
                            world_containers[0].container_items[i].sprite = items[0].sprite;
                        }

                        // Drop items in container
                        else if (selected_item.item_id != 0 && world_containers[0].container_items[i].item_id == 0) {
                            world_containers[0].container_items[i].item_id = selected_item.item_id;
                            world_containers[0].container_items[i].name = selected_item.name;
                            world_containers[0].container_items[i].sprite = selected_item.sprite;
                            cursor = gx_make_sprite("cursor.png");
                            selected_item = items[0];
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
                    if (selected_item.item_id == 0 && inv_items[i].item_id != 0) {
                        selected_item = inv_items[i];
                        cursor = selected_item.sprite;
                        inv_items[i].item_id = items[0].item_id;
                        inv_items[i].name = items[0].name;
                        inv_items[i].sprite = items[0].sprite;
                    }

                    // Drop items in inventory bar
                    else if (selected_item.item_id != 0 && inv_items[i].item_id == 0) {
                        item* selected_item_ptr;
                        selected_item_ptr = &selected_item;
                        cursor = gx_make_sprite("cursor.png");
                        inv_items[i] = add_inv(inv_items[i].x, inv_items[i].y, inv_items[i], selected_item, selected_item_ptr);
                        selected_item = items[0];
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
            right_pressed = 1;
            break;

        case SAPP_KEYCODE_LEFT:
            left_pressed = 1;
            break;

        case SAPP_KEYCODE_DOWN:
            down_pressed = 1;
            break;

        case SAPP_KEYCODE_UP:
            up_pressed = 1;
            break;

        case SAPP_KEYCODE_E:
            // Distance check player to door
            if (current_map_id == 0) {
                if (distance(player_x, door_x, player_y, door_y) < 64) {
                    current_map_id = 1;
                    player.width *= 1.5;
                    player.height *= 1.5;
                    player_x = TOWER_DOOR_X;
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
                                        item* old_item_ptr;
                                        old_item_ptr = &crafting_items[i];
                                        inv_items[j] = add_inv(inv_items[j].x, inv_items[j].y, inv_items[j], crafting_items[i], old_item_ptr);
                                        break;
                                    }
                                }
                                crafting_items[i].item_id = items[0].item_id;
                                crafting_items[i].name = items[0].name;
                                crafting_items[i].sprite = items[0].sprite;
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
                        if (brew_page == 1) {
                            for (int i = 0; i < 3; i++) {
                                if (brewing_items[i].item_id != 0) {
                                    if (i != 2)
                                    {
                                        for (j = 0; j < 8; j++) {
                                            if (inv_items[j].item_id == 0) {
                                                item* old_item_ptr;
                                                old_item_ptr = &brewing_items[i];
                                                inv_items[j] = add_inv(inv_items[j].x, inv_items[j].y, inv_items[j], brewing_items[i], old_item_ptr);
                                                break;
                                            }
                                        }
                                        brewing_items[i].item_id = items[0].item_id;
                                        brewing_items[i].name = items[0].name;
                                        brewing_items[i].sprite = items[0].sprite;
                                    }

                                    else {
                                        int id;

                                        id = brewing_items[2].item_id - 20;
                                        potion_liquids[id].item_id = brewing_items[2].item_id;
                                        potion_liquids[id].name = brewing_items[2].name;
                                        potion_liquids[id].sprite = brewing_items[2].sprite;

                                        brewing_items[2].item_id = items[0].item_id;
                                        brewing_items[2].name = items[0].name;
                                        brewing_items[2].sprite = items[0].sprite;
                                    }
                                }

                            }

                            if (brew_page == 2) {
                                if (potion_result.item_id != 0) {
                                    for (int i = 0; i < 8; j++) {
                                        if (inv_items[i].item_id == 0) {
                                            item* old_item_ptr;
                                            old_item_ptr = &potion_result;
                                            inv_items[i] = add_inv(inv_items[i].x, inv_items[i].y, inv_items[i], potion_result, old_item_ptr);
                                            break;
                                        }
                                    }
                                    potion_result.item_id = items[0].item_id;
                                    potion_result.name = items[0].name;
                                    potion_result.sprite = items[0].sprite;
                                }
                            }
                        }

                        gameplay_state = 0;
                        brew_page = 0;
                        near_button = 0;
                        show_continue = false;
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

    case SAPP_EVENTTYPE_KEY_UP:
        switch (ev->key_code) {
        case SAPP_KEYCODE_RIGHT:
            right_pressed = 0;
            break;
        case SAPP_KEYCODE_LEFT:
            left_pressed = 0;
            break;
        case SAPP_KEYCODE_DOWN:
            down_pressed = 0;
            break;
        case SAPP_KEYCODE_UP:
            up_pressed = 0;
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
