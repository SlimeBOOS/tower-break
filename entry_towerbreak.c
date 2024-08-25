
bool debug_entity = false;
bool debug_cheat_mode = true;
f32 player_item_pickup_distance = 24;

typedef struct InventorySlot {
	bool is_used;
	ItemID item_id;
	u32 count;
} InventorySlot;

#define MAX_ENTITIES 64
#define MAX_INVENTORY_SLOTS 5
typedef struct World {
	Entity entities[MAX_ENTITIES];

	InventorySlot inventory[MAX_INVENTORY_SLOTS];
} World;

World *world = NULL;

Entity *entity_create(EntityArchetype arch)
{
	for (int i = 0; i < MAX_ENTITIES; i++) {
		Entity *entity = &world->entities[i];
		if (!entity->is_valid) {
			entity->is_valid = true;
			entity_init(entity, arch);
			return entity;
		}
	}

	panic("Failed to create entity, no empty slots");
}

void entity_destroy(Entity *entity)
{
	memset(entity, 0, sizeof(Entity));
}

bool almost_equals(f32 a, f32 b, f32 epsilon)
{
	return fabs(a - b) <= epsilon;
}

bool animate_f32_to_target(f32 *value, f32 target, f32 dt, f32 speed)
{
	*value += (target - *value) * (1 - pow(2, -speed * dt));

	if (almost_equals(*value, target, 1e-3)) {
		*value = target;
		return true;
	}

	return false;
}

bool animate_v2_to_target(Vector2 *value, Vector2 target, f32 dt, f32 speed)
{
	bool x_reached_target = animate_f32_to_target(&value->x, target.x, dt, speed);
	bool y_reached_target = animate_f32_to_target(&value->y, target.y, dt, speed);

	return x_reached_target && y_reached_target;
}

Vector2 get_world_mouse()
{
	Vector2 screen_size = v2(window.width, window.height);
	Vector2 mouse_screen = v2(input_frame.mouse_x, input_frame.mouse_y);

	Vector2 ndc = v2_div(mouse_screen, screen_size);
	ndc         = v2_mulf(ndc, 2);
	ndc         = v2_sub(ndc, v2(1, 1));

	Vector4 world_pos = v4(ndc.x, ndc.y, 0, 1);
	world_pos         = m4_transform(m4_inverse(draw_frame.projection), world_pos);
	world_pos         = m4_transform(draw_frame.view, world_pos);

	return world_pos.xy;
}

bool inventory_pickup_item(ItemID item_id)
{
	for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
		InventorySlot *slot = &world->inventory[i];
		if (slot->is_used && slot->item_id == item_id && slot->count < items_get(item_id)->max_stack_size) {
			slot->count++;
			return true;
		}
	}

	for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
		InventorySlot *slot = &world->inventory[i];
		if (!slot->is_used) {
			slot->count = 1;
			slot->is_used = true;
			slot->item_id = item_id;
			return true;
		}
	}

	return false;
}

void draw_text_pivot(Gfx_Font *font, string text, u32 raster_height, Vector2 position, Vector2 scale, Vector4 color, Vector2 pivot)
{
	Gfx_Text_Metrics text_metrics = measure_text(font, text, raster_height, scale);

	position = v2_sub(position, text_metrics.visual_pos_min);
	position = v2_sub(position, v2_mul(text_metrics.visual_size, pivot));

	draw_text(font, text, raster_height, position, scale, color);
}

Vector2 tile_size = (Vector2){ .x = 8, .y = 8 };

Vector2 v2_div_floor(Vector2 a, Vector2 b)
{
	Vector2 c = v2_div(a, b);
	c.x = floorf(c.x);
	c.y = floorf(c.y);
	return c;
}

Vector2 v2_div_ceil(Vector2 a, Vector2 b)
{
	Vector2 c = v2_div(a, b);
	c.x = ceilf(c.x);
	c.y = ceilf(c.y);
	return c;
}

Vector2 v2_round_multiple(Vector2 a, Vector2 b)
{
	return v2_mul(v2_div_floor(a, b), b);
}

Vector2 world_pos_to_tile_pos(Vector2 world_pos)
{
	return v2_div_floor(world_pos, tile_size);
}

Vector2 round_world_pos_to_tile(Vector2 world_pos)
{
	return v2_round_multiple(world_pos, tile_size);
}

Vector4 color_blend_opacity(Vector4 color, float opacity)
{
	return v4_mul(color, v4(1, 1, 1, opacity));
}

void draw_tile_grid(Vector4 color, Vector2 bottom_left, Vector2 size)
{
	Vector2 bottom_left_tile = v2_div_floor(bottom_left, tile_size);
	Vector2 top_right_tile = v2_div_ceil(v2_add(bottom_left, size), tile_size);

	for (int y = bottom_left_tile.y; y < top_right_tile.y; y++) {
		for (int x = bottom_left_tile.x; x < top_right_tile.x; x++) {
			if ((x + y) %  2 != 0) continue;

			draw_rect(v2_mul(v2(x, y), tile_size), tile_size, color);
		}
	}
}

