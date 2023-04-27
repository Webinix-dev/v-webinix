// v install https://github.com/malisipi/vwebinix
import malisipi.vwebinix as webinix

mut my_window := webinix.new_window()
my_window.show("<html>Hello</html>")
webinix.wait()
