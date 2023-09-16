import vwebinix as ui

mut w := ui.new_window()
w.show('<html><head><script src="webinix.js"></script></head>Hello</html>')!
ui.wait()
