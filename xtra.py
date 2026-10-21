# xtra.py
# ctypes bindings for xtra.h (Windows only)
# Python 3.10+
#
# Важно:
# 1. Путь к DLL задаётся в XTRA_DLL.
# 2. Callback-и LayoutFunc/CursorFunc с возвратом struct by value
#    могут быть ABI-зависимыми. Для Python callback используйте shim.
# 3. Структуры App/Widget ниже открыты полностью, как в xtra.h.

import ctypes as ct
import os

# ---------------------------------------------------------------------
# DLL loading
# ---------------------------------------------------------------------



#XTRA_DLL = os.environ.get("XTRA_DLL", "xtra.dll")
#lib = ct.CDLL(XTRA_DLL)  # cdecl; для __stdcall используйте WinDLL только если API так собран

lib = ct.CDLL("xtra.dll");

# ---------------------------------------------------------------------
# C scalar types
# ---------------------------------------------------------------------

BOOL_C  = ct.c_bool
SHORT   = ct.c_short
UINT16  = ct.c_uint16
UINT32  = ct.c_uint32
INT     = ct.c_int
SIZE_T  = ct.c_size_t
WCHAR_P = ct.c_wchar_p
CHAR_P  = ct.c_char_p
VOID_P  = ct.c_void_p

# ---------------------------------------------------------------------
# Forward declarations
# ---------------------------------------------------------------------

class Event(ct.Structure):
    pass

class App(ct.Structure):
    pass

class Widget(ct.Structure):
    pass

class Rect(ct.Structure):
    pass

class WidgetMethods(ct.Structure):
    pass

class InputEvent(ct.Structure):
    pass

class Coord(ct.Structure):
    pass

class Cursor(ct.Structure):
    pass

class Size(ct.Structure):
    pass

class RectDelta(ct.Structure):
    pass

class Cell(ct.Structure):
    pass

class KeyEvent(ct.Structure):
    pass

class MouseEvent(ct.Structure):
    pass

# ---------------------------------------------------------------------
# Enums (C enum is normally int)
# ---------------------------------------------------------------------

# enum Key

KEY_DOWN = 0
KEY_RIGHT = 1
KEY_UP = 2
KEY_LEFT = 3
KEY_ENTER = 4
KEY_ESCAPE = 5
KEY_UNKNOWN = 6

# enum InputType

KEY_TYPE = 0
MOUSE_TYPE = 1
NONE_TYPE = 2

# enum MouseFlags

MOUSE_PRESS = 1 << 0
MOUSE_RELEASE = 1 << 1
MOUSE_MOVE = 1 << 2
MOUSE_HOLD = 1 << 3

# enum MouseButton

MOUSE_LEFT = 0
MOUSE_RIGHT = 1
MOUSE_MIDDLE = 2
MOUSE_WHEEL_UP = 3
MOUSE_WHEEL_DOWN = 4

# enum MouseMod

MOD_SHIFT_PRESSED = 1 << 0
MOD_LEFT_ALT_PRESSED = 1 << 1
MOD_RIGHT_ALT_PRESSED = 1 << 2
MOD_LEFT_CTRL_PRESSED = 1 << 3
MOD_RIGHT_CTRL_PRESSED = 1 << 4

# enum (WIDGET_FLAGS)

WIDGET_NONE_FLAGS = 0
WIDGET_IS_VISIBLE = 1 << 0
WIDGET_IS_FOCUSABLE = 1 << 1
WIDGET_HAS_FRAME = 1 << 2
WIDGET_IS_FRAME = 1 << 3

# ---------------------------------------------------------------------
# Callback signatures
#
# LayoutFunc/CursorFunc return structs by value. To keep the ABI layout
# exact without relying on ctypes struct-return callbacks, the function
# pointer slots inside Widget/WidgetMethods are represented as c_void_p.
# Use the C shim for Python implementations of those callbacks.
# ---------------------------------------------------------------------

RenderFunc = ct.CFUNCTYPE(None, ct.POINTER(App), ct.POINTER(Widget))
ResizeFunc = ct.CFUNCTYPE(None, ct.POINTER(App), ct.POINTER(Widget))
EventFunc = ct.CFUNCTYPE(
    None,
    ct.POINTER(App),
    ct.POINTER(Widget),
    ct.POINTER(InputEvent),
)

# ---------------------------------------------------------------------
# Structures
# ---------------------------------------------------------------------

Coord._fields_ = [
    ("x", SHORT),
    ("y", SHORT),
]

Size._fields_ = [
    ("w", SHORT),
    ("h", SHORT),
]

Rect._fields_ = [
    ("x", SHORT),
    ("y", SHORT),
    ("w", SHORT),
    ("h", SHORT),
]

