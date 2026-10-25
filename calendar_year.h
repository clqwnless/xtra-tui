#include "xtra.h"

#include <stdarg.h>


#define init_data(widget) Calendar *calendar = (Calendar*)widget->data;

#define MONTH_ROW_NDAYS 5


typedef struct Calendar  Calendar;
typedef struct Mapping   Mapping;
typedef struct YM        YM;

typedef struct CalendarWidget CalendarWidget;

typedef enum DispatchEvent  DispatchEvent;

/*
const char *months_names[13] = {
    NULL,
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec",
};
*/

const uint32_t *months_names[13] = {
    NULL,
    (const uint32_t[]){'J', 'a', 'n', 0},
    (const uint32_t[]){'F', 'e', 'b', 0},
    (const uint32_t[]){'M', 'a', 'r', 0},
    (const uint32_t[]){'A', 'p', 'r', 0},
    (const uint32_t[]){'M', 'a', 'y', 0},
    (const uint32_t[]){'J', 'u', 'n', 0},
    (const uint32_t[]){'J', 'u', 'l', 0},
    (const uint32_t[]){'A', 'u', 'g', 0},
    (const uint32_t[]){'S', 'e', 'p', 0},
    (const uint32_t[]){'O', 'c', 't', 0},
    (const uint32_t[]){'N', 'o', 'v', 0},
    (const uint32_t[]){'D', 'e', 'c', 0},
};



struct Mapping
{
    int year;
    int month;
    int day;
    int x;
    int y;
    int w;
};

struct Calendar
{
    Widget *root;
    
    int year;
    short scroll_y;
    
    int mapping_ndays;
    
    Mapping *selecting_map_start;
    Mapping *selecting_map_end;

    Mapping *mapping;
};

enum DispatchEvent
{
    EVENT_CHANGE_YM
};


