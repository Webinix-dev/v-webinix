/*
  V-Webinix 2.2.0
  https://github.com/malisipi/vwebinix
  Copyright (c) 2023 Mehmet Ali.
  Licensed under GNU General Public License v2.0.
  All rights reserved.
*/

module vwebinix

// Webinix Core

#include "@VMODROOT/webinix/mongoose.h"
#include "@VMODROOT/webinix/webinix.h"
#include "@VMODROOT/webinix/webinix_core.h"
#flag @VMODROOT/webinix/mongoose.c
#flag @VMODROOT/webinix/webinix.c
#flag windows -Dstrtoll=_strtoi64 -Dstrtoull=_strtoui64 -lws2_32 -lAdvapi32 -luser32
$if tinyc {
	#flag windows -DWEBUI_NO_TLHELPER32
}
// Debug
$if webinix_log? {
	#flag -DWEBUI_LOG
}

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
	runtime_none = 0
	runtime_deno = 1
	runtime_nodejs = 2
)

// Typedefs of struct

pub type Window = voidptr
pub struct C.webinix_event_t {
	pub mut:
		window		Window // Pointer to the window object
		@type		u64 // Event type :)
		element		&char // HTML element ID
		data		&char // JavaScript data
		response	&char // Callback response
}
pub type Event = C.webinix_event_t
pub type Function = fn(e &Event)

// C Functions

fn C.webinix_new_window() Window
fn C.webinix_bind(win Window, element &char, func fn (&Event)) u64
fn C.webinix_show(win Window, content &char) bool
fn C.webinix_show_browser(win Window, content &char, browser u64) bool
fn C.webinix_wait()
fn C.webinix_close(win Window)
fn C.webinix_exit()
fn C.webinix_is_shown(win Window) bool
fn C.webinix_set_timeout(second u64)
fn C.webinix_set_icon(win Window, icon &char, icon_type &char)
fn C.webinix_multi_access(win Window, status bool)
fn C.webinix_run(win Window, script &char)
fn C.webinix_script(win Window, script &char, timeout u64, buffer &char, size_buffer u64)
fn C.webinix_set_runtime(win Window, runtime u64)
fn C.webinix_get_int(e &Event) i64
fn C.webinix_get_string(e &Event) &char
fn C.webinix_get_bool(e &Event) bool
fn C.webinix_return_int(e &Event, n i64)
fn C.webinix_return_string(e &Event, s &char)
fn C.webinix_return_bool(e &Event, b bool)
fn C.webinix_interface_is_app_running() bool
fn C.webinix_interface_get_window_id(win Window) u64

// V Interface

pub fn (window Window) script (javascript string, timeout u64, size_buffer int) string {
	response := &char(" ".repeat(size_buffer).str)
    C.webinix_script(window, &char(javascript.str), timeout, response, size_buffer)
	return unsafe { response.vstring() }
}

// Get
struct WebuiResponseData {
pub mut:
	string	string
	int		int
	bool	bool
}
pub fn (e &Event) get () WebuiResponseData {
    str := unsafe { C.webinix_get_string(e).vstring() }
    return WebuiResponseData {
        string: str
        int: str.int()
        bool: str == "true"
    }
}

// Return
type WebuiResponseReturn = int | string | bool
pub fn (e &Event) @return (response WebuiResponseReturn) {
    match response {
        string {
            C.webinix_return_string(e, &char(response.str))
    	}
        int {
            C.webinix_return_int(e, i64(response))
    	}
        bool {
            C.webinix_return_bool(e, int(response))
    	}
    }
}

// Create a new webinix window object.
pub fn new_window() Window {
	return C.webinix_new_window()
}

// Wait until all opened windows get closed.
pub fn wait() {
	C.webinix_wait()
}

// Show a window using a embedded HTML, or a file. If the window is already opened then it will be refreshed.
pub fn (window Window) show (content string) bool {
	return C.webinix_show(window, content.str)
}

// Close a specific window.
pub fn (window Window) close () {
	C.webinix_close(window)
}

// Close all opened windows. webinix_wait() will break.
pub fn exit() {
	C.webinix_exit()
}

// Bind a specific html element click event with a function. Empty element means all events.
pub fn (window Window) bind (element string, func Function) {
	C.webinix_bind(window, element.str, func)
}

// Set the maximum time in seconds to wait for browser to start
pub fn set_timeout(timeout u64){
	C.webinix_set_timeout(timeout)
}