RectDelta._fields_ = [
    ("dx", SHORT),
    ("dy", SHORT),
    ("dw", SHORT),
    ("dh", SHORT),
]

Cell._fields_ = [
    ("codepoint", UINT32),
    ("attr", UINT16),
]

Cursor._fields_ = [
    ("x", SHORT),
    ("y", SHORT),
    ("hidden", BOOL_C),
]

Event._fields_ = [
    ("id", INT),
    ("source", ct.POINTER(Widget)),
    ("data", VOID_P),
]

KeyEvent._fields_ = [
    ("key", INT),
    ("codepoint", UINT32),
    ("pressed", BOOL_C),
    ("ctrl", BOOL_C),
    ("alt", BOOL_C),
    ("shift", BOOL_C),
]

MouseEvent._fields_ = [
    ("x", SHORT),
    ("y", SHORT),
    ("button", INT),
    ("flags", UINT32),
    ("mod", UINT32),
]

class InputEventUnion(ct.Union):
    _fields_ = [
        ("key", KeyEvent),
        ("mouse", MouseEvent),
    ]

InputEvent._anonymous_ = ("value",)
InputEvent._fields_ = [
    ("type", INT),
    ("value", InputEventUnion),
]

WidgetMethods._fields_ = [
    ("calc_layout", VOID_P),
    ("calc_cursor", VOID_P),
    ("render_func", RenderFunc),
    ("event_func",  EventFunc),
    ("resize_func", ResizeFunc),
]

App._fields_ = [
    ("root", ct.POINTER(Widget)),
    ("focused", ct.POINTER(Widget)),
    ("overlay", ct.POINTER(Widget)),
    ("overlay_count", UINT16),
    ("termsize", Size),
    ("oldScreenBuffer", ct.POINTER(Cell)),
    ("screenBuffer", ct.POINTER(Cell)),
]

Widget._fields_ = [
    ("id", ct.c_char * MAX_ID_LENGTH),
    
    ("flags", UINT32),
    ("calc_layout", VOID_P),
    ("calc_cursor", VOID_P),
    ("render_func", RenderFunc),
    ("event_func", EventFunc),
    ("resize_func", ResizeFunc),
    
    ("delta", RectDelta),
    ("data", VOID_P),
    
    ("last_event_type", INT),
    ("last_mouse", MouseEvent),
    ("last_key", KeyEvent),
    ("frame", ct.POINTER(Widget)),
    
    ("parent", ct.POINTER(Widget)),
    ("children", ct.POINTER(ct.POINTER(Widget))),
    ("child_count", UINT32),
]

# ---------------------------------------------------------------------
# Function binding helper
# ---------------------------------------------------------------------

def _bind(name, restype, argtypes):
    fn          = getattr(lib, name)
    fn.restype  = restype
    fn.argtypes = argtypes
    return fn

# ---------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------

_clear_sb = _bind("clear_sb", None, [ct.POINTER(App)])
_find_child = _bind("find_child", ct.POINTER(Widget), [ct.POINTER(Widget), CHAR_P])
_debug_render_bits = _bind("debug_render_bits", None, [SHORT, SHORT, INT])

_init = _bind("init", None, [ct.POINTER(App)])
_new_app = _bind("new_app", App, [])

_get_term_size = _bind("get_term_size", Size, [])
_is_term_resized = _bind("is_term_resized", BOOL_C, [ct.POINTER(App)])
_on_resize = _bind("on_resize", None, [ct.POINTER(App)])

_match_cursor = _bind(
    "match_cursor",
    ct.POINTER(Widget),
    [ct.POINTER(Widget), SHORT, SHORT],
)
_get_cursor = _bind("get_cursor", Cursor, [ct.POINTER(App)])
_show_cursor = _bind("show_cursor", None, [BOOL_C])

_new_widget = _bind(
    "new_widget",
    ct.POINTER(Widget),
    [CHAR_P, WidgetMethods, UINT32],
)
_register_widget = _bind(
    "register_widget",
    INT,
    [ct.POINTER(Widget), ct.POINTER(Widget)],
)
_calc_wsabs = _bind("calc_wsabs", Rect, [ct.POINTER(Widget)])
_deregister_widget = _bind(
    "deregister_widget",
    None,
    [ct.POINTER(Widget), ct.POINTER(Widget)],
)
_pop_overlay = _bind("pop_overlay", None, [ct.POINTER(App), ct.POINTER(Widget)])
_push_overlay = _bind("push_overlay", None, [ct.POINTER(App), ct.POINTER(Widget)])

_input_poll = _bind("input_poll", BOOL_C, [ct.POINTER(InputEvent)])