bool is_leap_year(int year)
{
    return year % 400 == 0 ||
          (year % 4 == 0 && year % 100 != 0);
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


int get_max_scroll(Calendar *calendar, int visible_height)
{
    int max_y = 0;

    for (int i = 0; i < calendar->mapping_ndays; i++)
    {
        if (calendar->mapping[i].y > max_y)
            max_y = calendar->mapping[i].y;
    }

    int max_scroll = max_y - visible_height + 1;

    return max_scroll > 0 ? max_scroll : 0;
}

/* the calendar-field widget (calendar rect itself) */


#define MAPPING_DAYS 366


Mapping *get_month_map(Mapping *m0, int year, int month)
{
    int offset = 0;
    
    for (int i = 1; i < month; i++)
    {
        offset += get_month_days(year, i);
    }
    
    //offset += 1;
    return &m0[offset];
}

void update_mapping(Calendar *calendar)
{
    if (calendar->mapping == NULL)
    {
        calendar->mapping = malloc(sizeof(Mapping) * MAPPING_DAYS);
        assert(calendar->mapping != NULL);
    }
    
    
    Widget *calendar_box = find_child(calendar->root, "calendar_box");
    
    // calendar box
    
    Rect rect     = calendar_box->calc_layout(calendar_box);

    /*
    
    month_cols:
    
        s01 02 03 04 05 06 07 08 09 10 11 12s
        
        2 -> space
        
        MONTH_ROW_NDAYS (12) * 3 = 36 
        36 - 1 (last space) = 35
        
        35 + 2 spaces = 37
        
        month_cols = 37
    
    month_rows:
    
        s -> free rows
    
        s 
        01 02 03 04 05 06 07 08 09 10
        11 12 13 14 15 16 17 18 19 20
        21 22 23 24 25 26 27 28 29 30
        31
        s
        
        31 / 10 = 3.1 -> round_to_up -> 4
        
        4 + 2 (two rows) -> 6
        
        month_rows = 6

    */
    
    int month_cols  = ((MONTH_ROW_NDAYS * 3) - 1) + 2;
    int month_rows  = round_to_up((double)31 / MONTH_ROW_NDAYS) + 2; // double нужен иначе там все в int и оно 3 вернет round_to_up неправильным будет
    
    int nmonths_x   = rect.w / month_cols;
    int nmonths_y   = rect.h / month_rows;
    
    
    
    //debug(0, 0, L"nmonths_x=%d, nmonths_y=%d", nmonths_x, nmonths_y);
    //Sleep(10000);
    
    int mapping_ndays = is_leap_year(calendar->year) ? 366 : 365;
    
    assert(nmonths_x > 0 && nmonths_y > 0);
    
    // nmonths_x = 2;
    // nmonths_y = 1;
    
    int month_yoff = 1;
    int month_xoff = 1;
    
    for (int month = 1; month <= 12; month++)
    {
        //int current_month_rows = round_to_up(get_month_days(month) / MONTH_ROW_NDAYS);

        Mapping *month_map = get_month_map(calendar->mapping, calendar->year, month);

        int x_off = month_xoff;
        int y_off = month_yoff;

        int month_days = get_month_days(calendar->year, month);

        for (int day = 1; day <= month_days; day++)
        {
            Mapping *day_map = month_map + (day - 1); // поскольку работаем с индексными данными -> month_map возвращает первый элемент месяца
            
            day_map->year  = calendar->year;
            day_map->month = month;
            day_map->day   = day;
            
            day_map->x     = x_off;
            day_map->y     = y_off;
            day_map->w     = 1;
            
            if ((day % MONTH_ROW_NDAYS) == 0)
            {
                x_off = month_xoff;
                y_off += 1;
            } else {
                x_off += 3;
            } 
        }
        
        if ((month % nmonths_x) == 0)
        {
            month_xoff = 1;
            month_yoff += month_rows;
        } else {
            month_xoff += month_cols;
        }
    }

    calendar->mapping_ndays = mapping_ndays;
}




Mapping *map(int x, int y, Calendar *calendar)
{
    
    
    y += calendar->scroll_y;
    
    for (int day = 0; day < calendar->mapping_ndays; day++)
    {
        Mapping *m = &calendar->mapping[day];
        
        if (x >= m->x && x <= (m->x + m->w) && y == m->y)
        {
            
            //debug(0, 0, L"                                               ");
            //debug(0, 0, L"map: x=%d, y=%d, mapped->day: %d", x, y, m->day);
            return m;
        
        }
    }
    
    return NULL;
}


void event_calendar(App *app, Widget *w, InputEvent event)
{
    init_data(w);
    
    Rect rect = w->calc_layout(w);
    
    int max_scroll = get_max_scroll(calendar, rect.h);
    
    
    
    if (event.type == MOUSE_TYPE)
    {
        if (event.mouse.button == MOUSE_WHEEL_UP && calendar->scroll_y > 0)
        {
            calendar->scroll_y -= 1;
        } else if (event.mouse.button == MOUSE_WHEEL_DOWN && calendar->scroll_y < max_scroll) {
            calendar->scroll_y += 1;
        } else if (event.mouse.button == MOUSE_LEFT && event.mouse.flags & MOUSE_PRESS && (event.mouse.mod & MOD_SHIFT_PRESSED)) {
            calendar->selecting_map_start = map(event.mouse.x, event.mouse.y, calendar);
            calendar->selecting_map_end   = NULL;
            
            /*
            if (selecting_map)
            {
                debug(0, 0, L"                                                                  ");
                debug(0, 0, L"selecting, x=%d, y=%d day: %d", event.mouse.x, event.mouse.y, selecting_map->day);
            } else {
                debug(0, 0, L"not selecting");
            }
            */
        } else if (calendar->selecting_map_start && event.mouse.button == MOUSE_LEFT && event.mouse.flags & (MOUSE_RELEASE | MOUSE_HOLD)) {
            calendar->selecting_map_end = map(event.mouse.x, event.mouse.y, calendar);
            
            
            if (calendar->selecting_map_end)
            {
                debug(0, 4, L"start_day=%d, end_day=%d", calendar->selecting_map_start->day, calendar->selecting_map_end->day);    
            }
        }
        
    }
}

void render_day(App *app, Widget *w, Calendar *calendar, Mapping *day_map, uint16_t attr)
{
    uint32_t codepoints[2];
    char     buffer[3];
    
    snprintf(buffer, ARRAY_SIZE(buffer), "%d", day_map->day);

    //debug(0, 0, L"m->day: %d, day=%d", m->day, day);
    
    if (day_map->day < 10)
    {
        codepoints[0] = (uint32_t)'0';
        codepoints[1] = (uint32_t)buffer[0];
    } else {
        codepoints[0] = (uint32_t)buffer[0];
        codepoints[1] = (uint32_t)buffer[1];
    }
    
    //addstr(app, w, day_map->x, day_map->y, codepoints, 2, attr);
    addstr(app, w, day_map->x, day_map->y - calendar->scroll_y, codepoints, 2, attr);
}

void render_month(App *app, Widget *w, Calendar *calendar, Mapping *first_day, uint16_t attr)
{
    const uint32_t *month_name = months_names[first_day->month];
    
    addstr(app, w, first_day->x, (first_day->y - calendar->scroll_y) - 1, month_name, unicode_strlen(month_name),  attr);
}

void render_calendar(App *app, Widget *w)
{
    clear_widget_sb(app, w);
    
    init_data(w);
    
    Rect rect = w->calc_layout(w);
    
    for (int day = 0; day < calendar->mapping_ndays; day++)
    {
        
        
        Mapping *m = &calendar->mapping[day];
        
        if (m->day == 1)
        {
            render_month(app, w, calendar, m, FG(0, 1, 0));
        }
        
        render_day(app, w, calendar, m, FG(0, 1, 1));
        
    
    }
    

    
    
    Mapping *selected_day = map(w->last_mouse.x, w->last_mouse.y, calendar);
    if (selected_day != NULL)
    {
        render_day(app, w, calendar, selected_day, FG(1, 1, 0));
    }
    
    if (calendar->selecting_map_start && calendar->selecting_map_end)
    {
        Mapping *start = calendar->selecting_map_start;
        Mapping *end   = calendar->selecting_map_end;
        
        if (end < start)
        {
            start = end;
            end   = calendar->selecting_map_start;
        }
        
        Mapping *m = start;
        
        while (m <= end)
        {
            render_day(app, w, calendar, m, FG(1, 1, 0));
            m++;
        }
        
    }
    
}

void resize_calendar(App *app, Widget *w)
{
    init_data(w);
    
    update_mapping(calendar);
    
    Rect rect = w->calc_layout(w);
    
    int max_scroll = get_max_scroll(calendar, rect.h);
    
    if (calendar->scroll_y > max_scroll)
        calendar->scroll_y = max_scroll;
    
    w->last_mouse.x = -1;
    w->last_mouse.y = -1;
}



/* the year && month selection widget */


LAYOUT_PARENT(ws_yearmonth_field, 0, 0, parent.w, 3);
LAYOUT_ABS(ws_leftarrow, 0, 0, 2, 1);
LAYOUT_PARENT(ws_rightarrow, parent.w - 2, 0, 2, 1);
LAYOUT_MARGIN_BOTTOM(ws_calendar_box, "ym_field", margin.x, margin.y, margin.w, margin.h);


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
            calendar->year -= 1;
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
            calendar->year += 1;
            dispatch_calendar(EVENT_CHANGE_YM, w);
        }
        
    }
    
}

