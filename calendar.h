#include "xtra.h"

#include <stdarg.h>


#define LAST_SCROLL(calendar) calendar->mapping[calendar->mapping_ndays - 1].y
#define is_valid_scroll(calendar) calendar->scroll_y < LAST_SCROLL(calendar)
#define init_data(widget) Calendar *calendar = (Calendar*)widget->data;


typedef struct Calendar  Calendar;
typedef struct Mapping   Mapping;
typedef struct YM        YM;

typedef struct CalendarWidget CalendarWidget;

typedef enum DispatchEvent  DispatchEvent;


uint32_t *unicode_conv(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    
    static char     utf8_buffer[4096];  
    static uint32_t cp_buffer[4096];
    
    vsnprintf(utf8_buffer, ARRAY_SIZE(utf8_buffer), fmt, args);
    
    debug(0, 14, L"utf8_buffer len: %d, array_size: %d", strlen(utf8_buffer), ARRAY_SIZE(utf8_buffer));
    
    utf8_to_unicode(utf8_buffer, cp_buffer, ARRAY_SIZE(cp_buffer));
    
    va_end(args);
    
    return cp_buffer;
}



struct Mapping
{
    int day;
    int x;
    int y;
    int w;
};

struct Calendar
{
    Widget *root;
    
    int year;
    int month;
    int day;

    short scroll_y;
    
    int selected_day;
    
    int mapping_ndays;
    int row_ndays;
    Mapping *mapping;
};

enum DispatchEvent
{
    EVENT_CHANGE_YM
};


bool is_leap_year(int year) {
    if (year % 400 == 0) {
        return true;
    }
    if (year % 100 == 0) {
        return true;
    }
    if (year % 4 == 0) {
        return true;
    }
    return false;
}

int get_month_days(int year, int month)
{
    switch (month)
    {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            return 31;
        
        case 4:
        case 6:
        case 9:
        case 11:
            return 30;
        
        case 2:
            return is_leap_year(year) ? 29 : 28;
        
        default:
            return -1;
    }
}

int round_to_up(double x)
{
    int n = (int)x;
    
    if (x > n)
        return n + 1;
    return n;
}


/* the calendar-field widget (calendar rect itself) */

void update_mapping(Calendar *calendar)
{
    Widget *calendar_box = find_child(calendar->root, "calendar_box");
    
    // calendar box
    
    Rect rect     = calendar_box->calc_layout(calendar_box);

    int days      = get_month_days(calendar->year, calendar->month);
    
    int ndays_row = round_to_up((rect.w - 2) / 3);
    int nrows     = round_to_up(days / ndays_row);
    
    calendar->mapping = realloc(calendar->mapping, sizeof(Mapping) * (days + 1)); // day + 1 because we ignore day[0], the first day is 1 and i don't want always doing -1
    assert(calendar->mapping != NULL);
    
    calendar->mapping_ndays = days;
    calendar->row_ndays     = ndays_row;
    
    if (days < 0 || ndays_row < 0 || nrows < 0)
        return;
    
    // clean mapping

    for (int day = 1; day <= days; day++)
    {
        calendar->mapping[day].day = -1;
    }
    
    // fill mapping with days
    
    int x_off = 1;
    int y_off = 0;
    
    for (int day = 1; day <= days; day++)
    {
        Mapping *m = &calendar->mapping[day];
        
        m->day = day;
        
        m->x   = x_off;
        m->y   = y_off;
        m->w   = 1;
        
        if ((day % ndays_row) == 0)
        {
            x_off = 1;
            y_off += 1;
        } else {
            x_off += 3;
        }
    }
}

Mapping *map(int x, int y, Calendar *calendar)
{
    y += calendar->scroll_y;
    
    for (int day = 1; day <= calendar->mapping_ndays; day++)
    {
        Mapping *m = &calendar->mapping[day];
        
        if (x >= m->x && x <= (m->x + m->w) && y == m->y)
            return m;
    }
    
    return NULL;
}


void event_calendar(App *app, Widget *w, InputEvent event)
{
    init_data(w);
    
    if (event.type == MOUSE_TYPE)
    {        
        if (event.mouse.button == MOUSE_WHEEL_UP && calendar->scroll_y > 0)
        {
            calendar->scroll_y -= 1;
        } else if (event.mouse.button == MOUSE_WHEEL_DOWN && is_valid_scroll(calendar)) {
            calendar->scroll_y += 1;
        }
        
        
        
        //wchar_t buffer[1024];
        
        //swprintf(buffer, 1024, L"last_x=%d, last_y=%d\n", calendar->last_x, calendar->last_y);
        //debug(0, 0, L"last_x: %d, last_y: %d", calendar->last_x, calendar->last_y);
        
    }
}

void render_day(App *app, Widget *w, Calendar *calendar, int day, uint16_t attr)
{
    Mapping *m = &calendar->mapping[day];
    
    uint32_t codepoints[2];
    char     buffer[3];
    
    snprintf(buffer, ARRAY_SIZE(buffer), "%d", m->day);

    //debug(0, 0, L"m->day: %d, day=%d", m->day, day);
    
    if (day < 10)
    {
        codepoints[0] = (uint32_t)'0';
        codepoints[1] = (uint32_t)buffer[0];
    } else {
        codepoints[0] = (uint32_t)buffer[0];
        codepoints[1] = (uint32_t)buffer[1];
    }
    
    addstr(app, w, m->x, m->y - calendar->scroll_y, codepoints, 2, attr);
}

