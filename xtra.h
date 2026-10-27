#ifndef XTRA_H
#define XTRA_H

#include <windows.h>
#include <inttypes.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>


#define ENABLE_ON_ENTRY_FUNCS true


#if defined(_WIN32)
    #if defined(LIB_BUILD)
        #define XTRA_API __declspec(dllexport)
    #elif defined(LIB_NO_DLL)
        #define XTRA_API
    #else
        #define XTRA_API __declspec(dllimport) /* dll */
    #endif
#else
    #error "This library supports windows only"
#endif



/* (can be changed) */

#define COMPILE_AS_EXTERNAL_LIB 0
#define INPUT_POLL_WAIT_TIMEOUT 10

/* frame chars (unicode) */

#define BOX_TL  0x250C  // ┌ (TOP-LEFT)
#define BOX_TR  0x2510  // ┐ (TOP-RIGHT)

#define BOX_BL  0x2514  // └ (BOTTOM-LEFT)
#define BOX_BR  0x2518  // ┘ (BOTTOM-RIGHT)
#define BOX_H   0x2500  // ─ (HORIZONTAL)
#define BOX_V   0x2502  // │ (VERTICAL)

/* (don't change) */

#define UTF16_SURROGATE_START 0xD800
#define UTF16_SURROGATE_END   0XDFFF

#define is_utf16(ch) (uint32_t)(ch) < UTF16_SURROGATE_START || (uint32_t)(ch) > UTF16_SURROGATE_END
#define MOUSE_BUTTONS (FROM_LEFT_1ST_BUTTON_PRESSED | RIGHTMOST_BUTTON_PRESSED | FROM_LEFT_2ND_BUTTON_PRESSED)

#define FRAME_POSTFIX "_frame" // widgets which are frames have this postfix at the end
#define FRAME_POSTFIX_LENGTH 6

#define ARRAY_SIZE(arr) sizeof(arr) / sizeof(arr[0])
#define MAX_ID_LENGTH 32

#define init_data_wrapper(widget, type, var_name) \
    assert(widget->data != NULL); \
    type *var_name = (type*)widget->data;

#define _calc_layout(w) (assert((w) != NULL), assert((w)->calc_layout != NULL), (w)->calc_layout((w)))
#define _calc_cursor(w) (assert((w) != NULL), assert((w)->calc_cursor != NULL), (w)->calc_cursor((w)))
#define _on_entry(w)    (assert((w) != NULL), assert((w)->on_entry    != NULL), (w)->on_entry((w)))
#define _on_exit(w)     (assert((w) != NULL), assert((w)->on_exit     != NULL), (w)->on_exit((w)))

/* main api structs */

typedef struct Event      Event;
typedef enum   EventType  EventType;
typedef struct App        App;
typedef struct Widget     Widget;
typedef struct Rect       Rect;

typedef struct WidgetMethods WidgetMethods;

typedef struct InputEvent InputEvent;

/* another */

typedef struct Coord      Coord;
typedef struct Cursor     Cursor;
typedef struct Size       Size;
typedef struct Rect       Rect;
typedef struct RectDelta  RectDelta;
typedef struct Cell       Cell;

/* input */

typedef struct KeyEvent   KeyEvent;
typedef struct MouseEvent MouseEvent;

typedef enum InputType   InputType;
typedef enum MouseFlags  MouseFlags;
typedef enum MouseButton MouseButton;
typedef enum Key         Key;
typedef enum MouseMod    MouseMod;

/* widget methods */

typedef Rect (*LayoutFunc)(Widget *);
typedef Cursor (*CursorFunc)(Widget *);
typedef void (*RenderFunc)(App *, Widget *);
typedef void (*ResizeFunc)(App *, Widget *);
typedef void (*EventFunc)(App *, Widget *, InputEvent);

typedef void (*EntryFunc)(Widget *); // focus
typedef void (*ExitFunc)(Widget *);  // unfocus



struct Coord
{
    short x;
    short y;
};

struct Size
{
    short w;
    short h;
};

struct Rect
{
    short x;
    short y;
    short w;
    short h; 
};

struct RectDelta
{
    short dx;
    short dy;
    short dw;
    short dh;
};

struct Cell
{
    uint32_t codepoint;
    uint16_t attr;
};

