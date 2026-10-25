#include "xtra.h"


App *app;


#define init_button(w) Button *button = (Button*)w->data

#define init_button_stack(w) ButtonStack *button_stack = ((Button*)w->data)->stack_ptr

#define LAYOUT_INSIDE(func_name, w, h) LAYOUT_PARENT(func_name, 0, 0, (w), (h))
#define BUTTON(name, content, rel) LAYOUT_RIGHT(name, rel, 0, 0, sizeof(content) - 1, 1)

#define WIDGET_NO_METHODS (WidgetMethods){}

#define BUTTON_STACK_SIZE 8

LAYOUT_DEFAULT(ws_root, 0, 0, app->termsize.w, app->termsize.h);
LAYOUT_PARENT(ws_panel_container, 1, 1, parent.w - 1, 1);


// buttons

LAYOUT_INSIDE(ws_main_opt, sizeof("main"), 1);

BUTTON(ws_calendar_opt,   "calendar",   "ws_main_opt");
BUTTON(ws_extensions_opt, "extensions", "ws_calendar_opt");
BUTTON(ws_settings_opt,   "settings",   "ws_extensions_opt");


typedef struct Button Button;
typedef struct ButtonStack ButtonStack;

typedef enum {
    OPEN_MAIN,
    OPEN_CALENDAR,
    OPEN_EXTENSIONS,
    OPEN_SETTINGS
} ButtonPressedEvent;


struct Button {
    ButtonStack *stack_ptr;
    
    uint16_t color_highlight;
    uint16_t color_default;
    
    ButtonPressedEvent on_pressed_event;
    
    const char *text;
    
    bool selecting;
    bool pressed;
};

struct ButtonStack {
    Button data[BUTTON_STACK_SIZE];
    size_t nbuttons;
};






void reset_pressed(ButtonStack *button_stack)
{
    for (size_t i = 0; i < button_stack->nbuttons; i++)
    {
        Button *button = &button_stack->data[i];
        button->pressed = false;
    }
}


void button_entry(Widget *w)
{
    init_button(w);
    button->selecting = true;
}

void button_exit(Widget *w)
{
    init_button(w);
    button->selecting = false;
}

void button_event(App *app, Widget *w, InputEvent event)
{
    init_button(w);
    init_button_stack(w);
    
    if (event.type == MOUSE_TYPE)
    {
        if (event.mouse.button == MOUSE_LEFT)
        {
            reset_pressed(button_stack);
            button->pressed = true;
        }
    }
}

void button_render(App *app, Widget *w)
{    
    init_button(w);

    
    uint32_t *cp = unicode_conv(button->text);
    int cp_len   = unicode_strlen(cp);

    printf("w->id: %s, button->text: %s, cp_len=%d\n", w->id, button->text, cp_len);
    
    int c = 'A';
    
    addstr(app, w, 0, 0, &c, 1, FG(1, 1, 1));
    
    printf("after rendering\n");
    
    /*
    if (button->selecting || button->pressed)
    {
        addstr(app, w, 0, 0, cp, cp_len, button->color_highlight);
    } else {
        //printf("this case111\n");
        
        addstr(app, w, 0, 0, cp, cp_len, button->color_default);
        //Sleep(10000);
    }
    */
}










int main(void)
{
    App a = new_app();
    app = &a;
        
    Widget *root = new_widget(
        "root",
        WIDGET_NO_METHODS,
        WIDGET_NONE_FLAGS
    );
    
    Widget *panel_container = new_widget(
        "panel_container",
        (WidgetMethods){
            .calc_layout = ws_panel_container
        },
        WIDGET_NONE_FLAGS
    );
    
    Widget *opt_main = new_widget(
        "ws_main_opt",
        (WidgetMethods){
            .calc_layout = ws_main_opt,
            .on_entry    = button_entry,
            .on_exit     = button_exit,
            .event_func  = button_event,
            .render_func = button_render
        },
        WIDGET_IS_VISIBLE | WIDGET_IS_FOCUSABLE
    );
    
    Widget *opt_calendar = new_widget(
        "ws_calendar_opt",
        (WidgetMethods){
            .calc_layout = ws_calendar_opt,
            .on_entry    = button_entry,
            .on_exit     = button_exit,
            .event_func  = button_event,
            .render_func = button_render
        },
        WIDGET_IS_VISIBLE | WIDGET_IS_FOCUSABLE
    );

    Widget *opt_extensions = new_widget(
        "ws_extensions_opt",
        (WidgetMethods){
            .calc_layout = ws_extensions_opt,
            .on_entry    = button_entry,
            .on_exit     = button_exit,
            .event_func  = button_event,
            .render_func = button_render
        },
        WIDGET_IS_VISIBLE | WIDGET_IS_FOCUSABLE
    );

    Widget *opt_settings = new_widget(
        "ws_settings_opt",
        (WidgetMethods){
            .calc_layout = ws_settings_opt,
            .on_entry    = button_entry,
            .on_exit     = button_exit,
            .event_func  = button_event,
            .render_func = button_render
        },
        WIDGET_IS_VISIBLE | WIDGET_IS_FOCUSABLE
    );
    
    register_widget(panel_container, root);
    register_widget(opt_main, panel_container);
    //register_widget(opt_calendar, panel_container);
    //register_widget(opt_extensions, panel_container);
    //register_widget(opt_settings, panel_container);
    
    
    ButtonStack stack = {0};
    
    stack.data[0] = (Button){
        .stack_ptr        = &stack,
        .color_highlight  = FG(0, 1, 0),
        .color_default    = FG(1, 1, 1),
        .on_pressed_event = OPEN_MAIN,
        .text             = "main",
    };
    
    stack.nbuttons = 1;
    
    opt_main->data = (void*)&stack.data[0];


    init(app);
    app->root = root; 
    
    printf("got111\n");
    
    cls(); 

    on_resize(app, false);

    set_term_minsize(app, 40, 10);

    InputEvent event;

    
    while (true)
    {
        bool resized = resize(app);
        if (!resized)
            continue;

        //int ret = default_event_handler(app, &event);

        render_ui(app);
        render_screen(app);
    }
    
    

    return 0;
}

