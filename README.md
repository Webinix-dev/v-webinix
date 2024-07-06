<div align="center">

# Webinix V

#### [Features](#features) · [Installation](#installation) · [Usage](#usage) · [Documentation](#documentation) · [Webinix](https://github.com/webinix-dev/webinix)

[build-status]: https://img.shields.io/github/actions/workflow/status/webinix-dev/v-webinix/ci.yml?branch=main&style=for-the-badge&logo=V&labelColor=414868&logoColor=C0CAF5
[last-commit]: https://img.shields.io/github/last-commit/webinix-dev/v-webinix?style=for-the-badge&logo=github&logoColor=C0CAF5&labelColor=414868
[release-version]: https://img.shields.io/github/v/release/webinix-dev/v-webinix?style=for-the-badge&logo=webtrees&logoColor=C0CAF5&labelColor=414868&color=7664C6
[license]: https://img.shields.io/github/license/webinix-dev/v-webinix?style=for-the-badge&logo=opensourcehardware&label=License&logoColor=C0CAF5&labelColor=414868&color=8c73cc

[![][build-status]](https://github.com/webinix-dev/v-webinix/actions?query=branch%3Amain)
[![][last-commit]](https://github.com/webinix-dev/v-webinix/pulse)
[![][release-version]](https://github.com/webinix-dev/v-webinix/releases/latest)
[![][license]](https://github.com/webinix-dev/v-webinix/blob/main/LICENSE)

> Use any web browser or WebView as GUI.\
> With V in the backend and modern web technologies in the frontend.

![Screenshot](https://github.com/webinix-dev/webinix/assets/16948659/39c5b000-83eb-4779-a7ce-9769d3acf204)

</div>

## Features

- Parent library written in pure C
- Fully Independent (_No need for any third-party runtimes_)
- Lightweight ~200 Kb & Small memory footprint
- Fast binary communication protocol between Webinix and the browser (_Instead of JSON_)
- Multi-platform & Multi-Browser
- Using private profile for safety

## Installation

```sh
v install https://github.com/webinix-dev/v-webinix
```

## Usage

> [!NOTE]
> It is recommended to use GCC or Clang to compile a Webinix V program.
> TCC is currently not working due to missing header files. E.g.:
>
> ```
> v -cc gcc run .
> ```

### Example

```v
import vwebinix as ui

const html = '<!DOCTYPE html>
<html>
   <head>
      <script src="webinix.js"></script>
      <style>
         body {
            background: linear-gradient(to left, #36265a, #654da9);
            color: AliceBlue;
            font: 16px sans-serif;
            text-align: center;
            margin-top: 30px;
         }
      </style>
   </head>
   <body>
      <h1>Welcome to Webinix!</h1>
      <br>
      <input type="text" id="name" value="Neo">
      <button onclick="handleVResponse();">Call V</button>
      <br>
      <br>
      <div><samp id="greeting"></samp></div>
      <script>
         async function handleVResponse() {
            const inputName = document.getElementById("name");
            const result = await webinix.greet(inputName.value);
            document.getElementById("greeting").innerHTML = result;
         }
      </script>
   </body>
</html>'

fn greet(e &ui.Event) string {
	name := e.get_arg[string]() or { panic('expected an argument') }
	println('${name} has reached the backend!')
	return 'Hello ${name} 🐇'
}

fn main() {
   mut w := ui.new_window()
   w.bind('greet', greet)
   w.show(html)!
   ui.wait()
}
```

Find more examples in the [`examples/`](https://github.com/webinix-dev/v-webinix/tree/main/examples) directory.

## Documentation

- [Online Documentation](https://webinix.me/docs/#/v) (WIP)

> [!TIP]
> Until our online documentation is finished, you can refer to [`src/lib.v`](https://github.com/webinix-dev/v-webinix/tree/main/src/lib.v) or use V's built-in `v doc -comments vwebinix` in the terminal for the latest overview of exported functions.

- To use Webinix's debug build in your V-Webinix application, add the `-d debug` flag. E.g.:

  ```sh
  v -d webinix_log run examples/call_v_from_js.v
  ```

- Run tests locally:

  ```sh
  VJOBS=1 v -stats test tests/
  ```

## Supported Web Browsers

| Browser         | Windows         | macOS         | Linux           |
| --------------- | --------------- | ------------- | --------------- |
| Mozilla Firefox | ✔️              | ✔️            | ✔️              |
| Google Chrome   | ✔️              | ✔️            | ✔️              |
| Microsoft Edge  | ✔️              | ✔️            | ✔️              |
| Chromium        | ✔️              | ✔️            | ✔️              |
| Yandex          | ✔️              | ✔️            | ✔️              |
| Brave           | ✔️              | ✔️            | ✔️              |
| Vivaldi         | ✔️              | ✔️            | ✔️              |
| Epic            | ✔️              | ✔️            | _not available_ |
| Apple Safari    | _not available_ | _coming soon_ | _not available_ |
| Opera           | _coming soon_   | _coming soon_ | _coming soon_   |

### License

> Licensed under the MIT License.
