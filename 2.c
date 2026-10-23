#include "xtra.h"


#


App *app;


LAYOUT_DEFAULT(ws_root, 0, 0, app->termsize.w, app->termsize.h);

LAYOUT_PARENT(ws_panel_container, 1, 1, parent.w - 1, 1);


LAYOUT_PARENT(ws_settings_panel, 0, 0, sizeof("settings"), 1);




int main(void)
{
    App a = new_app();
    app = &a;
    
    
    
    
    return 0;
}