struct App
{
    Widget *root;
    Widget *focused;
    
    Widget   *overlay;
    uint16_t overlay_count;
    
    Size termsize;
    Size mintermsize;
    
    bool set_min_termsize;
    bool term_already_resized;

    Cell *oldScreenBuffer;
    Cell *screenBuffer;
};

struct Cursor
{
    short x;
    short y;
    
    bool hidden;
};


struct Event
{
    int id; // enum can be used (custom id numeration)
    Widget *source;
    void *data;
};

enum Key
{
    KEY_DOWN,
    KEY_RIGHT,
    KEY_UP,
    KEY_LEFT,
    KEY_ENTER,
    KEY_ESCAPE,
    KEY_UNKNOWN
};

enum InputType
{
    KEY_TYPE,
    MOUSE_TYPE,
    NONE_TYPE
};

enum MouseFlags {
    MOUSE_PRESS   = 1 << 0,
    MOUSE_RELEASE = 1 << 1,
    MOUSE_MOVE    = 1 << 2,
    MOUSE_HOLD    = 1 << 3
};

enum MouseButton {
    MOUSE_LEFT,
    MOUSE_RIGHT,
    MOUSE_MIDDLE,
    MOUSE_WHEEL_UP,
    MOUSE_WHEEL_DOWN
};

enum MouseMod {
    MOD_SHIFT_PRESSED      = 1 << 0,
    
    MOD_LEFT_ALT_PRESSED   = 1 << 1,
    MOD_RIGHT_ALT_PRESSED  = 1 << 2,
    
    MOD_LEFT_CTRL_PRESSED  = 1 << 3,
    MOD_RIGHT_CTRL_PRESSED = 1 << 4,
};

struct KeyEvent {
    Key key;
    uint32_t codepoint;
    bool pressed;
    bool ctrl;
    bool alt;
    bool shift;
};

struct MouseEvent {
    short x;
    short y;
    MouseButton button;
    uint32_t flags;
    uint32_t mod;
};

struct InputEvent
{
    InputType type;
    
    union
    {
        KeyEvent   key;
        MouseEvent mouse;
    };
};

enum
{
    WIDGET_NONE_FLAGS   = 0,
    WIDGET_IS_VISIBLE   = 1 << 0,
    WIDGET_IS_FOCUSABLE = 1 << 1,
    WIDGET_HAS_FRAME    = 1 << 2,
    
    /* not public api */
    WIDGET_IS_FRAME  = 1 << 3
};

struct WidgetMethods
{
    LayoutFunc calc_layout;
    CursorFunc calc_cursor;
    RenderFunc render_func;
    EventFunc  event_func;
    ResizeFunc resize_func;
    EntryFunc  on_entry;
    ExitFunc   on_exit;
};

struct Widget
{
    char id[MAX_ID_LENGTH];
    
    uint32_t flags;
    
    LayoutFunc calc_layout;
    CursorFunc calc_cursor;
    RenderFunc render_func;
    EventFunc  event_func;
    ResizeFunc resize_func;
    EntryFunc  on_entry;
    ExitFunc   on_exit;

    RectDelta  delta;

    void *data;
    
    InputType  last_event_type;
    
    MouseEvent last_mouse;
    KeyEvent   last_key;


    Widget *frame;

    Widget *parent;
    Widget **children;
    uint32_t child_count;
};



/* main api */

/* helpers */

XTRA_API void clear_sb(App *app);
XTRA_API Widget *find_child(Widget *w, const char *id);

void debug_render_bits(short x, short y, int val);

// init & create & new

XTRA_API void init(App *app);
XTRA_API App  new_app(void);

// terminal info

XTRA_API Size get_term_size(void);

// resize

XTRA_API bool is_term_resized(App *app);
XTRA_API void on_resize(App *app, bool resize_widgets);

XTRA_API void set_term_minsize(App *app, short w, short h);
XTRA_API bool resize(App *app);

// cursor

XTRA_API Widget *match_cursor(Widget *root, short x, short y);
XTRA_API Cursor get_cursor(App *app);
XTRA_API void show_cursor(bool flag);

// widgets

XTRA_API struct Widget *new_widget(
    const char *id,
    WidgetMethods methods,
    uint32_t flags
);
XTRA_API int register_widget(Widget *w, Widget *parent);

