module vwebinix

#include "@VMODROOT/webinix/mongoose.h"
#flag @VMODROOT/webinix/mongoose.c
#include "@VMODROOT/webinix/webinix.h"
#flag @VMODROOT/webinix/webinix.c
#flag windows -lws2_32
#flag windows -luser32

// Consts

pub const (
	event_disconnected = 0
	event_connected = 1
	event_multi_connection = 2
	event_unwanted_connection = 3
	event_mouse_click = 4
	event_navigation = 5
	event_callback = 6
	browser_any = 0
	browser_chrome = 1
	browser_firefox = 2
	browser_edge = 3
	browser_safari = 4
	browser_chromium = 5
	browser_opera = 6
	browser_brave = 7
	browser_vivaldi = 8
	browser_epic = 9
	browser_yandex = 10
)

// Typedefs of struct

pub type Window_t = voidptr
pub type Webui_function = fn(details &C.webinix_event_t)
pub type Event_t = C.webinix_event_t

// C Functions & Typedefs

pub struct C.webinix_event_t {
pub mut:
	window_id    u32
	element_id   u32
	element_name &char
	window       Window_t
	data         voidptr
	response     voidptr
	@type        int // It's not my fault. `type` is a keyword of V.
}

pub struct C.webinix_javascript_result_t {
pub mut:
	error  bool
	length u32
	data   &char
}

fn C.webinix_new_window() Window_t

fn C.webinix_bind(win Window_t, element &char, func fn (&C.webinix_event_t)) u32

fn C.webinix_show(win Window_t, content &char) bool

fn C.webinix_show_browser(win Window_t, url &char, browser u32) bool

fn C.webinix_wait()

fn C.webinix_close(win Window_t)

fn C.webinix_exit()

fn C.webinix_is_shown(win Window_t) bool

fn C.webinix_set_timeout(second u32)

fn C.webinix_set_icon(win Window_t, icon_s &char, type_s &char)

fn C.webinix_multi_access(win Window_t, status bool)

// ? fn C.webinix_run(win Window_t, script &char)

fn C.webinix_script(win Window_t, script &char, timeout int, buffer &char, size_buffer int)

// fn C.webinix_set_runtime(win Window_t, script &char)

fn C.webinix_get_int(e &C.webinix_event_t) i64

fn C.webinix_get_string(e &C.webinix_event_t) &char

fn C.webinix_get_bool(e &C.webinix_event_t) bool

fn C.webinix_return_int(e &C.webinix_event_t, n i64)

fn C.webinix_return_string(e &C.webinix_event_t, s &char)

fn C.webinix_return_bool(e &C.webinix_event_t, b bool)

fn C.webinix_bind_interface(win Window_t, element &char, func fn (u32, u32, &char, Window_t, &char, &&char)) u32

// ? fn C.webinix_interface_set_response(ptr &char, response &char)

fn C.webinix_is_app_running() bool

fn C.webinix_interface_get_window_id(win Window_t) u32

// V Interface

pub fn (window Window_t) script (javascript string, timeout int, size_buffer int) string {
	response := &char(" ".repeat(size_buffer).str)
    C.webinix_script(window, &char(javascript.str), timeout, response, size_buffer)
	return unsafe { response.vstring() }
}

struct WebuiResponseData {
pub mut:
	string	string
	int		int
	bool	bool
}

pub fn (event &C.webinix_event_t) get () WebuiResponseData {
    str := unsafe { C.webinix_get_string(event).vstring() }
    return WebuiResponseData {
        string: str
        int: str.int()
        bool: str == "true"
    }
}

type WebuiResponseReturn = int | string | bool

pub fn (event &C.webinix_event_t) @return (response WebuiResponseReturn) {
    match response {
        string {
            C.webinix_return_string(event, &char(response.str))
    	}
        int {
            C.webinix_return_int(event, response)
    	}
        bool {
            C.webinix_return_bool(event, int(response))
    	}
    }
}

pub fn new_window() Window_t {
	return C.webinix_new_window()
}

pub fn wait() {
	C.webinix_wait()
}

pub fn (window Window_t) show (html_code string) bool {
	return C.webinix_show(window, html_code.str)
}

pub fn (window Window_t) close () {
	C.webinix_close(window)
}

pub fn (window Window_t) open (html_code string, browser int) bool {
	return C.webinix_show_browser(window, html_code.str, browser)
}

pub fn exit() {
	C.webinix_exit()
}

pub fn (window Window_t) bind (button_id string, funct Webui_function) {
	C.webinix_bind(window, button_id.str, funct)
}
