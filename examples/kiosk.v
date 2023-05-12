import vwebinix as webinix

mut my_window := webinix.new_window()
my_window.set_kiosk(true)
my_window.show("<html>A kiosk example</html>")
webinix.wait()
