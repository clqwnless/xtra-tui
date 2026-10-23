#include "xtra.h"
//#include "calendar.h"
#include "calendar_year.h"


#define LAYOUT_PARENT(name, ...) \
    Rect name(Widget *w) { \
        Rect parent = get_parent_rect(w); \
        return layout_wrapper(__VA_ARGS__); \
    }

#define LAYOUT_ABS(name, ...) \
    Rect name(Widget *w) { \
        return layout_wrapper \
    }





App *app;

Rect ws_root(Widget *w)
{
    //printf("termsize.w=%d\n", app->termsize.w);
    //printf("termsize.h=%d\n", app->termsize.h);
    
    
    return layout_wrapper(0, 0, app->termsize.w, app->termsize.h);
}




Rect ws_calendar(Widget *w)
{
    //printf("ws_calendar\n");
    Rect r = get_parent_rect(w);
    return layout_wrapper(0, 0, r.w, r.h);
    //return layout_wrapper(0, 0, r.w / 3, r.h);
}



#define NO_EVENT    -1
#define NOT_FOCUSED -2
#define FOCUSED     1

int focus(App *app, InputEvent *event)
{
    bool got_event = input_poll(event);
 
    if (got_event == false)
        return NO_EVENT;
    
    if (event->type == MOUSE_TYPE)    
    {
        app->focused       = match_cursor(app->root, event->mouse.x, event->mouse.y);
        
        if (app->focused == NULL)
            return NOT_FOCUSED;
        
        Rect focused_wsabs = calc_wsabs(app->focused);
        
        
        //Rect focused_rect = app->focused->calc_layout(app->focused);
        
        app->focused->last_mouse = event->mouse;
        
        // calculating relative coords
        
        app->focused->last_mouse.x -= focused_wsabs.x;
        app->focused->last_mouse.y -= focused_wsabs.y;
        
        // copying in a base-event
        
        event->mouse = app->focused->last_mouse;
    } else if (event->type == KEY_TYPE) {
        app->focused->last_key = event->key;
    }
    
    app->focused->last_event_type = event->type;
    
    return FOCUSED;
}

void dispatch_events_to_focused(App *app, InputEvent *event, int focus_ret)
{
    Widget *focused = app->focused;
    if (focus_ret != NO_EVENT && focused != NULL && focused->event_func != NULL)
        focused->event_func(app, focused, *event);
}

int event_handler(App *app, InputEvent *event)
{
    int ret = focus(app, event);
    dispatch_events_to_focused(app, event, ret);
    return ret;
}


int main(void) {    
    App a = new_app();
    app   = &a;
    
    init(app);
   
    Widget *root     = new_widget("root",     (WidgetMethods){ws_root, NULL, NULL, NULL, NULL}, WIDGET_NONE_FLAGS);
    Widget *calendar = new_calendar("calendar", ws_calendar);
    
    app->root    = root;
    app->focused = calendar;
    
    register_widget(calendar, root);
    
    
    cls(); 
    on_resize(app);
    
    
    
    InputEvent event;

    while (true)
    {

        int ret = event_handler(app, &event);


        if (is_term_resized(app))
        {
            //cls();
            on_resize(app);
        }

        render_ui(app);
        render_screen(app);
        
        //Sleep(40);
    }
    
    
}







