import malisipi.vwebinix as webinix
import encoding.base64
import os

fn close(e &webinix.Event) webinix.Response {
	webinix.exit()
	return 0
}

fn open(e &webinix.Event) webinix.Response {
	file := os.input("input file:")
	if file != "" {
		file_content :=os.read_file(file) or {""}

		encoded_file := base64.encode_str(file)
		encoded_file_content := base64.encode_str(file_content)
		e.window.run("SetFile`${encoded_file}`")
		e.window.run("addText`${encoded_file_content}`")
	}
	return 0
}

fn save(e &webinix.Event) webinix.Response {
	content := e.data.string
	println(content)
	return 0
}

main_window := webinix.new_window()

main_window.bind("Open", open)
	.bind("Save", save)
	.bind("Close", close)
	.show("ui/MainWindow.html")

webinix.wait()