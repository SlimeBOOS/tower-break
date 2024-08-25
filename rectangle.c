
typedef struct Rect {
    Vector2 pos;
    Vector2 size;
} Rect;

bool rect_is_v2_inside(Rect rect, Vector2 point)
{
    return (rect.pos.x <= point.x && point.x <= rect.pos.x + rect.size.x) &&
            (rect.pos.y <= point.y && point.y <= rect.pos.y + rect.size.y);
}

Rect rect_shrink(Rect rect, f32 amount)
{
    return (Rect){
        .pos  = v2_add(rect.pos , v2(  amount,   amount)),
        .size = v2_sub(rect.size, v2(2*amount, 2*amount))
    };
}

Rect rect_bottom_sub_rect(Rect container, f32 height)
{
    return (Rect){
        .pos = container.pos,
        .size = v2(container.size.x, height)
    };
}

Vector2 rect_center(Rect rect)
{
    return v2_add(rect.pos, v2_divf(rect.size, 2));
}