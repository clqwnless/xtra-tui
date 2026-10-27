#include "xtra.h"
#include "button.h"
#include "calendar_year.h"


App *app;
Widget *calendar;
Widget *root;


enum {
    OPEN_MAIN,
    OPEN_CALENDAR
};


Rect ws_root(Widget *w)
{
    return layout_wrapper(0, 0, app->termsize.w, app->termsize.h);
}

BUTTON_CONTAINER(ws_panel, 0, 0, parent.w, 3);

BUTTON_START(ws_main_opt, "main");
BUTTON_NEXT(ws_calendar_opt, "ws_main_opt", "calendar");


LAYOUT_MARGIN_BOTTOM(ws_calendar, "panel", margin.x, margin.y, margin.w, margin.h);



void button_dispatch(int event)
{
    switch (event)
    {
        case OPEN_CALENDAR:
            register_widget(calendar, root);
            break;
    }
    
    //debug(0, 0, L"got event: %d", event);
}




int main(void)
{
    App a = new_app();
    app = &a;
    
    init(app);
    
    ButtonStack stack = new_button_stack(button_dispatch);
    
    root = new_widget(
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

    Widget *button_container = new_button_container(
        "panel",
        ws_panel
    );

    calendar = new_calendar("calendar", ws_calendar);

    register_widget(button_container, root);
    register_widget(opt_main, button_container);
    register_widget(opt_calendar, button_container);



    app->root = root;
    
    cls(); 

    on_resize(app, false);
    set_term_minsize(app, 40, 10);

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