void draw_circle_centered(Vector2 center, f32 radius, Vector4 color)
{
	Vector2 circle_size = v2(radius, radius);
	draw_circle(v2_sub(center, v2_divf(circle_size, 2)), circle_size, color);
}

Entity *entity_get_at_tile(Vector2 pos)
{
	Rect tile_bounds = {
		.pos = v2_round_multiple(pos, tile_size),
		.size = tile_size
	};

	for (int i = 0; i < MAX_ENTITIES; i++) {
		Entity* entity = &world->entities[i];
		if (!entity->is_valid) continue;

		if (rect_is_v2_inside(tile_bounds, entity->pos)) {
			return entity;
		}
	}

	return NULL;
}

#define MAX_VIEW_MATRIX_STACK_SIZE 8
Matrix4 view_matrix_stack[MAX_VIEW_MATRIX_STACK_SIZE];
u32 view_matrix_stack_size = 0;

void push_view_matrix()
{
	assert(view_matrix_stack_size < MAX_VIEW_MATRIX_STACK_SIZE);
	view_matrix_stack[view_matrix_stack_size] = draw_frame.view;
	view_matrix_stack_size++;
}

void pop_view_matrix()
{
	assert(view_matrix_stack_size > 0);
	view_matrix_stack_size--;
	draw_frame.view = view_matrix_stack[view_matrix_stack_size];
}

