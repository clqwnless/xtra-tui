#include "xtra.h"


#define MAX_LAYOUT_ADAPTERS 4096

typedef void (*PyLayoutFunc)(Widget *w, Rect *r);
typedef struct LayoutAdapter LayoutAdapter;

struct LayoutAdapter
{
    Widget *widget;
    PyLayoutFunc callback;
};

LayoutAdapter layout_adapters[MAX_LAYOUT_ADAPTERS];
size_t layout_adapters_count = 0;;


LayoutAdapter *find_layout_adapter(Widget *w)
{
    for (size_t i = 0; i < layout_adapters_count; ++i)
        if (layout_adapters[i].widget == w)
            return &layout_adapters[i];

    return NULL;
}

Rect layout_trampoline(Widget *w)
{
    Rect r = {0};
    LayoutAdapter *adapter = find_layout_adapter(w);

    if (adapter && adapter->callback)
        adapter->callback(w, &r);

    return r;
}

void set_python_layout(Widget *w, PyLayoutFunc callback)
{
    assert(layout_adapters_count < MAX_LAYOUT_ADAPTERS);
    
    layout_adapters[layout_adapters_count++] = (LayoutAdapter){w, callback};
    
    //w->calc_layout = layout_trampoline;
}

