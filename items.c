typedef enum {
	ITEM_nil,
	ITEM_rock_nugget,
	ITEM_MAX
} ItemID;
typedef struct {
	string name;
	SpriteID sprite_id;
	u32 max_stack_size;
} Item;
Item items[ITEM_MAX];

Item *items_get(ItemID item_id)
{
	if (0 <= item_id && item_id < ITEM_MAX) {
		return &items[item_id];
	}

	// TODO: Add warning if this occured?
	return &items[ITEM_nil];
}


void items_init()
{
    items[ITEM_nil] = (Item){
        .name = STR("Nil"),
        .sprite_id = SPRITE_nil,
		.max_stack_size = 0
    };

    items[ITEM_rock_nugget] = (Item){
        .name = STR("Rock nugget"),
        .sprite_id = SPRITE_rock_nugget,
		.max_stack_size = 20
    };
}