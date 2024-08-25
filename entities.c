typedef enum EntityArchetype {
	ARCH_nil        = 0,
	ARCH_home_tower = 1,
	ARCH_player     = 2,
	ARCH_rock       = 3,
	ARCH_item       = 4
} EntityArchetype;

typedef struct Entity {
	bool is_valid;
	EntityArchetype arch;
	Vector2 pos;

	bool render_sprite;
	SpriteID sprite_id;

	bool breakable;
	u32 break_progress;
	u32 break_health;
	ItemID break_item;

	ItemID item_id;
} Entity;

Rect entity_get_bounds(Entity *entity)
{
	Rect rect = { 0 };
	rect.pos = entity->pos;

	if (entity->render_sprite) {
		Sprite *sprite = sprites_get(entity->sprite_id);
		Gfx_Image *image = sprite->image;
		Vector2 image_size = v2(image->width, image->height);

		rect.pos = v2_sub(rect.pos, v2_mul(image_size, sprite->pivot));
		rect.size = image_size;
	}

	return rect;
}

void entity_player_init(Entity *entity)
{
	entity->arch = ARCH_player;
	entity->render_sprite = true;
	entity->sprite_id = SPRITE_player;
}

void entity_home_tower_init(Entity *entity)
{
	entity->arch = ARCH_home_tower;
	entity->render_sprite = true;
	entity->sprite_id = SPRITE_home_tower;
}

void entity_rock_init(Entity *entity)
{
	entity->arch = ARCH_rock;
	entity->render_sprite = true;
	entity->sprite_id = SPRITE_rock;

	entity->breakable = true;
	entity->break_health = 3;
	entity->break_progress = 0;
	entity->break_item = ITEM_rock_nugget;
}

void entity_item_init(Entity *entity, ItemID item_id)
{
	entity->arch = ARCH_item;
	entity->render_sprite = true;
	entity->sprite_id = items_get(item_id)->sprite_id;
	entity->item_id = item_id;
}

void entity_init(Entity *entity, EntityArchetype arch)
{
    switch (arch) {
        case ARCH_nil: {
            break;
		}
        case ARCH_player: {
            entity_player_init(entity);
            break;
		}
        case ARCH_home_tower: {
            entity_home_tower_init(entity);
            break;
		}
        case ARCH_rock: {
            entity_rock_init(entity);
            break;
		}
		case ARCH_item: {
            entity_item_init(entity, ITEM_nil);
            break;
		}
        default: panic("Unknown entity archetype");
    }
}