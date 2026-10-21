#include "xtra.h"
//#include "calendar.h"
#include "calendar_year.h"


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
    bool got_event;
    
    
    while (true)
    {
        got_event = input_poll(&event);
        
        
        
        if (got_event && event.type == MOUSE_TYPE)
        {
            app->focused = match_cursor(app->root, event.mouse.x, event.mouse.y);
        }
        
        debug_render_bits(0, 1, event.mouse.flags);
        
        if (got_event && app->focused != NULL)
        {
            if (event.type == MOUSE_TYPE)
            {
                
                
                //Rect focused_rect = calc_outer_rect(app->focused);
                
                Rect focused_wsabs = calc_wsabs(app->focused);
                
                //Rect focused_rect = app->focused->calc_layout(app->focused);
                
                app->focused->last_mouse = event.mouse;
                MouseEvent *mouse        = &app->focused->last_mouse;
                
                // calculating relative coords
                
                mouse->x -= focused_wsabs.x;
                mouse->y -= focused_wsabs.y; 
                
                // copying in a base-event
                
                event.mouse = app->focused->last_mouse;
                
                debug(0, 20, L"focused_rect.x=%d, focused_rect.y=%d, mouse event: rels: x=%d, y=%d\n", focused_wsabs.x, focused_wsabs.y, mouse->x, mouse->y);
                
                
            } else if (event.type == KEY_TYPE) {
                app->focused->last_key = event.key;
            }
            
            app->focused->last_event_type = event.type;
        }
        
        
        if (got_event && app->focused != NULL && app->focused->event_func != NULL)
            app->focused->event_func(app, app->focused, event);
        
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







