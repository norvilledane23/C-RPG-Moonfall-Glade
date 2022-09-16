#pragma once

#define INV_SIZE 8
#define	WORLD_SIZE 20
#define STACK_LIMIT 30

typedef struct {
	int item_id;
	int amount;
	char* name;
	float x;
	float y;
	float z; // Displacement value
} item;

int inv_item_stack(item inv_items[], int current_w_item_id) {

	for (int j = 0; j < INV_SIZE; j++) {
		if (inv_items[j].item_id == current_w_item_id) {
			return j;
		}
	}

	return -1;
}