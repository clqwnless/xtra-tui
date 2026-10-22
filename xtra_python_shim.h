#ifndef XTRA_PYTHON_SHIM
#define XTRA_PYTHON_SHIM

#include "xtra.h"


typedef void (*PyLayoutFunc)(Widget *w, Rect *r);
typedef struct LayoutAdapter LayoutAdapter;

struct LayoutAdapter
{
    Widget *widget;
    PyLayoutFunc callback;
};

XTRA_API Rect layout_trampoline(Widget *w);
XTRA_API void set_python_layout(Widget *w, PyLayoutFunc callback);

#endif /* XTRA_PYTHON_SHIM */
