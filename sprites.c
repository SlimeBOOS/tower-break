typedef enum SpriteID {
	SPRITE_nil,
	SPRITE_player,
	SPRITE_home_tower,
	SPRITE_rock,
	SPRITE_rock_nugget,
	SPRITE_MAX
} SpriteID;
typedef struct Sprite {
	Gfx_Image *image;
	Vector2 pivot;
} Sprite;
Sprite sprites[SPRITE_MAX];

Sprite *sprites_get(SpriteID sprite_id)
{
	if (0 <= sprite_id && sprite_id < SPRITE_MAX) {
		return &sprites[sprite_id];
	}

	log_warning("Sprite ID out of bounds");
	// TODO: Add warning if this occured?
	return &sprites[SPRITE_nil];
}

Gfx_Image *load_image_from_disk_assert(string path)
{
	Gfx_Image* image = load_image_from_disk(path, get_heap_allocator());
	assert(image != NULL, "Failed to load '%s'", path);
	return image;
}

Vector2 sprite_get_size(Sprite *sprite)
{
	Gfx_Image *image = sprite->image;
	return v2(image->width, image->height);
}

void sprite_draw(Sprite *sprite, Vector2 position)
{
	Gfx_Image *image = sprite->image;
	Vector2 image_size = sprite_get_size(sprite);
	Vector2 image_pivot = v2_mul(image_size, sprite->pivot);

	draw_image(image, v2_sub(position, image_pivot), image_size, COLOR_WHITE);
}

void sprites_init()
{
	sprites[SPRITE_nil]         = (Sprite){ .image = load_image_from_disk_assert(STR("assets/missing-sprite.png")), .pivot = v2(0.5, 0.5) };
	sprites[SPRITE_player]      = (Sprite){ .image = load_image_from_disk_assert(STR("assets/player.png"))        , .pivot = v2(0.5,   0) };
	sprites[SPRITE_home_tower]  = (Sprite){ .image = load_image_from_disk_assert(STR("assets/home-tower.png"))    , .pivot = v2(0.5,   0) };
	sprites[SPRITE_rock]        = (Sprite){ .image = load_image_from_disk_assert(STR("assets/rock.png"))          , .pivot = v2(0.5,   0) };
	sprites[SPRITE_rock_nugget] = (Sprite){ .image = load_image_from_disk_assert(STR("assets/rock-nugget.png"))   , .pivot = v2(0.5, 0.5) };
}