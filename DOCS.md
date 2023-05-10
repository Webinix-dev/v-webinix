# Webinix v2.2.0 V APIs

* [Download and Install](#download-and-install)
* Examples
* Window
    * New Window
    * Show Window
    * Window status
* Binding & Events
    * Bind
    * Events
* Application
    * Wait
    * Exit
    * Close
    * Startup Timeput
    * Multi Access

## Download and Install

Install the VWebinix package from VPM (~ 800 Kb).

```sh
v install malisipi.mui
```

To see the VWebinix source code, please visit [VWebinix](https://github.com/malisipi/vwebinix) in our GitHub repository.

## Examples

A minimal V example

```v
import malisipi.vwebinix as webinix

mut my_window := webinix.new_window()
my_window.show("<html>Hello</html>")
webinix.wait()
```

Please visit [V Examples](https://github.com/malisipi/vwebinix/tree/main/examples) in our GitHub repository for more complete examples.