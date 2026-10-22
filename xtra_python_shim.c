#include "xtra_python_shim.h"


#define MAX_LAYOUT_ADAPTERS 4096

static LayoutAdapter layout_adapters[MAX_LAYOUT_ADAPTERS];
static size_t layout_adapters_count = 0;;


static LayoutAdapter *find_layout_adapter(Widget *w)
{
    for (size_t i = 0; i < layout_adapters_count; ++i)
        if (layout_adapters[i].widget == w)
            return &layout_adapters[i];

    return NULL;
}

XTRA_API Rect layout_trampoline(Widget *w)
{
    Rect r = {0};
    LayoutAdapter *adapter = find_layout_adapter(w);

    if (adapter && adapter->callback)
        adapter->callback(w, &r);

    return r;
}

XTRA_API void set_python_layout(Widget *w, PyLayoutFunc callback)
{
    assert(layout_adapters_count < MAX_LAYOUT_ADAPTERS);
    
    layout_adapters[layout_adapters_count++] = (LayoutAdapter){w, callback};
    
    //w->calc_layout = layout_trampoline;
}

