/*
    Webinix Library 2.x
    V Example
    Licensed under GNU General Public License v3.0.
    Copyright (C)2022 Hassan DRAGA <https://github.com/hassandraga>.
    Copyright (C)2022 Mehmet Ali <https://github.com/malisipi>.
*/

import malisipi.vwebinix as webinix

// Check the password function
fn check_the_password(e &webinix.Event) {
    // This function get called every time the user click on "MyButton1"
    mut js:=webinix.Javascript {timeout: 3}
    js.set_script("return document.getElementById(\"MyInput\").value;")

    // Run the JavaScript on the UI (Web Browser)
    webinix.script(e.window, &js)
    
    // Check if there is any JavaScript error
    if js.result.error {
        println("JavaScript Error:\n"+js.result.result())
        return
    }

    // Get the password
    password := js.result.result()
    println("Password: "+password)
    
    // Check the password
    if password == "123456" {
        // Correct password
        js.set_script("alert('Good. Password is correct.')")
        webinix.script(e.window, &js)
    }
    else {
        // Wrong password
        js.set_script("alert('Sorry. Wrong password.')")
        webinix.script(e.window, &js)
    }
    
    // Free data resources
    webinix.script_cleanup(&js)
}

fn close_the_application(e &webinix.Event) {
    // Close all opened windows
    webinix.exit()
}

// Create a window
mut my_window := webinix.new_window()

// UI HTML
my_html := ('
    <!DOCTYPE html>
    <html><head><title>Webinix 2 - V Example</title>
    <style>body{color: white; background: #0F2027;
    background: -webkit-linear-gradient(to right, #2C5364, #203A43, #0F2027);
    background: linear-gradient(to right, #2C5364, #203A43, #0F2027);
    text-align:center; font-size: 18px; font-family: sans-serif;}</style></head><body>
    <h1>Webinix 2 - V Example</h1><br>
    <input type=\"password\" id=\"MyInput\"><br><br>
    <button id=\"MyButton1\">Check Password</button> - <button id=\"MyButton2\">Exit</button>
    </body></html>
')

// Bind HTML elements with functions
webinix.bind(my_window, "MyButton1", check_the_password)
webinix.bind(my_window, "MyButton2", close_the_application)

// Show the window
if !webinix.show(my_window, my_html, webinix.browser_chrome) {  // Run the window on Chrome
    webinix.show(my_window, my_html, webinix.browser_any)       // If not, run on any other installed web browser
}

// Wait until all windows get closed
webinix.wait()
