#include "xtra.h"


HANDLE hstdout;
HANDLE hstdin;

DWORD old_buttons = 0;



// compile

#ifndef LIB_NO_DLL /* compilation as library */

XTRA_API uint16_t FG(bool r, bool g, bool b)
{
    return (
        (r ? FOREGROUND_RED : 0) |
        (g ? FOREGROUND_GREEN : 0) |
        (b ? FOREGROUND_BLUE : 0)
    );
}

XTRA_API uint16_t BG(bool r, bool g, bool b)
{
    return (
        (r ? BACKGROUND_RED : 0) |
        (g ? BACKGROUND_GREEN : 0) |
        (b ? BACKGROUND_BLUE : 0)
    );
}

XTRA_API Rect get_parent_rect(Widget *w)
{
    return w->parent->calc_layout(w->parent);
}

XTRA_API Widget *find_rel_widget(Widget *w, const char *id)
{
    return find_child(w->parent, id);
}

#endif /* LIB_NO_DLL */


/* main code */

/* default layout & render functions */

Rect frame_connected_widget_calc_layout(Widget *w)
{
    Rect rect = w->frame->calc_layout(w->frame);
    
    return layout_wrapper(rect.x + 1, rect.y + 1, rect.w - 2, rect.h - 2);
}

void render_frame_default_func(App *app, Widget *w)
{
    Rect rect = w->calc_layout(w);

    uint32_t c;
    uint16_t attr = FG(1, 0, 1);
    
    c = BOX_TL; addstr(app, w, 0, 0, &c, 1, attr);
    c = BOX_TR; addstr(app, w, rect.w - 1, 0, &c, 1, attr);
    c = BOX_BL; addstr(app, w, 0, rect.h - 1, &c, 1, attr);
    c = BOX_BR; addstr(app, w, rect.w - 1, rect.h - 1, &c, 1, attr);
    
    c = BOX_H;
    
    for (int i = 1; i < rect.w - 1; i++)
    {
        addstr(app, w, i, 0, &c, 1, attr);
        addstr(app, w, i, rect.h - 1, &c, 1, attr);
    }
    
    c = BOX_V;
    
    for (int i = 1; i < rect.h - 1; i++)
    {
        addstr(app, w, 0, i, &c, 1, attr);
        addstr(app, w, rect.w - 1, i, &c, 1, attr);
    }
}

/* helpers */

Widget *find_root_widget(Widget *w)
{
    while (w->parent != NULL)
        w = w->parent;
    return w;
}

Key translate_key(WORD vk)
{
    switch (vk) {
        case VK_UP:    return KEY_UP;
        case VK_DOWN:  return KEY_DOWN;
        case VK_LEFT:  return KEY_LEFT;
        case VK_RIGHT: return KEY_RIGHT;

        case VK_RETURN: return KEY_ENTER;
        case VK_ESCAPE: return KEY_ESCAPE;

        default:
            return KEY_UNKNOWN;
    }
}

Widget *deepest_right(Widget *w)
{
    while (w->child_count > 0) {
        w = w->children[w->child_count - 1];
    }
    
    return w;
}

XTRA_API Widget *find_child(Widget *w, const char *id)
{
    //printf("in find_child\n");
    
    for (uint32_t i = 0; i < w->child_count; i++)
    {
        //printf("w->children[i]->id: %s\n", w->children[i]->id);
        
        if (strcmp(w->children[i]->id, id) == 0)
            return w->children[i];
    }
    
    return NULL;
}

/* encoding */

