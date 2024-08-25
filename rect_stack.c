typedef enum RectStackDirection {
    STACK_LEFT_TO_RIGHT,
} RectStackDirection;

typedef struct RectStack {
    Rect container;
    f32 gap;
    RectStackDirection direction;
    Vector2 used_size;
} RectStack;

Rect rect_stack_next(RectStack* rect_stack, f32 size)
{
    Vector2 child_pos = rect_stack->container.pos;
    Vector2 child_size;
    switch (rect_stack->direction)
    {
    case STACK_LEFT_TO_RIGHT: {
        child_pos.x += rect_stack->used_size.x;
        child_size = v2(size, rect_stack->container.size.y);

        rect_stack->used_size.x += child_size.x + rect_stack->gap;
        rect_stack->used_size.y = max(rect_stack->used_size.y, child_size.y);
        break;
    }
    default: panic("Unknown rect stack direction");
    }

    return (Rect){
        .pos = child_pos,
        .size = child_size
    };
}