void render_calendar(App *app, Widget *w)
{
    clear_widget_sb(app, w);
    
    init_data(w);
    
    for (int day = calendar->scroll_y * calendar->row_ndays + 1; day <= calendar->mapping_ndays; day++)
    {
        render_day(app, w, calendar, day, FG(1, 1, 1));
    }
    
    
    
    
    Mapping *selected_day = map(w->last_mouse.x, w->last_mouse.y, calendar);
    if (selected_day != NULL)
    {
        render_day(app, w, calendar, selected_day->day, FG(1, 1, 0));
    }
    
    
    
}

void resize_calendar(App *app, Widget *w)
{
    init_data(w);
    
    update_mapping(calendar);
    
    if (is_valid_scroll(calendar) == false)
    {
        calendar->scroll_y = LAST_SCROLL(calendar);
    }        
    
    w->last_mouse.x = -1;
    w->last_mouse.y = -1;
}


/* the year && month selection widget */

Rect ws_yearmonth_field(Widget *w)
{
    Rect rect = get_parent_rect(w);    
    return layout_wrapper(0, 0, rect.w, 3);
}

Rect ws_leftarrow(Widget *w)
{
    return layout_wrapper(0, 0, 2, 1);
}

Rect ws_rightarrow(Widget *w)
{
    Rect rect = get_parent_rect(w);
    return layout_wrapper(rect.w - 2, 0, 2, 1);
}

void dispatch_calendar(DispatchEvent event, Widget *w)
{
    if (event == EVENT_CHANGE_YM)
    {
        init_data(w);
        update_mapping(calendar);
        
        calendar->scroll_y = 0;
    }
    
}


void render_leftarrow(App *app, Widget *w)
{
    uint32_t *str = unicode_conv("←-");
    addstr(app, w, 0, 0, str, 2, FG(1, 1, 0));
}

void render_rightarrow(App *app, Widget *w)
{
    uint32_t *str = unicode_conv("-→");
    addstr(app, w, 0, 0, str, 2, FG(1, 1, 0));
}

void event_leftarrow(App *app, Widget *w, InputEvent event)
{
    init_data(w);
    
    if (event.type == MOUSE_TYPE)
    {
        if (event.mouse.button == MOUSE_LEFT && event.mouse.flags & MOUSE_PRESS)
        {
            
            if (calendar->month == 1)
            {
                calendar->year  -= 1;
                calendar->month = 12;
            } else {
                calendar->month -= 1;
            }
            
            dispatch_calendar(EVENT_CHANGE_YM, w);
        }
        
    }
    

}

void event_rightarrow(App *app, Widget *w, InputEvent event)
{
    init_data(w);
    
    if (event.type == MOUSE_TYPE)
    {
        if (event.mouse.button == MOUSE_LEFT && event.mouse.flags & MOUSE_PRESS)
        {
            if (calendar->month == 12)
            {
                calendar->year += 1;
                calendar->month = 1;
            } else {
                calendar->month += 1;
            }
            
            dispatch_calendar(EVENT_CHANGE_YM, w);
            
        }
        
    }
    
}

void render_ym(App *app, Widget *w)
{
    clear_widget_sb(app, w);
    
    init_data(w);
    
    Rect rect = w->calc_layout(w);

    uint32_t *str = unicode_conv("%d/%d", calendar->month, calendar->year);
    int len = unicode_strlen(str);
    
    //debug(0, 0, L"here, len: %d", unicode_strlen(str));
    
    addstr(app, w, (rect.w / 2) - (len / 2), 0, str, len, FG(1, 1, 1));
}

Rect ws_calendar_box(Widget *w)
{
    //return layout_wrapper(1, 1, 5, 5);
    
    Rect rect = margin_bottom(w, "ym_field");
    return layout_wrapper(rect.x, rect.y, rect.w, rect.h);
}





Widget *new_calendar(const char *id, LayoutFunc layout_func)
{
    Widget *container = new_widget(
        id,
        layout_func,
        NULL,
        NULL,
        NULL,
        NULL,
        true,
        false,
        true
    );
    
    Widget *ym_field = new_widget(
        "ym_field",
        ws_yearmonth_field,
        NULL,
        render_ym,
        NULL,
        NULL,
        true,
        false,
        true
    );
    
    Widget *calendar_box = new_widget(
        "calendar_box",
        ws_calendar_box,
        NULL,
        render_calendar,
        event_calendar,
        resize_calendar,
        
        true,
        true,
        true
    );
    
    Widget *left_arrow = new_widget(
        "left_arrow",
        ws_leftarrow,
        NULL,
        render_leftarrow,
        event_leftarrow,
        NULL,
        
        true,
        true,
        false
    );
    
    Widget *right_arrow = new_widget(
        "right_arrow",
        ws_rightarrow,
        NULL,
        render_rightarrow,
        event_rightarrow,
        NULL,
        
        true,
        true,
        false
    );
    
    assert(container != NULL);
    assert(ym_field != NULL);
    assert(calendar_box != NULL);
    assert(left_arrow != NULL);
    assert(right_arrow != NULL);
    
    Calendar *ptr = malloc(sizeof(Calendar));
    assert(ptr != NULL);

    ptr->root  = container;

    ptr->year  = 2026;
    ptr->month = 1;
    ptr->day   = 3;
    
    ptr->mapping = NULL;
    ptr->mapping_ndays = 0;
    ptr->row_ndays = 0;
    
    ptr->scroll_y = 0;

    // update_mapping(&calendar, ptr); // should be called in on_resize
    
    container->data    = (void*)ptr;
    ym_field->data     = (void*)ptr;
    calendar_box->data = (void*)ptr;
    left_arrow->data   = (void*)ptr;
    right_arrow->data  = (void*)ptr;
    
    
    
    register_widget(calendar_box, container);
    register_widget(ym_field, container);
    register_widget(left_arrow, ym_field);
    register_widget(right_arrow, ym_field);
    
    return container;
}