XTRA_API int utf8_decode(const char *source, uint32_t *out)
{
    const uint8_t *p = (const uint8_t *)source;

    if (p[0] < 0x80)
    {
        *out = p[0];
        return 1;
    }

    // 2-byte sequence
    if ((p[0] & 0xE0) == 0xC0)
    {
        if ((p[1] & 0xC0) != 0x80)
            return -1;

        uint32_t cp =
            ((uint32_t)(p[0] & 0x1F) << 6) |
            (uint32_t)(p[1] & 0x3F);

        // Overlong encoding
        if (cp < 0x80)
            return -1;

        *out = cp;
        return 2;
    }

    // 3-byte sequence
    if ((p[0] & 0xF0) == 0xE0)
    {
        if ((p[1] & 0xC0) != 0x80 ||
            (p[2] & 0xC0) != 0x80)
            return -1;

        uint32_t cp =
            ((uint32_t)(p[0] & 0x0F) << 12) |
            ((uint32_t)(p[1] & 0x3F) << 6) |
            (uint32_t)(p[2] & 0x3F);

        // Overlong encoding
        if (cp < 0x800)
            return -1;

        // UTF-16 surrogate range is not valid Unicode scalar value
        if (cp >= 0xD800 && cp <= 0xDFFF)
            return -1;

        *out = cp;
        return 3;
    }

    // 4-byte sequence
    if ((p[0] & 0xF8) == 0xF0)
    {
        if ((p[1] & 0xC0) != 0x80 ||
            (p[2] & 0xC0) != 0x80 ||
            (p[3] & 0xC0) != 0x80)
            return -1;

        uint32_t cp =
            ((uint32_t)(p[0] & 0x07) << 18) |
            ((uint32_t)(p[1] & 0x3F) << 12) |
            ((uint32_t)(p[2] & 0x3F) << 6) |
            (uint32_t)(p[3] & 0x3F);

        // Overlong encoding
        if (cp < 0x10000)
            return -1;

        // Unicode ends at U+10FFFF
        if (cp > 0x10FFFF)
            return -1;

        *out = cp;
        return 4;
    }

    return -1;
}

