module vwebinix

#include "@VMODROOT/webinix/include/webinix.h"

$if webinix_log ? {
	#flag -L@VMODROOT/webinix/debug -lwebinix-2-static
} $else {
	#flag -L@VMODROOT/webinix -lwebinix-2-static
}

#flag linux -lpthread -lm
#flag darwin -lpthread -lm
#flag windows -lwebinix-2 -lws2_32

[typedef]
struct C.webinix_event_t {
pub:
	window       Window    // The window object number
	event_type   EventType // Event type
	element      &char     // HTML element ID
	event_number usize     // Internal Webinix
	bind_id      usize     // Bind ID
}

// -- Definitions ---------------------
fn C.webinix_new_window() Window
fn C.webinix_new_window_id(win_id Window)
fn C.webinix_get_new_window_id() Window
fn C.webinix_bind(win Window, elem &char, func fn (&C.webinix_event_t)) Function
fn C.webinix_show(win Window, content &char) bool
fn C.webinix_show_browser(win Window, content &char, browser Browser) bool
fn C.webinix_set_kiosk(win Window, kiosk bool)
fn C.webinix_wait()
fn C.webinix_close(win Window)
fn C.webinix_destroy(win Window)
fn C.webinix_exit()
fn C.webinix_set_root_folder(win Window, path &char)
fn C.webinix_set_default_root_folder(path &char)
fn C.webinix_set_file_handler(win Window, handler fn (file_name &char, length int)) // not wrapped
fn C.webinix_is_shown(win Window) bool
fn C.webinix_set_timeout(second usize)
fn C.webinix_set_icon(win Window, icon &char, icon_type &char)
fn C.webinix_encode(str &char) &char
fn C.webinix_decode(str &char) &char
fn C.webinix_free(ptr voidptr) // not wrapped
fn C.webinix_malloc(size usize) voidptr // not wrapped
fn C.webinix_send_raw(size Window, func &char, raw voidptr, size usize) // not wrapped
fn C.webinix_set_hide(win Window, status bool)
fn C.webinix_set_size(win Window, width usize, height usize)
fn C.webinix_set_position(win Window, x usize, y usize)
fn C.webinix_set_profile(win Window, name &char, path &char)
fn C.webinix_get_url(win Window) &char
fn C.webinix_navigate(win Window, url &char)
fn C.webinix_clean()

// -- JavaScript ----------------------
fn C.webinix_run(win Window, script &char)
fn C.webinix_script(win Window, script &char, timeout usize, buffer &char, buffer_length usize) bool
fn C.webinix_set_runtime(win Window, runtime Runtime)
fn C.webinix_get_int(e &C.webinix_event_t) i64
fn C.webinix_get_int_at(e &C.webinix_event_t, idx usize) i64
fn C.webinix_get_string(e &C.webinix_event_t) &char
fn C.webinix_get_string_at(e &C.webinix_event_t, idx usize) &char
fn C.webinix_get_bool(e &C.webinix_event_t) bool
fn C.webinix_get_bool_at(e &C.webinix_event_t, idx usize) bool
fn C.webinix_get_size(e &C.webinix_event_t) usize
fn C.webinix_get_size_at(e &C.webinix_event_t, idx usize) usize
fn C.webinix_return_int(e &C.webinix_event_t, n i64)
fn C.webinix_return_string(e &C.webinix_event_t, s &char)
fn C.webinix_return_bool(e &C.webinix_event_t, b bool)

// -- Interface ----------------------- // not wrapped
fn C.webinix_interface_bind(win Window, element &char, func fn (win Window, event_type EventType, element &char, event_num usize, bind_id usize)) Function
fn C.webinix_interface_set_response(win Window, event_num usize, resp &char)
fn C.webinix_interface_is_app_running() bool
fn C.webinix_interface_get_window_id(win Window) Window