int entry(int argc, char **argv) {
	Vector2 canvas_size = v2(320, 180);

	window.title = STR("Tower break");
	window.scaled_width = canvas_size.x*4; // We need to set the scaled size if we want to handle system scaling (DPI)
	window.scaled_height = canvas_size.y*4;
	window.x = 90;
	window.y = 200;
	window.clear_color = hex_to_rgba(0x222034ff);

	sprites_init();
	items_init();

	world = alloc(get_heap_allocator(), sizeof(World));
	assert(world != NULL);

	Entity* player_en = entity_create(ARCH_player);
	player_en->pos = v2_mul(v2(2.5, 2.5), tile_size);

	Entity* home_tower_en = entity_create(ARCH_home_tower);

	for (int i = 0; i < 10; i++) {
		Entity* rock_en = entity_create(ARCH_rock);
		rock_en->pos.x = floorf(get_random_float32_in_range(-10, 10)) + 0.5;
		rock_en->pos.y = floorf(get_random_float32_in_range(-10, 10)) + 0.1;
		rock_en->pos   = v2_mul(rock_en->pos, tile_size);
	}

	Vector2 camera_pos = player_en->pos;

	Gfx_Font* font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());
	assert(font != NULL);

	if (debug_cheat_mode) {
		inventory_pickup_item(ITEM_rock_nugget);
	}

	float64 last_time = os_get_current_time_in_seconds();
	while (!window.should_close) {
		f64 now = os_get_current_time_in_seconds();
		if ((int)now != (int)last_time) log("%.2f FPS (%.2fms)", 1.0/(now-last_time), (now-last_time)*1000);
		f64 dt = now - last_time;
		last_time = now;

		reset_temporary_storage();

		Rect visible_area;
		{
			f32 x_scale = (f32)window.width  / canvas_size.x;
			f32 y_scale = (f32)window.height / canvas_size.y;
			f32 min_scale = min(x_scale, y_scale);
			f32 visible_width = canvas_size.x * (x_scale/min_scale);
			f32 visible_height = canvas_size.y * (y_scale/min_scale);
			draw_frame.projection = m4_make_orthographic_projection(
				-visible_width/2 , visible_width/2,
				-visible_height/2, visible_height/2,
				-1, 10
			);
			visible_area.pos = v2(-visible_width/2, -visible_height/2);
			visible_area.size = v2(visible_width, visible_height);

			animate_v2_to_target(&camera_pos, player_en->pos, dt, 15);
			draw_frame.view = m4_scalar(1);
			draw_frame.view = m4_translate(draw_frame.view, v3(camera_pos.x, camera_pos.y, 0));
		}

		Vector2 mouse_pos = get_world_mouse();
		Entity *entity_under_mouse = entity_get_at_tile(mouse_pos);

		draw_tile_grid(
			color_blend_opacity(COLOR_BLACK, 0.5f),
			v2_sub(camera_pos, v2_divf(canvas_size, 2)),
			canvas_size
		);

		for (int i = 0; i < MAX_ENTITIES; i++) {
			Entity* entity = &world->entities[i];
			if (!entity->is_valid) continue;
			if (!entity->render_sprite) continue;

			switch (entity->arch)
			{

			default:
				{
					Sprite *sprite = sprites_get(entity->sprite_id);

					if (debug_entity) {
						Vector2 image_size = sprite_get_size(sprite);
						Vector2 image_pivot = v2_mul(image_size, sprite->pivot);

						draw_rect(v2_sub(entity->pos, image_pivot), image_size, color_blend_opacity(COLOR_RED, entity == entity_under_mouse ? 0.5f : 0.25f));
					}

					sprite_draw(sprite, entity->pos);
					if (debug_entity) {
						draw_circle_centered(entity->pos, 1, COLOR_RED);
					}

					break;
				}
			}
		}

		draw_line(v2(0, -3), v2(0, 3), 1, COLOR_RED);
		draw_line(v2(-3, 0), v2(3, 0), 1, COLOR_RED);

		Vector2 mouse_tile_pos = round_world_pos_to_tile(mouse_pos);
		draw_rect(mouse_tile_pos, tile_size, color_blend_opacity(COLOR_WHITE, 0.25f));

		{
			Vector4 offscreen_color = COLOR_BLACK;
			f32 offscreen_width = (visible_area.size.x - canvas_size.x)/2;
			f32 offscreen_height = (visible_area.size.y - canvas_size.y)/2;

			push_view_matrix();
			draw_frame.view = m4_scalar(1);

			draw_rect(visible_area.pos, v2(offscreen_width, visible_area.size.y), offscreen_color);
			draw_rect(v2_add(visible_area.pos, v2(visible_area.size.x - offscreen_width, 0)), v2(offscreen_width, visible_area.size.y), offscreen_color);
			draw_rect(visible_area.pos, v2(visible_area.size.x, offscreen_height), offscreen_color);
			draw_rect(v2_add(visible_area.pos, v2(0, visible_area.size.y - offscreen_height)), v2(visible_area.size.x, offscreen_height), offscreen_color);
			pop_view_matrix();
		}

		// UI
		{
			push_view_matrix();
			draw_frame.view = m4_scalar(1);

			Rect screen_rect = {
				.pos = v2_divf(canvas_size, -2),
				.size = canvas_size
			};

			f32 slot_size = 16;
			RectStack slot_stack = {
				.container = rect_bottom_sub_rect(rect_shrink(screen_rect, 8), slot_size),
				.direction = STACK_LEFT_TO_RIGHT,
				.gap = 8
			};

			for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
				InventorySlot *slot = &world->inventory[i];
				Rect slot_rect = rect_stack_next(&slot_stack, slot_size);
				Vector2 slot_center = rect_center(slot_rect);

				draw_rect(slot_rect.pos, slot_rect.size, color_blend_opacity(COLOR_BLACK, 0.5f));
				if (slot->is_used) {
					Item *item = items_get(slot->item_id);
					Sprite *sprite = sprites_get(item->sprite_id);
					sprite_draw(sprite, slot_center);

					draw_text_pivot(
						font,
						sprint(get_temporary_allocator(), "%d/%d", slot->count, item->max_stack_size),
						32,
						v2_add(slot_center, v2(0, slot_size/2)),
						v2(0.2, 0.2),
						COLOR_WHITE,
						v2(0.5, 0.5)
					);
				}
			}

			pop_view_matrix();
		}

		for (int i = 0; i < MAX_ENTITIES; i++) {
			Entity* entity = &world->entities[i];
			if (!entity->is_valid) continue;
			if (entity->arch != ARCH_item) continue;

			f32 distance = v2_length(v2_sub(entity->pos, player_en->pos));
			if (distance > player_item_pickup_distance) continue;

			if (inventory_pickup_item(entity->item_id)) {
				entity_destroy(entity);
			}
		}

		if (is_key_just_pressed(KEY_ESCAPE)) {
			window.should_close = true;
		}

		Vector2 move_dir = v2(0, 0);
		if (is_key_down('A')) {
			move_dir.x -= 1.0;
		}
		if (is_key_down('D')) {
			move_dir.x += 1.0;
		}
		if (is_key_down('S')) {
			move_dir.y -= 1.0;
		}
		if (is_key_down('W')) {
			move_dir.y += 1.0;
		}
		move_dir = v2_normalize(move_dir);

		player_en->pos = v2_add(player_en->pos, v2_mulf(move_dir, 32.0 * dt));

		if (entity_under_mouse && entity_under_mouse->breakable && is_key_just_pressed(MOUSE_BUTTON_LEFT)) {
			consume_key_just_pressed(MOUSE_BUTTON_LEFT);

			entity_under_mouse->break_progress++;
			if (entity_under_mouse->break_progress >= entity_under_mouse->break_health) {
				Entity* item_en = entity_create(ARCH_nil);
				entity_item_init(item_en, entity_under_mouse->break_item);
				item_en->pos = entity_under_mouse->pos;

				entity_under_mouse->break_progress = 0;
			}
		}

		os_update();
		gfx_update();
	}

	return 0;
}