XTRA_API int unicode_to_utf8(uint32_t codepoint, char out[4]) // the opposite of utf8_decode
{
    if (codepoint <= 0x7F)
    {
        out[0] = (char)codepoint;
        return 1;
    }

    if (codepoint <= 0x7FF)
    {
        out[0] = (char)(0xC0 | (codepoint >> 6));
        out[1] = (char)(0x80 | (codepoint & 0x3F));
        return 2;
    }

    if (codepoint <= 0xFFFF)
    {
        // UTF-16 surrogate range is not a valid Unicode codepoint
        if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
            return -1;

        out[0] = (char)(0xE0 | (codepoint >> 12));
        out[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        out[2] = (char)(0x80 | (codepoint & 0x3F));
        return 3;
    }

    if (codepoint <= 0x10FFFF)
    {
        out[0] = (char)(0xF0 | (codepoint >> 18));
        out[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
        out[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        out[3] = (char)(0x80 | (codepoint & 0x3F));
        return 4;
    }

    return -1;
}

XTRA_API int unicode_to_wchar(uint32_t codepoint, WCHAR out[2])
{
    // bmp
    
    if (codepoint <= 0xFFFF)
    {
        // surrogates aren't valid codepoints
        if (codepoint >= UTF16_SURROGATE_START &&
            codepoint <= UTF16_SURROGATE_END)
        {
            return -1;
        }

        out[0] = (WCHAR)codepoint;
        return 1;
    }

    // unicode ends at U+10FFFF
    if (codepoint > 0x10FFFF)
        return -1;

    codepoint -= 0x10000;

    out[0] = (WCHAR)(0xD800 + (codepoint >> 10));
    out[1] = (WCHAR)(0xDC00 + (codepoint & 0x3FF));

    return 2;
}

// works with strings

XTRA_API int utf8_to_unicode(const char *s, uint32_t *buffer, size_t bufsize)
{
    uint32_t out;
    int      charlen;
    
    size_t   buf_index = 0;
    
    bufsize -= 1; // so that there is space for '\0'
    
    while (*s && buf_index < bufsize)
    {
        charlen = utf8_decode(s, &out);
        
        buffer[buf_index] = out;
        
        s += charlen;
        buf_index += 1;
    }

    buffer[buf_index] = (uint32_t)'\0';
    
    return buf_index;
}

// compare strings

XTRA_API int unicode_strcmp_utf8(const uint32_t *codepoints, const char *s)
{
    while (*codepoints)
    {
        uint32_t cp;
        
        int len = utf8_decode(s, &cp);
        
        assert(len > 0);
        
        if (*codepoints != cp)
            return *codepoints > cp ? 1 : -1;
        
        codepoints++;
        s += len;
    }
    
    return *s == '\0' ? 0 : -1;
}

XTRA_API int unicode_strcmp_unicode(const uint32_t *s1, const uint32_t *s2)
{
    while (*s1 == *s2)
    {
        if (*s1 == '\0')
            return 0;
        
        s1++;
        s2++;
    }
    
    return (*s1 > *s2) ? 1 : -1;
}

// len

XTRA_API int unicode_strlen(const uint32_t *cps)
{
    int i = 0;
    while (*cps != (uint32_t)'\0')
    {        
        cps++;
        i++;
    }
    
    return i;
}

/* geometry */

XTRA_API Rect layout_wrapper(int x, int y, int w, int h)
{
    Rect ws;
    
    ws.x = x;
    ws.y = y;
    ws.w = w;
    ws.h = h;
    
    return ws;
}

XTRA_API Rect calc_wsabs(Widget *w)
{
    Rect wsabs = {0, 0, 0, 0};
    
    /* get width and size */
    
    Rect ws = w->calc_layout(w);
    
    wsabs.w = ws.w;
    wsabs.h = ws.h;
    
    /* iterative increment of x and y */
    
    while (w != NULL)
    {    
        Rect ws = w->calc_layout(w);
        
        wsabs.x += ws.x;
        wsabs.y += ws.y;
        
        w = w->parent;
    }
    
    return wsabs;
}

XTRA_API bool is_in_rect(Rect rect, int x, int y)
{
    return (rect.x <= x && x < (rect.x + rect.w)) && (rect.y <= y && y < (rect.y + rect.h));
}

#define MARGIN_INIT() \
    Widget *rel = find_rel_widget(current, relative_id); \
    assert(rel != NULL && "margin err: not found"); \
    Rect rect_rel    = calc_outer_rect(rel); \
    Rect rect_parent = current->parent->calc_layout(current->parent);

Rect calc_outer_rect(Widget *w)
{
    if (w->has_frame)
        return w->frame->calc_layout(w->frame);
    return w->calc_layout(w);
}

XTRA_API Rect margin_right(Widget *current, const char *relative_id)
{
    MARGIN_INIT();

    short x = rect_rel.x + rect_rel.w;
    short y = rect_rel.y;
    short available_width  = rect_parent.w - x;
    short available_height = rect_parent.h - rect_rel.y;
    
    Rect margin = {.x=x, .y=y, .w=available_width, .h=available_height};
    
    return margin;
}

XTRA_API Rect margin_bottom(Widget *current, const char *relative_id)
{
    MARGIN_INIT();
    
    short x = rect_rel.x;
    short y = rect_rel.y + rect_rel.h;
    short available_width  = rect_parent.w - x;
    short available_height = rect_parent.h - y;
    
    Rect margin = {.x=x, .y=y, .w=available_width, .h=available_height};
    return margin;
}

/* cursor */

Widget *_match_cursor(Widget *start_node, Widget *end_node, short x, short y)
{
    // if passing "deepest", then pass "deepest->parent" as start_node
    
    Widget *node = start_node;
    
    Widget *stack_checked[64]; // nodes whose children were already checked
    uint32_t stack_index = 0;
    
    //printf("got, start_node->id=%s, child_count=%d\n", start_node->id, start_node->child_count);
    
    do
    {

        // i-- > 0 (pattern for unsigned since "i >= 0 and i--" would not reach -1
        // там ещё до цикла самого же до body уже сделает i-- в condition поэтому начнётся оно с (node->child_count - 1)
        // так что всё ок
        // а потом дойдёт до нуля и когда 1 > 0 там потом post-increment и становится на нуль, так что и он тоже учитывается
        
        for (uint32_t i = node->child_count; i-- > 0;)
        {
            Widget *child = node->children[i];

            // check whether the children of a child were already checked

            bool children_checked = false;
            
            for (uint32_t j = 0; j < stack_index; j++)
            {
                if (child == stack_checked[j])
                {
                    children_checked = true;
                    break;
                }
            }
            
            if (children_checked == false)
            {
                Widget *dp_right = deepest_right(child);
                
                // if you remove this condition then there will be recursion
                
                if (dp_right != child) // otherwise it makes no sense since node == end_node and while loop will be terminated
                {
                    Widget *res = _match_cursor(dp_right->parent, child, x, y);
                    if (res != NULL)
                        return res;
                }
            }

            // main logic

            Rect wsabs = calc_wsabs(child);
            
            if (child->is_visible && is_in_rect(wsabs, x, y))
                return child;
        }
        
        assert(stack_index < ARRAY_SIZE(stack_checked));
        
        stack_checked[stack_index++] = node;
        node = node->parent; 
    } while (node != end_node && node != NULL); // check "node != NULL" is required, since if we are at root widget then node->parent is NULL
    
    return NULL;
}

XTRA_API Widget *match_cursor(Widget *root, short x, short y)
{
    Widget *deepest = deepest_right(root);
    return _match_cursor(deepest->parent, root, x, y);
}

XTRA_API Cursor get_cursor(App *app)
{
    Widget *focused = app->focused;
    Cursor cursor = {0};

    if (focused == NULL || focused->calc_cursor == NULL)
    {
        cursor.hidden = true;
        return cursor; // cursor is hidden
    }
    
    cursor = focused->calc_cursor(focused);
    
    Rect wsabs = calc_wsabs(focused);
    
    cursor.x += wsabs.x;
    cursor.y += wsabs.y;

    return cursor;
}

XTRA_API void show_cursor(bool flag)
{
    CONSOLE_CURSOR_INFO cursorinfo;
    
    cursorinfo.bVisible = (BOOL)flag;
    cursorinfo.dwSize   = 1;
    
    SetConsoleCursorInfo(hstdout, &cursorinfo);
}


/* screenbuffer */

void reset_sb(App *app) /* used only in update_sb */
{
    size_t ncells = app->termsize.w * app->termsize.h;
    
    for (size_t i = 0; i < ncells; i++)
    {
        Cell *oldCell = &app->oldScreenBuffer[i];
        Cell *cell    = &app->screenBuffer[i];
        
        oldCell->codepoint = 0;
        oldCell->attr      = 0;
        
        cell->codepoint    = 0;
        cell->attr         = 0;
    }
}

void update_sb(App *app)
{
    app->termsize = get_term_size();
    
    free(app->oldScreenBuffer);
    free(app->screenBuffer);
    
    size_t ncells = app->termsize.w * app->termsize.h;
    size_t sbSize = sizeof(Cell) * ncells;
    
    app->oldScreenBuffer = malloc(sbSize);
    app->screenBuffer    = malloc(sbSize);
    
    assert(app->oldScreenBuffer != NULL);
    assert(app->screenBuffer    != NULL);

    reset_sb(app);
}

XTRA_API void cls_sb(App *app)
{
    size_t ncells = app->termsize.w * app->termsize.h;
    
    for (size_t i = 0; i < ncells; i++)
    {
        Cell *cell    = &app->screenBuffer[i];

        cell->codepoint    = 0;
        cell->attr         = 0;
    }
}

XTRA_API void clear_widget_sb(App *app, Widget *w)
{
    //debug(1, 0, L"clear_widget_sb");
    
    Rect wsabs = calc_wsabs(w);

    int y_off;
    int x_off;

    for (short y = 0; y < wsabs.h; y++)
    {
        y_off = y + wsabs.y;
        
        for (short x = 0; x < wsabs.w; x++)
        {
            x_off = x + wsabs.x;
            
            Cell *cell = &app->screenBuffer[y_off * app->termsize.w + x_off];
            
            cell->codepoint = (uint32_t)' ';
            cell->attr      = FG(1, 1, 1);
        }
    }
}

/* resize */

void resize_widget_tree(App *app, Widget *w)
{
    for (uint32_t i = 0; i < w->child_count; i++)
    {
        Widget *child = w->children[i];
        
        if (child->resize_func != NULL)
            child->resize_func(app, child);
        
        if (child->child_count > 0)
            resize_widget_tree(app, child);
    }
}

XTRA_API void on_resize(App *app)
{
    update_sb(app);
    resize_widget_tree(app, app->root);
}

XTRA_API bool is_term_resized(App *app)
{
    Size newtermsize = get_term_size();
    return (newtermsize.w != app->termsize.w) || (newtermsize.h != app->termsize.h);
}

/* init */

XTRA_API void init(App *app)
{
    hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
    hstdin  = GetStdHandle(STD_INPUT_HANDLE);

    DWORD mode;
    GetConsoleMode(hstdin, &mode);

    mode &= ~ENABLE_ECHO_INPUT;  // noecho
    mode &= ~ENABLE_LINE_INPUT;  // не ждать Enter
    
    mode |= ENABLE_EXTENDED_FLAGS; // needed to disable quick edit mode
    mode &= ~ENABLE_QUICK_EDIT_MODE; // disable mouse selection
    mode |= ENABLE_MOUSE_INPUT;

    SetConsoleMode(hstdin, mode);
    
    SetConsoleCP(CP_UTF8);
    
    update_sb(app);
}

XTRA_API App new_app(void)
{
    App app = {0};
    return app;
}

/* widgets */

XTRA_API int register_widget(Widget *w, Widget *parent)
{
    //printf("registering id: %s, parent id: %s\n", w->id, parent->id);
    
    w->parent      = parent;
    
    if (w->has_frame)
    {
        //printf("registering: \"%s\", has frame\n");
        register_widget(w->frame, parent);
    }
    
    // add child in parent
    
    if (parent != NULL)
    {
        //printf("registering: \"%s\", parent: \"%s\": add children\n", w->id, w->parent->id);
        
        //printf("w->id=%s, parent->id=%s, new size=%zu\n", w->id, parent->id, (parent->child_count + 1) * sizeof(Widget*));
        
        
        Widget **p_children = realloc(parent->children, (parent->child_count + 1) * sizeof(*parent->children));

        assert(p_children != NULL);
        
        //if (p_children == NULL)
        //    return -1;
        
        parent->children    = p_children;
        parent->children[parent->child_count] = w;
        parent->child_count += 1;
    }
    
    return 0;
}

XTRA_API void deregister_widget(Widget *parent, Widget *w)
{
    for (uint32_t i = 0; i < parent->child_count; )
    {
        if (parent->children[i] == w || (w->has_frame && parent->children[i] == w->frame))
        {
            for (uint32_t j = i; j + 1 < parent->child_count; j++)
            {
                parent->children[j] = parent->children[j + 1];
            }

            parent->child_count--;
        } else {
            i++;
        }
    }
}

XTRA_API struct Widget *new_widget(
    const char *id,
    LayoutFunc calc_layout,
    CursorFunc calc_cursor,
    RenderFunc render_func,
    EventFunc  event_func,
    ResizeFunc resize_func,
    
    bool is_visible,
    bool is_focusable,
    bool has_frame
)
{
    Widget *w = malloc(sizeof(Widget));
    
    if (w == NULL)
        return (Widget*)-1;
    
    strncpy(w->id, id, MAX_ID_LENGTH);

    w->parent      = NULL;
    w->children    = NULL;
    
    if (has_frame)
    {
        size_t id_len = strlen(id);
        size_t frame_postfix_len = strlen(FRAME_POSTFIX);
        
        char *frame_id = malloc(id_len + frame_postfix_len + 1); // +1 for null-terminator
        
        assert(frame_id != NULL);
        
        strcpy(frame_id, id);
        strcpy(frame_id + id_len, "_frame");
        frame_id[id_len + frame_postfix_len] = '\0';
        
        Widget *frame = new_widget(
            frame_id,
            calc_layout,
            NULL,
            render_frame_default_func,
            NULL,
            NULL,
            is_visible,
            false,
            false
        );
        
        assert(frame != NULL);
        w->frame = frame;
        
        free(frame_id);
    } else {
        w->frame = NULL;
    }
    
    w->is_visible   = is_visible;
    w->is_focusable = is_focusable;
    w->has_frame    = has_frame;
    
    if (has_frame)
        w->calc_layout = frame_connected_widget_calc_layout;
    else
        w->calc_layout = calc_layout;

    w->last_event_type = NONE_TYPE;

    w->calc_cursor  = calc_cursor;
    w->render_func  = render_func;
    w->event_func   = event_func;
    w->resize_func  = resize_func;
    
    w->child_count = 0;
    
    return w;
}

XTRA_API void push_overlay(App *app, Widget *overlay)
{
    //debug(0, 0, L"here, overlay: %d, app->root: %d", overlay, app->root);
    register_widget(overlay, app->root);
}

XTRA_API void pop_overlay(App *app, Widget *overlay)
{
    cls_sb(app);
    deregister_widget(app->root, overlay);
}

/* terminal information */

XTRA_API Size get_term_size(void)
{
    CONSOLE_SCREEN_BUFFER_INFO terminfo;
    GetConsoleScreenBufferInfo(hstdout, &terminfo);

    Size s;
    
    s.w = terminfo.srWindow.Right  - terminfo.srWindow.Left + 1;
    s.h = terminfo.srWindow.Bottom - terminfo.srWindow.Top + 1;
    
    return s;
}

/* input */

void parse_key_event(InputEvent *event, KEY_EVENT_RECORD key)
{
    //printf("codepoint=%u\n", (uint32_t)key.uChar.UnicodeChar);
    
    assert(is_utf16(key.uChar.UnicodeChar));
    
    DWORD keyState = key.dwControlKeyState;
    
    event->key.key     = translate_key(key.wVirtualKeyCode);
    event->key.codepoint = (uint32_t)key.uChar.UnicodeChar;
    
    event->key.pressed = key.bKeyDown;
    
    event->key.ctrl  = keyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED);
    event->key.alt   = keyState & (LEFT_ALT_PRESSED  | RIGHT_ALT_PRESSED);
    event->key.shift = keyState & SHIFT_PRESSED;
}

void define_mouse_button_by_state(DWORD buttonState, InputEvent *event, MOUSE_EVENT_RECORD mouse)
{
    // buttonState got from mouse.dwButtonState (can be old_state or new_state)
    
    if (buttonState & FROM_LEFT_1ST_BUTTON_PRESSED)
        event->mouse.button = MOUSE_LEFT;
    else if (buttonState & RIGHTMOST_BUTTON_PRESSED)
        event->mouse.button = MOUSE_RIGHT;
    else if (buttonState & FROM_LEFT_2ND_BUTTON_PRESSED)
        event->mouse.button = MOUSE_MIDDLE;
}

void parse_mouse_event(InputEvent *event, MOUSE_EVENT_RECORD mouse)
{
    /* x and y coords */
    
    event->mouse.button  = -1;
    event->mouse.flags   = 0;
    
    event->mouse.x = mouse.dwMousePosition.X;
    event->mouse.y = mouse.dwMousePosition.Y;
    
    DWORD new_buttons = mouse.dwButtonState;

    // wheel buttons
    
    if (mouse.dwEventFlags & MOUSE_WHEELED)
    {
        short wheel_delta = GET_WHEEL_DELTA_WPARAM(new_buttons);
        
        if (wheel_delta > 0)
            event->mouse.button = MOUSE_WHEEL_UP;
        else
            event->mouse.button = MOUSE_WHEEL_DOWN;
    }
    
    /* get mousestate (pressed, released, hold, move) */
    
    DWORD old_pressed = (old_buttons & MOUSE_BUTTONS);
    DWORD new_pressed = (new_buttons & MOUSE_BUTTONS);
     
    /*
        по сути берем вот условно old_pressed = 001
        а вот new_pressed у нас 100
        потом инвертируем 100: 011
        
        и потом маска:
        
        001 &
        011 = 
        001
        
        по сути мы инвертируя new_pressed смотрим на то что не было нажато 
        и потом принимаем маску на old_pressed с тем что не было нажато в новом event
        и таким образом получаем released
        
        
        а в случае "new_pressed & ~old_pressed"
        мы смотрим какие кнопки не были нажаты в прошлом и сравниваем их с текущими нажатыми (and)
    */
    DWORD just_released = old_pressed & ~new_pressed;
    DWORD just_pressed  = new_pressed & ~old_pressed;

    //debug_render_bits(0, 10, new_pressed);
    //debug_render_bits(0, 5, just_pressed);

    if (just_pressed)
    {
        event->mouse.flags |= MOUSE_PRESS;
        define_mouse_button_by_state(just_pressed, event, mouse); // press: define mouse button
    }

    if (just_released)
    {
        event->mouse.flags |= MOUSE_RELEASE;
        define_mouse_button_by_state(just_released, event, mouse); // release: define mouse button
    }

    if (new_pressed && !just_pressed && !just_released)
        event->mouse.flags |= MOUSE_HOLD;
        define_mouse_button_by_state(new_buttons, event, mouse); // hold: define mouse button
     
        if (mouse.dwEventFlags & MOUSE_MOVED)
            event->mouse.flags |= MOUSE_MOVE;
    
    /* modifiers */

    DWORD keystate = mouse.dwControlKeyState;
    
    if (keystate & SHIFT_PRESSED)
        event->mouse.mod |= MOD_SHIFT_PRESSED;
    
    if (keystate & LEFT_CTRL_PRESSED)
        event->mouse.mod |= MOD_LEFT_CTRL_PRESSED;
    
    if (keystate & RIGHT_CTRL_PRESSED)
        event->mouse.mod |= MOD_RIGHT_CTRL_PRESSED;
    
    if (keystate & LEFT_ALT_PRESSED)
        event->mouse.mod |= MOD_LEFT_ALT_PRESSED;
    
    if (keystate & RIGHT_ALT_PRESSED)
        event->mouse.mod |= MOD_RIGHT_ALT_PRESSED;
    
    
    
    // if not MOUSE_MOVED then that means that the wheel was moved (and dwButtonState is an deltaoffset in wheel)
    
    old_buttons = new_buttons;
    
    /*
    if (mouse.dwEventFlags == 0 || mouse.dwEventFlags & MOUSE_MOVED)
    {
        old_buttons = new_buttons;
    }
    */
    
}

XTRA_API bool input_poll(InputEvent *event)
{
    DWORD status = WaitForSingleObject(hstdin, INPUT_POLL_WAIT_TIMEOUT);
    
    if (status == WAIT_TIMEOUT)
        return false;
    
    INPUT_RECORD record;
    DWORD        n;
    
    if (!ReadConsoleInputW(hstdin, &record, 1, &n))
        return false;

    WCHAR c = record.Event.KeyEvent.uChar.UnicodeChar;

    switch (record.EventType)
    {
        case KEY_EVENT:
        
            // ignoring on release buttons
            
            if (!record.Event.KeyEvent.bKeyDown)
                return false;
            
            event->type = KEY_TYPE;
            parse_key_event(event, record.Event.KeyEvent);
            return true;
        case MOUSE_EVENT:
            event->type = MOUSE_TYPE;
            parse_mouse_event(event, record.Event.MouseEvent);
            return true;
        default:
            return false;
    }
}

/* output */

void output_codepoint(
    short x,
    short y,
    uint32_t codepoint,
    short attr
)
{
    WCHAR     out[2];
    CHAR_INFO chars[2];
    
    int count = unicode_to_wchar(codepoint, out);
    
    for (int i = 0; i < count; i++)
    {
        chars[i].Char.UnicodeChar = out[i];
        chars[i].Attributes = attr;
    }
    
    WriteConsoleOutputW(
        hstdout,
        chars,
        (COORD){.X = count, .Y = 1}, // buffer size
        (COORD){.X = 0, .Y = 0}, // beginning offset in 'chars'
        &(SMALL_RECT){.Left=x, .Top=y, .Right=x + (count - 1), .Bottom=y} // x + (count - 1) since we work with buffer (and terminal interprets this buffer data and renders for example two chars as one cell)
    );
}

XTRA_API void addstr(App *app, Widget *w, short x, short y, const uint32_t *codepoints, size_t num, uint16_t attr)
{
    if (x < 0 || y < 0)
        return;
    
    Rect wsabs       = calc_wsabs(w);
    Rect wsparentabs = calc_wsabs(w->parent);
    
    //printf("wsabs.w=%d, wsabs.h=%d\n", wsabs.w, wsabs.h);
    
    for (size_t i = 0; i < num; i++)
    {
        /*
        
        короче нужно y >= wsabs.h делать после x >= wsabs.w
        поскольку иначе там может оно дойти условно до y >= wsabs.h
        после инкрементации y в условии x >= wsabs.w
        и потом там первый символ зарендерить где нельзя уже рендерить
        (например на frame) - и только потом уже на следующей итерации оно завершится
        (что неправильно по сути)
        
        
        
        */
        
        uint32_t cp = codepoints[i];
        
        if (x >= wsabs.w)
        {
            x = 0;
            y += 1;
        }
        
        if (cp == 0x0A) // new line character '\n'
        {
            x = 0;
            y += 1;
            continue;
        }
        
        if (y >= wsabs.h)
            break;
        
        int x_off = x + wsabs.x;
        int y_off = y + wsabs.y;
        
        if (x_off >= (wsparentabs.x + wsparentabs.w) || y_off >= (wsparentabs.y + wsparentabs.h))
            break;
        
        Cell *cell = &app->screenBuffer[y_off * app->termsize.w + x_off];
        
        cell->codepoint = cp;
        cell->attr      = attr;
        
        x += 1;
    }
}

XTRA_API void render_screen(App *app)
{
    for (short y = 0; y < app->termsize.h; y++)
    {
        for (short x = 0; x < app->termsize.w; x++)
        {
            int pos = y * app->termsize.w + x;
            
            Cell *oldCell = &app->oldScreenBuffer[pos];
            Cell *cell    = &app->screenBuffer[pos];
            
            if (oldCell->codepoint != cell->codepoint || oldCell->attr != cell->attr)
            {
                output_codepoint(x, y, cell->codepoint, cell->attr);
                
                oldCell->codepoint = cell->codepoint;
                oldCell->attr      = cell->attr;
            }
        }
    }
}

XTRA_API void cls(void)
{
    COORD coordScreen = { 0, 0 };    // home for the cursor
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    // Get the number of character cells in the current buffer.
    if (!GetConsoleScreenBufferInfo(hstdout, &csbi))
    {
        return;
    }

    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    // Fill the entire screen with blanks.
    if (!FillConsoleOutputCharacter(hstdout,        // Handle to console screen buffer
                                    (TCHAR)' ',      // Character to write to the buffer
                                    dwConSize,       // Number of cells to write
                                    coordScreen,     // Coordinates of first cell
                                    &cCharsWritten)) // Receive number of characters written
    {
        return;
    }

    // Get the current text attribute.
    if (!GetConsoleScreenBufferInfo(hstdout, &csbi))
    {
        return;
    }

    // Set the buffer's attributes accordingly.
    if (!FillConsoleOutputAttribute(hstdout,         // Handle to console screen buffer
                                    csbi.wAttributes, // Character attributes to use
                                    dwConSize,        // Number of cells to set attribute
                                    coordScreen,      // Coordinates of first cell
                                    &cCharsWritten))  // Receive number of characters written
    {
        return;
    }

    // Put the cursor at its home coordinates.
    SetConsoleCursorPosition(hstdout, coordScreen);
}

/* debug */

void _debug(short x, short y, const wchar_t *str)
{
    CHAR_INFO buffer[1024];
    
    int i;
    
    for (i = 0; i < wcslen(str) && str[i] != '\0'; i++)
    {
        buffer[i].Attributes       = FG(1, 1, 1);
        buffer[i].Char.UnicodeChar = (WCHAR)str[i];
    }
    
    WriteConsoleOutputW(
        hstdout,
        buffer,
        (COORD){.X=i, .Y=1},
        (COORD){.X=0, .Y=0},
        &(SMALL_RECT){.Left=x, .Top=y, .Right=x + i, .Bottom=y}
    );
}

XTRA_API void debug(short x, short y, const wchar_t *format, ...)
{
    va_list args;
    
    va_start(args, format);
    
    wchar_t buffer[1024];
    
    int result = vswprintf(
        buffer,
        sizeof(buffer) / sizeof(buffer[0]),
        format,
        args
    );
    
    _debug(x, y, buffer);

    

    va_end(args);
}

XTRA_API void render_bits(int val)
{
    for (int i = 7; i >= 0; i--)
    {
        printf("%d", (val >> i) & 1);
    }
    printf("\n");
}

XTRA_API void debug_render_bits(short x, short y, int val)
{
    int j = 0;
    for (int i = 7; i >= 0; i--)
    {
        debug(x + j, y, L"%d", (val >> i) & 1);
        j++;
    }
}

