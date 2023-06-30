// Student Name: Norville Amao
#pragma once

void potion_selection(float mouse_x, float mouse_y, int *near_button, gx_sprite brew_menu, gx_sprite pbutton1, gx_sprite pbutton1_h);
void ingredient_selection(float mouse_x, float mouse_y, bool show_continue, int* near_button, gx_sprite brew_UI_sprites[], gx_sprite brew_continue_h, item brewing_items[], item potion_liquids[]);
void cooking_selection(float mouse_x, float mouse_y, int* near_button, item potion_result, gx_sprite brew_UI_sprites[], gx_sprite brew_UI_sprites_h[]);

// Potion selection page (Page 1)
void potion_selection(float mouse_x, float mouse_y, int *near_button, gx_sprite brew_menu, gx_sprite pbutton1, gx_sprite pbutton1_h) {
    gx_draw_sprite((1200 / 2) - (brew_menu.width / 2), (900 / 2) - (brew_menu.height / 2), &brew_menu);

    quadrilateral potion_button;
    potion_button.x1 = 360;
    potion_button.x2 = 603;
    potion_button.y1 = 370;
    potion_button.y2 = 425;

    if (simple_mouse_collision(potion_button, mouse_x, mouse_y)) {
        gx_draw_sprite(360, 370, &pbutton1_h);
        *near_button = 1;
    }

    else {
        gx_draw_sprite(360, 370, &pbutton1);
        *near_button = 0;
    }
}

// Ingredient selection page (Page 2)
void ingredient_selection(float mouse_x, float mouse_y, bool show_continue, int* near_button, gx_sprite brew_UI_sprites[], gx_sprite brew_continue_h, item brewing_items[], item potion_liquids[]) {
    gx_sprite brew_menu2 = brew_UI_sprites[11];
    gx_sprite brew_box = brew_UI_sprites[13];
    gx_sprite brew_continue = brew_UI_sprites[14];
    
    gx_draw_sprite((1200 / 2) - (brew_menu2.width / 2), (900 / 2) - (brew_menu2.height / 2), &brew_menu2);
    gx_draw_sprite(384, 336, &brew_box);
    gx_draw_sprite(384, 436, &brew_box);
    gx_draw_sprite(384, 536, &brew_box);
    gx_draw_sprite(610, 336, &brew_box);
    gx_draw_sprite(710, 336, &brew_box);
    gx_draw_sprite(660, 436, &brew_box);

    for (int i = 0; i < 3; i++) {
        if (brewing_items[i].item_id != 0) {
            gx_draw_sprite(brewing_items[i].x - 30, brewing_items[i].y - 30, &brewing_items[i].sprite);
        }

        if (potion_liquids[i].item_id != 0)
        {
            gx_draw_sprite(potion_liquids[i].x - 30, potion_liquids[i].y - 30, &potion_liquids[i].sprite);
        }
    }

    if (show_continue)
    {
        quadrilateral potion_button;
        potion_button.x1 = 580;
        potion_button.x2 = 823;
        potion_button.y1 = 550;
        potion_button.y2 = 605;

        if (simple_mouse_collision(potion_button, mouse_x, mouse_y)) {
            gx_draw_sprite(580, 550, &brew_continue_h);
            *near_button = 1;
        }

        else {
            gx_draw_sprite(580, 550, &brew_continue);
            *near_button = 0;
        }
    }
}

// Cooking temperature and speed selection page (Page 3)
void cooking_selection(float mouse_x, float mouse_y, int* near_button, item potion_result, gx_sprite brew_UI_sprites[], gx_sprite brew_UI_sprites_h[]) {
    gx_sprite brew_menu3 = brew_UI_sprites[12];
    gx_sprite brew_box = brew_UI_sprites[13];

    gx_draw_sprite((1200 / 2) - (brew_menu3.width / 2), (900 / 2) - (brew_menu3.height / 2), &brew_menu3);
    gx_draw_sprite((1200 / 2) - (brew_box.width / 2), 600, &brew_box);

    for (int i = 0; i < 6; i++)
    {
        quadrilateral potion_button;

        if (i < 3)
        {
            potion_button.x1 = 350;
            potion_button.x2 = 593;
        }

        else
        {
            potion_button.x1 = 607;
            potion_button.x2 = 850;
        }

        potion_button.y1 = 390 + ((i % 3) * 65);
        potion_button.y2 = potion_button.y1 + 55;

        if (simple_mouse_collision(potion_button, mouse_x, mouse_y)) {
            gx_draw_sprite(potion_button.x1, potion_button.y1, &brew_UI_sprites_h[i + 4]);
            *near_button = i + 1;
        }

        else {
            gx_draw_sprite(potion_button.x1, potion_button.y1, &brew_UI_sprites[i + 4]);
        }
    }

    if (potion_result.item_id != 0)
    {
        gx_draw_sprite(potion_result.x - 30, potion_result.y - 30, &potion_result.sprite);
    }
}
