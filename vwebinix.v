module vwebinix

#include "@VMODROOT/webinix/mongoose.h"
#flag @VMODROOT/webinix/mongoose.c
#include "@VMODROOT/webinix/webinix.h"
#flag @VMODROOT/webinix/webinix.c
#flag windows -lws2_32
#flag windows -luser32

pub const (
	event_connected = 1
	event_multi_connection = 2
	event_unwanted_connection = 3
	event_disconnected = 4
	event_mouse_click = 5
	event_navigation = 6
	event_callback = 7
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
	browser_custom = 99
)

// C Functions & Typedefs

pub struct C.webinix_window_core_t {
pub mut:
	window_number        u32
	server_running       bool
	connected            bool
	server_handled       bool
	multi_access         bool
	server_root          bool
	server_port          u32
	url                  &char
	html                 &char
	html_cpy             &char
	icon                 &char
	icon_type            &char
	CurrentBrowser       u32
	browser_path         &char
	profile_path         &char
	connections          u32
	runtime              u32
	detect_process_close bool
	has_events           bool
}

pub type Window_t = voidptr

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

pub struct C.webinix_script_t {
pub mut:
	script  &char
	timeout u32
	result  C.webinix_javascript_result_t
}

pub struct C.webinix_cb_t {
pub mut:
	win               Window_t
	webinix_internal_id &char
	element_name      &char
	data              voidptr
	data_len          u32
	event_type        int
}

pub struct C.webinix_cmd_async_t {
pub mut:
	win Window_t
	cmd &char
}

pub struct C.webinix_custom_browser_t {
pub mut:
	app       &char
	arg       &char
	auto_link bool
}

pub struct C.webinix_runtime_t {
pub mut:
	none_  u32
	deno   u32
	nodejs u32
}

pub struct C.webinix_t {
pub mut:
	servers                u32
	connections            u32
	process                u32
	custom_browser         &C.webinix_custom_browser_t
	wait_for_socket_window bool
	html_elements          [1024]&char
	used_ports             [1024]u32
	last_window            u32
	startup_timeout        u32
	use_timeout            bool
	timeout_extra          bool
	exit_now               bool
	run_responses          [1024]&char
	run_done               [1024]bool
	run_error              [1024]bool
	run_last_id            u32
	browser                C.webinix_browser_t
	runtime                C.webinix_runtime_t
	initialized            bool
	executable_path &char
	ptr_list        [1024]voidptr
	ptr_position    u32
	ptr_size        [1024]usize
}

fn C.webinix_new_window() Window_t

fn C.webinix_bind(win Window_t, element &char, func fn (&C.webinix_event_t)) u32

fn C.webinix_show(win Window_t, content &char) bool

fn C.webinix_wait()

fn C.webinix_close(win Window_t)

fn C.webinix_exit()

fn C.webinix_script(win Window_t, script &char, timeout int, buffer &char, size_buffer int)

fn C.webinix_get_int(e &C.webinix_event_t) i64

fn C.webinix_get_string(e &C.webinix_event_t) &char

fn C.webinix_get_bool(e &C.webinix_event_t) bool

fn C.webinix_return_int(e &C.webinix_event_t, n i64)

fn C.webinix_return_string(e &C.webinix_event_t, s &char)

fn C.webinix_return_bool(e &C.webinix_event_t, b bool)

fn C.webinix_new_server(win Window_t, path &char) &char

fn C.webinix_show_browser(win Window_t, url &char, browser u32) bool

fn C.webinix_is_any_window_running() bool

fn C.webinix_is_app_running() bool

fn C.webinix_is_shown(win Window_t) bool

fn C.webinix_set_timeout(second u32)

fn C.webinix_set_icon(win Window_t, icon_s &char, type_s &char)

fn C.webinix_multi_access(win Window_t, status bool)

fn C.webinix_clean_mem(p voidptr)

fn C.webinix_bind_interface(win Window_t, element &char, func fn (u32, u32, &char, Window_t, &char, &&char)) u32

// V Interface

pub type Webui_function = fn(details &C.webinix_event_t)
pub type Event_t = C.webinix_event_t

pub fn (window Window_t) script (javascript string, timeout int, size_buffer int) string {
	response := &char(" ".repeat(size_buffer).str)
    C.webinix_script(window, &char(javascript.str), timeout, response, size_buffer)
	return unsafe { response.vstring() }
}

struct WebuiResponseData {
pub mut:
	string	string
	int	int
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
