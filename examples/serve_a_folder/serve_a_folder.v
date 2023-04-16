import malisipi.vwebinix as webinix

fn events(e &webinix.Event_t) { // Close all opened windows
    // This function gets called every time
    // there is an event
    if e.@type == webinix.event_connected {
        println("Connected.")
    } else if e.@type == webinix.event_disconnected {
        println("Disconnected.")
    } else if e.@type == webinix.event_mouse_click {
        println("Click.")
    } else if e.@type == webinix.event_navigation {
        //println("Starting navigation to: ${e.data}")
    }
}

fn switch_to_second_page(e &webinix.Event_t) {
    // This function gets called every
    // time the user clicks on "SwitchToSecondPage"
    // Switch to `/second.html` in the same opened window.
    e.window.show("second.html")
}

fn show_second_window(e &webinix.Event_t) {
    mut my_second_window := webinix.new_window()
    my_second_window.bind("Exit", exit_app)
    my_second_window.show("second.html")
}

fn exit_app(e &webinix.Event_t) { // Close all opened windows
    webinix.exit()
}

// Create new windows
mut my_window := webinix.new_window()

// Bind HTML element IDs with a C functions
my_window.bind("SwitchToSecondPage", switch_to_second_page)
my_window.bind("OpenNewWindow", show_second_window)
my_window.bind("Exit", exit_app)

my_window.bind("", events) // Bind events
my_window.show("index.html") // Show a new window

webinix.wait() // Wait until all windows get closed
