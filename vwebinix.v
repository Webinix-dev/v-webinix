module vwebinix

#include "@VMODROOT/webinix/mongoose.h"
#flag @VMODROOT/webinix/mongoose.c
#include "@VMODROOT/webinix/webinix.h"
#flag @VMODROOT/webinix/webinix.c
#flag windows -lws2_32
#flag windows -luser32

pub const (
    browser_any         = 0
    browser_chrome      = 1
    browser_firefox     = 2
    browser_edge        = 3
    browser_safari      = 4
    browser_chromium    = 5
    browser_custom      = 99
)

struct C.webinix_window_t {}

struct C.webinix_javascript_result_t {
    pub mut:
        error   bool
        length  int
        data    &char
}

struct C.webinix_event_t {
    pub mut:
        window_id       int
        element_id      int
        element_name    &char
        window          &Window
}

struct C.webinix_script_t {
    pub mut:
        script  &char
        timeout int
        result  C.webinix_javascript_result_t
}

pub type Window     = C.webinix_window_t
pub type Event      = C.webinix_event_t
pub type Javascript = C.webinix_script_t

pub fn (javascript_result C.webinix_javascript_result_t) result () string {
    unsafe {
        return javascript_result.data.vstring()
    }
    return ""
}

pub fn (mut javascript C.webinix_script_t) set_script (script string) {
    javascript.script = script.str
}

type Webui_function=fn(details &Event)

fn C.webinix_new_window() &Window
fn C.webinix_show(window &Window, html &char, browser int) bool
fn C.webinix_wait()
fn C.webinix_exit()
fn C.webinix_is_shown(window &Window) bool
fn C.webinix_bind(window &Window, id &char, Webui_function)
fn C.webinix_script(window &Window, javascript &Javascript)
fn C.webinix_script_cleanup(javascript &Javascript)

pub fn new_window() &Window {
    return C.webinix_new_window()
}

pub fn show(window &Window, html_code string, browser int) bool {
    return C.webinix_show(window, html_code.str, browser)
}

pub fn wait() {
    C.webinix_wait()
}

pub fn exit() {
    C.webinix_exit()
}

pub fn bind(window &Window, button_id string, funct Webui_function) {
    C.webinix_bind(window, button_id.str, funct)
}

pub fn script(window &Window, javascript &Javascript) {
    C.webinix_script(window, javascript)
}

pub fn script_cleanup(javascript &Javascript) {
    C.webinix_script_cleanup(javascript)
}

pub fn is_shown(window &Window) bool {
    return C.webinix_is_shown(window)
}
