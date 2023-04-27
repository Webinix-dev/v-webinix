// v install https://github.com/malisipi/vwebinix
import vwebinix as webinix

fn my_function_count(e &webinix.Event) {
  count := e.window.script("return count;", 0, 48)
  e.window.script("SetCount(${count.int() + 1});", 0, 0)
}

fn my_function_exit(e &webinix.Event) { // Close all opened windows
    webinix.exit()
}

mut my_window := webinix.new_window() // Create a window

// UI HTML
my_html := ('
<html>
  <head>
    <title>Call JavaScript from V Example</title>
    <style>
      body {
        color: white;
        background: #0F2027;
        text-align: center;
        font-size: 16px;
        font-family: sans-serif;
      }
    </style>
  </head>
  <body>
    <h2>Webinix - Call JavaScript from V Example</h2>
    <br>
    <h1 id="MyElementID">Count is ?</h1>
    <br>
    <br>
    <button id="MyButton1">Count</button>
    <br>
    <br>
    <button id="MyButton2">Exit</button>
    <script>
      var count = 0;
      function GetCount() {
        return count;
      }
      function SetCount(number) {
        const MyElement = document.getElementById("MyElementID");
        MyElement.innerHTML = "Count is " + number;
        count = number;
      }
    </script>
  </body>
</html>')

// Bind HTML elements with functions
my_window.bind("MyButton1", my_function_count)
my_window.bind("MyButton2", my_function_exit)

// Show the window
if !my_window.show(my_html) { // Run the window
    panic("The browser(s) was failed") // If not, print a error info
}

// Wait until all windows get closed
webinix.wait()
