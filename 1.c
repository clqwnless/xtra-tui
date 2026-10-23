#include "xtra.h"
//#include "calendar.h"
#include "calendar_year.h"

#include <time.h>






App *app;


LAYOUT_ABS(ws_root, 0, 0, app->termsize.w, app->termsize.h);
LAYOUT_PARENT(ws_calendar, 0, 0, parent.w, parent.h);


void set_term_minsize(App *app, short w, short h)
{
    app->mintermsize.w = w;
    app->mintermsize.h = h;

    app->set_min_termsize = true;
}

bool less_than_minsize(App *app)
{
    if (app->set_min_termsize == false)
        return false;
    
    return (app->termsize.w < app->mintermsize.w) || (app->termsize.h < app->mintermsize.h);
}

bool resize(App *app)
{
    bool is_less      = less_than_minsize(app);
    bool term_resized = is_term_resized(app);

    if (term_resized)
    {
        /*
        
        if the terminal size is less than minimum-size then it doesn't make any sense to resize widget tree
        since widgets should not be rendered anyway
        
        that is, if the current function returns false -> not render screen
        else: render
        
        */
        
        on_resize(app, !is_less);
    }

    if (is_less)
    {
        if (term_resized)
            cls();
        
        Size current = app->termsize;
        Size min     = app->mintermsize;
        
        int y = current.h / 2 - 1;
        
        render_screen(app);
        
        debug(0,  y,     L"err: expected: minimum width:  %d", min.w);         
        debug(15, y + 1, L"minimum height: %d", min.h);
        debug(0,  y + 3, L"current width:  %d",  current.w);
        debug(0,  y + 4, L"current height: %d", current.h);
    }
    
    /*
      invert (if is_less is true then the resizing is failed)
      if is_less is false then everything is fine
      (the termsize corresponds (or is greater than) to the minsize)
    */
   
    return !is_less;
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
    on_resize(app, true);
    
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