_cls_sb = _bind("cls_sb", None, [ct.POINTER(App)])
_clear_widget_sb = _bind("clear_widget_sb", None, [ct.POINTER(App), ct.POINTER(Widget)])
_render_widget = _bind("render_widget", None, [ct.POINTER(App), ct.POINTER(Widget)])
_addstr = _bind(
    "addstr",
    None,
    [
        ct.POINTER(App),
        ct.POINTER(Widget),
        SHORT,
        SHORT,
        ct.POINTER(UINT32),
        SIZE_T,
        UINT16,
    ],
)

_cls = _bind("cls", None, [])
_render_screen = _bind("render_screen", None, [ct.POINTER(App)])

# C varargs: ctypes can call variadic functions, but argtypes only describe
# the fixed arguments. Pass a format string and matching C values.
_debug = getattr(lib, "debug")
_debug.restype = None
_debug.argtypes = [SHORT, SHORT, WCHAR_P]

_is_in_rect = _bind("is_in_rect", BOOL_C, [Rect, INT, INT])
_layout_wrapper = _bind("layout_wrapper", Rect, [INT, INT, INT, INT])
_margin_right = _bind("margin_right", Rect, [ct.POINTER(Widget), CHAR_P])
_margin_bottom = _bind("margin_bottom", Rect, [ct.POINTER(Widget), CHAR_P])

_utf8_decode = _bind("utf8_decode", INT, [CHAR_P, ct.POINTER(UINT32)])
_utf8_to_unicode = _bind(
    "utf8_to_unicode",
    INT,
    [CHAR_P, ct.POINTER(UINT32), SIZE_T],
)
_unicode_strlen = _bind("unicode_strlen", INT, [ct.POINTER(UINT32)])

# These symbols exist only when building the external-DLL variant.
# Binding them conditionally lets this module also load LIB_NO_DLL builds
# that do not export these names.
def _bind_optional(name, restype, argtypes):
    try:
        return _bind(name, restype, argtypes)
    except AttributeError:
        return None

_FG = _bind_optional("FG", UINT16, [BOOL_C, BOOL_C, BOOL_C])
_BG = _bind_optional("BG", UINT16, [BOOL_C, BOOL_C, BOOL_C])
_get_parent_rect = _bind_optional("get_parent_rect", Rect, [ct.POINTER(Widget)])
_find_rel_widget = _bind_optional(
    "find_rel_widget",
    ct.POINTER(Widget),
    [ct.POINTER(Widget), CHAR_P],
)
_render_ui = _bind_optional("render_ui", None, [ct.POINTER(App)])

# ---------------------------------------------------------------------
# Plain Python functions: no wrapper classes
# ---------------------------------------------------------------------

def clear_sb(app):
    _clear_sb(app)

def find_child(widget, widget_id):
    return _find_child(widget, _as_bytes(widget_id))

def debug_render_bits(x, y, value):
    _debug_render_bits(x, y, value)

def init(app):
    _init(app)

def new_app():
    return _new_app()

def get_term_size():
    return _get_term_size()

def is_term_resized(app):
    return bool(_is_term_resized(app))

def on_resize(app):
    _on_resize(app)

def match_cursor(root, x, y):
    return _match_cursor(root, x, y)

def get_cursor(app):
    return _get_cursor(app)

def show_cursor(flag):
    _show_cursor(flag)

def new_widget(widget_id, methods=None, flags=WIDGET_NONE_FLAGS):
    if methods is None:
        methods = WidgetMethods()
    return _new_widget(_as_bytes(widget_id), methods, flags)

def make_widget_methods(calc_layout=None, calc_cursor=None,
                        render_func=None, event_func=None, resize_func=None):
    """Build WidgetMethods. Keep callback objects alive with keep_callback().

    calc_layout/calc_cursor should be C-shim trampolines or native C callbacks;
    their C ABI returns structs by value.
    """
    methods = WidgetMethods()
    methods.calc_layout = _callback_address(calc_layout)
    methods.calc_cursor = _callback_address(calc_cursor)
    methods.render_func = _callback_address(render_func)
    methods.event_func = _callback_address(event_func)
    methods.resize_func = _callback_address(resize_func)
    for cb in (calc_layout, calc_cursor, render_func, event_func, resize_func):
        if cb is not None:
            keep_callback(cb)
    return methods

def _callback_address(callback):
    if callback is None:
        return None
    return ct.cast(callback, ct.c_void_p).value

def register_widget(widget, parent):
    return _register_widget(widget, parent)

def calc_wsabs(widget):
    return _calc_wsabs(widget)

def deregister_widget(parent, widget):
    _deregister_widget(parent, widget)

