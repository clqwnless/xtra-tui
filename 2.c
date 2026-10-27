#include "xtra.h"



App *app;


#define init_button(w) init_data_wrapper(w, Button, button)

#define init_button_stack(w) ButtonStack *button_stack = ((Button*)w->data)->stack_ptr

#define BUTTON(name, content, rel) LAYOUT_RIGHT(name, rel, 0, 0, sizeof(content) - 1, 1)

#define BUTTON_STACK_SIZE 16


#define BUTTON_CONTAINER LAYOUT_PARENT

#define BUTTON_START(name, content) LAYOUT_PARENT(name, 0, 0, sizeof(content) - 1, 1)
#define BUTTON_NEXT(name, relative_button, content) LAYOUT_MARGIN_RIGHT(name, relative_button, margin.x, margin.y, sizeof(content) - 1, 1)




Rect ws_root(Widget *w)
{
    return layout_wrapper(0, 0, app->termsize.w, app->termsize.h);
}

Rect ws_main_opt(Widget *w)
{
    return layout_wrapper(0, 0, 4, 1);
}

LAYOUT_MARGIN_RIGHT(ws_calendar_opt, "ws_main_opt", margin.x + 1, margin.y, sizeof("calendar") - 1, 1);


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
    
    int on_pressed_event; // dispatch
    
    const char *text;
    
    bool selecting;
    bool pressed;
};

struct ButtonStack {
    Button buttons[BUTTON_STACK_SIZE];
    size_t nbuttons;
};






void reset_pressed(ButtonStack *button_stack)
{
    for (size_t i = 0; i < button_stack->nbuttons; i++)
    {
        Button *button = &button_stack->buttons[i];
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

   
    if (button->selecting || button->pressed)
    {
        addstr(app, w, 0, 0, cp, cp_len, button->color_highlight);
    } else {
        //printf("this case111\n");
        
        addstr(app, w, 0, 0, cp, cp_len, button->color_default);
        //Sleep(10000);
    }
    
}



Widget *new_button_container(const char *id, LayoutFunc calc_layout)
{
    return new_widget(
        id,
        (WidgetMethods){
            .calc_layout = calc_layout,
        },
        WIDGET_NONE_FLAGS
    );
}

Widget *new_button(const char *id, LayoutFunc calc_layout, Button button)
{
    ButtonStack *stack        = button.stack_ptr;
    Button      *stack_button = NULL;
    
    assert(stack->nbuttons < BUTTON_STACK_SIZE);
    
    Widget *w = new_widget(
        id,
        (WidgetMethods){
            .calc_layout = calc_layout,
            .on_entry    = button_entry,
            .on_exit     = button_exit,
            .event_func  = button_event,
            .render_func = button_render
        },
        WIDGET_IS_VISIBLE | WIDGET_IS_FOCUSABLE
    );
    
    
    stack_button = &stack->buttons[stack->nbuttons++];
 
    *stack_button = button;
    
    w->data = (void*)stack_button;

    return w;
}




int main(void)
{
    App a = new_app();
    app = &a;
    
    init(app);
    
    ButtonStack stack = {0};
    
    Widget *root = new_widget(
        "root",
        (WidgetMethods){
            .calc_layout=ws_root
        },
        WIDGET_NONE_FLAGS
    );
    
    Widget *opt_main     = new_button(
        "ws_main_opt",
        ws_main_opt,
        (Button){
            .stack_ptr        = &stack,
            .color_highlight  = BG(0, 1, 0),
            .color_default    = FG(0, 1, 0),
            .on_pressed_event = OPEN_MAIN,
            .text             = "main"
        }
    );
    
    Widget *opt_calendar = new_button(
        "ws_calendar_opt",
        ws_calendar_opt,
        (Button){
            .stack_ptr        = &stack,
            .color_highlight  = BG(0, 1, 0),
            .color_default    = FG(0, 1, 0),
            .on_pressed_event = OPEN_CALENDAR,
            .text             = "calendar"
        }
    );

    register_widget(opt_main, root);
    register_widget(opt_calendar, root);



    app->root = root; 
    
    //cls(); 

    //on_resize(app, false);
    //set_term_minsize(app, 40, 10);

    InputEvent event;

    
    while (true)
    {
        bool resized = resize(app);
        if (!resized)
            continue;

        int ret = default_event_handler(app, &event);

        render_ui(app);
        render_screen(app);
    }
    
    

    return 0;
}

