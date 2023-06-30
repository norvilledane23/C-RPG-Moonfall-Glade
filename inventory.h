// Student Name: Norville Amao

#pragma once

#define INV_SIZE 8
#define STACK_LIMIT 30

typedef struct {
	int item_id;
	int amount;
	float x;
	float y;
	char* name;
	gx_sprite sprite;
} item;

int empty_slot(item inv_items[]);
int item_check(item inv_items[], int world_item_id);
item add_inv(float inv_x, float inv_y, item new_item, item old_item, item *world_item);

int empty_slot(item inv_items[]) {
	for (int i = 0; i < INV_SIZE; i++) {
		if (inv_items[i].item_id == 0) {
			return i;
		}
	}

	return -1;
}

int item_check(item inv_items[], int world_item_id) {
	// Check if the picked up item already exists in inventory
	for (int i = 0; i < INV_SIZE; i++) {
		if (inv_items[i].item_id == world_item_id) {
			// Returns index if it does
			return i;
		}
	}

	return -1;
}

item add_inv(float inv_x, float inv_y, item new_item, item old_item, item *old_item_ptr) {
	old_item.x = inv_x;
	old_item.y = inv_y;
	new_item = old_item;

	new_item.amount = 1;
	old_item_ptr->item_id = 0;
	old_item_ptr->sprite = gx_make_sprite("");

	return new_item;
}