XTRA_API Rect calc_wsabs(Widget *w);

XTRA_API void deregister_widget(Widget *parent, Widget *w);
XTRA_API void pop_overlay(App *app, Widget *overlay);
XTRA_API void push_overlay(App *app, Widget *overlay);

// input

XTRA_API bool input_poll(InputEvent *event);

// output

/* work with screenbuffer */

XTRA_API void cls_sb(App *app);
XTRA_API void clear_widget_sb(App *app, Widget *w);

XTRA_API void render_widget(App *app, Widget *w);
XTRA_API void addstr(App *app, Widget *w, short x, short y, const uint32_t *codepoints, size_t num, uint16_t attr);

/* work with the terminal itself */

XTRA_API void cls(void);
XTRA_API void render_screen(App *app);
XTRA_API void debug(short x, short y, const wchar_t *format, ...);

// geometry

XTRA_API bool is_in_rect(Rect r, int x, int y);

XTRA_API Rect margin_right(Widget *current, const char *relative_id);
XTRA_API Rect margin_bottom(Widget *current, const char *relative_id);

// encoding

XTRA_API int utf8_decode(const char *source, uint32_t *out);
XTRA_API int utf8_to_unicode(const char *s, uint32_t *buffer, size_t bufsize);

XTRA_API int unicode_strlen(const uint32_t *cps);

uint32_t *unicode_conv(const char *fmt, ...); /* warning: thread-unsafe */

// events

XTRA_API int  default_focus(App *app, InputEvent *event);
XTRA_API void dispatch_events_to_focused(App *app, InputEvent *event, int focus_ret); /* focus return code */
XTRA_API int  default_event_handler(App *app, InputEvent *event);


// helpers (combine different widgets)

// XTRA_API void dispatch(App *app, Event event); // should be redefined by the user


#ifdef LIB_NO_DLL /* ifndef part is in xtra.c */

#define FG(r, g, b) ((r) ? FOREGROUND_RED : 0) | \
                    ((g) ? FOREGROUND_GREEN : 0) | \
                    ((b) ? FOREGROUND_BLUE : 0)
    
#define BG(r, g, b) ((r) ? BACKGROUND_RED : 0) | \
                    ((g) ? BACKGROUND_GREEN : 0) | \
                    ((b) ? BACKGROUND_BLUE : 0)

#define get_parent_rect(w) _calc_layout(w->parent)
#define find_rel_widget(w, id) find_child((w)->parent, (id))
#define render_ui(app) render_widget((app), (app)->root)
#define layout_wrapper(x, y, w, h) (Rect){(x), (y), (w), (h)}

#else

XTRA_API uint16_t FG(bool r, bool g, bool b);
XTRA_API uint16_t BG(bool r, bool g, bool b);

XTRA_API Rect get_parent_rect(Widget *w);
XTRA_API Widget *find_rel_widget(Widget *w, const char *id);
XTRA_API void render_ui(App *app);
XTRA_API Rect layout_wrapper(int x, int y, int w, int h);

#endif /* LIB_NO_DLL */


/* layout-functions fabric (generators) */

#define LAYOUT_PARENT(name, ...) \
    Rect name(Widget *w) { \
        Rect parent = get_parent_rect(w); \
        return layout_wrapper(__VA_ARGS__); \
    }

#define LAYOUT_ABS(name, ...) \
    Rect name(Widget *w) { \
        return layout_wrapper(__VA_ARGS__); \
    }

#define LAYOUT_MARGIN_RIGHT(name, rel_id, ...) \
    Rect name(Widget *w) { \
        Rect margin = margin_right(w, (rel_id)); \
        return layout_wrapper(__VA_ARGS__); \
    }

#define LAYOUT_MARGIN_BOTTOM(name, rel_id, ...) \
    Rect name(Widget *w) { \
        Rect margin = margin_bottom(w, (rel_id)); \
        return layout_wrapper(__VA_ARGS__); \
    }

#define LAYOUT_DEFAULT LAYOUT_ABS
#define LAYOUT_RIGHT   LAYOUT_MARGIN_RIGHT
#define LAYOUT_BOTTOM  LAYOUT_MARGIN_BOTTOM



#endif /* XTRA_H */
