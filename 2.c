#include "xtra.h"
#include "button.h"


App *app;




Rect ws_root(Widget *w)
{
    return layout_wrapper(0, 0, app->termsize.w, app->termsize.h);
}

BUTTON_CONTAINER(ws_panel, 10, 5, parent.w - 10, parent.h - 5);

BUTTON_START(ws_main_opt, "main");
BUTTON_NEXT(ws_calendar_opt, "ws_main_opt", "calendar");



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

    Widget *button_container = new_button_container(
        "panel",
        ws_panel
    );

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

