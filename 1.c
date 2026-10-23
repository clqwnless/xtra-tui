#include "xtra.h"
//#include "calendar.h"
#include "calendar_year.h"











App *app;


LAYOUT_ABS(ws_root, 0, 0, app->termsize.w, app->termsize.h);
LAYOUT_PARENT(ws_calendar, 0, 0, parent.w, parent.h);


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

        int ret = default_event_handler(app, &event);


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







