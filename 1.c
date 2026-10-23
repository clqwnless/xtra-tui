#include "xtra.h"
//#include "calendar.h"
#include "calendar_year.h"











App *app;


LAYOUT_ABS(ws_root, 0, 0, app->termsize.w, app->termsize.h);
LAYOUT_PARENT(ws_calendar, 0, 0, parent.w, parent.h);


void set_term_minsize(App *app, short w, short h)
{
    app->mintermsize.w = w;
    app->mintermsize.h = h;

    app->set_min_termsize = true;
}

bool check_min_termsize(App *app, bool last_result)
{
    if (app->set_min_termsize == false)
        return true;
    
    Size current = app->termsize;
    Size min     = app->mintermsize;
    
    if (current.w < min.w || current.h < min.h)
    {
        if (last_result == true)
            cls();
        
        int y = current.h / 2 - 1;
        
        debug(0,  y,     L"err: expected: minimum width:  %d", min.w);         
        debug(15, y + 1, L"minimum height: %d", min.h);
        debug(0,  y + 3, L"current width:  %d",  current.w);
        debug(0,  y + 4, L"current height: %d", current.h);
        
        
        return false;
    }
    
    return true;
}

bool resize(App *app)
{
    static bool last_check_result = true;
    
    bool checked = check_min_termsize(app, last_check_result);
    
    if (is_term_resized(app))
    {
        on_resize(app);
    }
    
    last_check_result = checked;
    
    return checked;
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
        
        //Sleep(40);
    }
    
    
}