void render_ym(App *app, Widget *w)
{
    clear_widget_sb(app, w);
    
    init_data(w);
    
    Rect rect = w->calc_layout(w);

    uint32_t *str = unicode_conv("%d", calendar->year);
    int len = unicode_strlen(str);
    
    //debug(0, 0, L"here, len: %d", unicode_strlen(str));
    
    addstr(app, w, (rect.w / 2) - (len / 2), 0, str, len, FG(1, 1, 1));
}





Widget *new_calendar(const char *id, LayoutFunc layout_func)
{
    Widget *container = new_widget(
        id,
        (WidgetMethods){
            .calc_layout = layout_func
        },
        WIDGET_IS_VISIBLE | WIDGET_HAS_FRAME
    );
     
    Widget *ym_field = new_widget(
        "ym_field",
        (WidgetMethods){
            .calc_layout = ws_yearmonth_field,
            .render_func = render_ym
        },
        WIDGET_IS_VISIBLE | WIDGET_HAS_FRAME
    );
    
    Widget *calendar_box = new_widget(
        "calendar_box",
        (WidgetMethods){
            .calc_layout = ws_calendar_box,
            .render_func = render_calendar,
            .event_func  = event_calendar,
            .resize_func = resize_calendar
        },
        WIDGET_IS_VISIBLE
    );
    
    Widget *left_arrow = new_widget(
        "left_arrow",
        (WidgetMethods){
            .calc_layout = ws_leftarrow,
            .render_func = render_leftarrow,
            .event_func  = event_leftarrow
        },
        WIDGET_IS_VISIBLE
    );
    
    Widget *right_arrow = new_widget(
        "right_arrow",
        (WidgetMethods){
            .calc_layout = ws_rightarrow,
            .render_func = render_rightarrow,
            .event_func  = event_rightarrow
        },
        WIDGET_IS_VISIBLE
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
    
    ptr->mapping = NULL;
    ptr->mapping_ndays = 0;
    
    ptr->scroll_y = 0;
    
    ptr->selecting_map_start = NULL;
    ptr->selecting_map_end   = NULL;

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