def pop_overlay(app, overlay):
    _pop_overlay(app, overlay)

def push_overlay(app, overlay):
    _push_overlay(app, overlay)

def input_poll():
    event = InputEvent()
    ok = bool(_input_poll(ct.byref(event)))
    return event if ok else None

def cls_sb(app):
    _cls_sb(app)

def clear_widget_sb(app, widget):
    _clear_widget_sb(app, widget)

def render_widget(app, widget):
    _render_widget(app, widget)

def addstr(app, widget, x, y, codepoints, attr):
    # codepoints: iterable of Unicode code points (integers)
    values = list(codepoints)
    array_type = UINT32 * len(values)
    array = array_type(*values)
    _addstr(app, widget, x, y, array, len(values), attr)

def cls():
    _cls()

def render_screen(app):
    _render_screen(app)

def debug(x, y, fmt, *args):
    # This is a C variadic function. Keep this simple wrapper for formats
    # without additional varargs. For variadic formatting, prefer a C shim.
    if args:
        raise NotImplementedError(
            "debug() is variadic; add a small C shim for extra arguments."
        )
    _debug(x, y, fmt)

def is_in_rect(rect, x, y):
    return bool(_is_in_rect(rect, x, y))

def layout_wrapper(x, y, w, h):
    return _layout_wrapper(x, y, w, h)

def margin_right(widget, relative_id):
    return _margin_right(widget, _as_bytes(relative_id))

def margin_bottom(widget, relative_id):
    return _margin_bottom(widget, _as_bytes(relative_id))

def utf8_decode(source):
    out = UINT32()
    result = _utf8_decode(_as_bytes(source), ct.byref(out))
    return result, out.value

def utf8_to_unicode(source, bufsize):
    buffer = (UINT32 * bufsize)()
    result = _utf8_to_unicode(_as_bytes(source), buffer, bufsize)
    return result, list(buffer[:max(0, min(result, bufsize))])

def unicode_strlen(codepoints):
    values = list(codepoints)
    array_type = UINT32 * len(values)
    array = array_type(*values)
    return _unicode_strlen(array)

def FG(r, g, b):
    if _FG is None:
        raise RuntimeError("FG is not exported by this DLL build.")
    return _FG(r, g, b)

def BG(r, g, b):
    if _BG is None:
        raise RuntimeError("BG is not exported by this DLL build.")
    return _BG(r, g, b)

def get_parent_rect(widget):
    if _get_parent_rect is None:
        raise RuntimeError("get_parent_rect is not exported by this DLL build.")
    return _get_parent_rect(widget)

def find_rel_widget(widget, relative_id):
    if _find_rel_widget is None:
        raise RuntimeError("find_rel_widget is not exported by this DLL build.")
    return _find_rel_widget(widget, _as_bytes(relative_id))

def render_ui(app):
    if _render_ui is None:
        raise RuntimeError("render_ui is not exported by this DLL build.")
    _render_ui(app)

def _as_bytes(value):
    if isinstance(value, bytes):
        return value
    if isinstance(value, str):
        return value.encode("utf-8")
    raise TypeError("Expected str or bytes")

# ---------------------------------------------------------------------
# Optional shim callback support
#
# Expected shim ABI:
#   typedef void (*PyLayoutFunc)(Widget *, Rect *);
#   void set_python_layout(Widget *, PyLayoutFunc);
#
# Keep callback objects alive in this list while C can call them.
# ---------------------------------------------------------------------

PyLayoutFunc = ct.CFUNCTYPE(None, ct.POINTER(Widget), ct.POINTER(Rect))
_python_layout_callbacks = []

def make_layout_callback(python_func):
    def callback(widget_ptr, out_ptr):
        try:
            rect = python_func(widget_ptr)
            out_ptr.contents.x = rect.x
            out_ptr.contents.y = rect.y
            out_ptr.contents.w = rect.w
            out_ptr.contents.h = rect.h
        except BaseException:
            # Exceptions must not cross the C ABI boundary.
            out_ptr.contents.x = 0
            out_ptr.contents.y = 0
            out_ptr.contents.w = 0
            out_ptr.contents.h = 0
            # Production code should log this exception explicitly.
    cb = PyLayoutFunc(callback)
    _python_layout_callbacks.append(cb)
    return cb

def keep_callback(callback):
    _python_layout_callbacks.append(callback)
    return callback

# ---------------------------------------------------------------------
# Sanity checks
# ---------------------------------------------------------------------

assert ct.sizeof(Coord) == 4
assert ct.sizeof(Size) == 4
assert ct.sizeof(Rect) == 8
assert ct.sizeof(RectDelta) == 8
assert ct.sizeof(Cell) == 8
