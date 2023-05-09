// v install https://github.com/malisipi/vwebinix
import malisipi.vwebinix as webinix

const (
    my_window = 1
    my_second_window = 2
)

fn events(e &webinix.Event) webinix.Response { // Close all opened windows
    // This function gets called every time
    // there is an event
    if e.event_type == webinix.event_connected {
        println("Connected.")
    } else if e.event_type == webinix.event_disconnected {
        println("Disconnected.")
    } else if e.event_type == webinix.event_mouse_click {
        println("Click.")
    } else if e.event_type == webinix.event_navigation {
        println("Starting navigation to: ${e.data}")
    }
    return 0
}

fn switch_to_second_page(e &webinix.Event) webinix.Response {
    // This function gets called every
    // time the user clicks on "SwitchToSecondPage"
    // Switch to `/second.html` in the same opened window.
    e.window.show("second.html")
    return 0
}

fn show_second_window(e &webinix.Event) webinix.Response {
    webinix.get_window(my_second_window)
        .show("second.html")
    return 0
}

fn exit_app(e &webinix.Event) webinix.Response { // Close all opened windows
    webinix.exit()
    return 0
}

// Create new windows
webinix.new_window_by_id(my_window)
    // Bind HTML element IDs with a C functions
    .bind("SwitchToSecondPage", switch_to_second_page)
    .bind("OpenNewWindow", show_second_window)
    .bind("Exit", exit_app)
    .bind("", events) // Bind events
    .show("index.html") // Show a new window

webinix.new_window_by_id(my_second_window)
    .bind("Exit", exit_app)

webinix.wait() // Wait until all windows get closed
