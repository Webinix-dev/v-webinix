/*
  Webinix Library 2.2.0
  http://_webinix_core.me
  https://github.com/alifcommunity/webinix
  Copyright (c) 2020-2023 Hassan Draga.
  Licensed under GNU General Public License v2.0.
  All rights reserved.
  Canada.
*/

// -- Third-party ---------------------
#include "mongoose.h"

// -- Webinix ---------------------------
#include "webinix_core.h"

// -- Heap ----------------------------
static _webinix_core_t _webinix_core;

// -- Webinix JS-Bridge ---------
// This is a uncompressed version to make the debugging
// more easy in the browser using the builtin dev-tools
#ifdef WEBUI_LOG
    #define WEBUI_JS_LOG "true"
#else
    #define WEBUI_JS_LOG "false"
#endif
static const char* webinix_javascript_bridge = 
"var _webinix_log = " WEBUI_JS_LOG "; \n"
"var _webinix_ws; \n"
"var _webinix_ws_status = false; \n"
"var _webinix_ws_status_once = false; \n"
"var _webinix_close_reason = 0; \n"
"var _webinix_close_value; \n"
"var _webinix_has_events = false; \n"
"var _webinix_fn_id = new Uint8Array(1); \n"
"var _webinix_fn_promise_resolve; \n"
"const WEBUI_HEADER_SIGNATURE = 221; \n"
"const WEBUI_HEADER_JS = 254; \n"
"const WEBUI_HEADER_JS_QUICK = 253; \n"
"const WEBUI_HEADER_CLICK = 252; \n"
"const WEBUI_HEADER_SWITCH = 251; \n"
"const WEBUI_HEADER_CLOSE = 250; \n"
"const WEBUI_HEADER_CALL_FUNC = 249; \n"
"function _webinix_close(reason = 0, value = 0) { \n"
"    _webinix_send_event_navigation(value); \n"
"    _webinix_ws_status = false; \n"
"    _webinix_close_reason = reason; \n"
"    _webinix_close_value = value; \n"
"    _webinix_ws.close(); \n"
"} \n"
"function _webinix_freeze_ui() { \n"
"    document.body.style.filter = 'contrast(1%)'; \n"
"} \n"
"function _webinix_start() { \n"
"    if('WebSocket' in window) { \n"
"        if(_webinix_bind_list.includes(_webinix_win_num + '/')) _webinix_has_events = true; \n"
"        _webinix_ws = new WebSocket('ws://localhost:' + _webinix_port + '/_webinix_ws_connect'); \n"
"        _webinix_ws.binaryType = 'arraybuffer'; \n"
"        _webinix_ws.onopen = function () { \n"
"            _webinix_ws.binaryType = 'arraybuffer'; \n"
"            _webinix_ws_status = true; \n"
"            _webinix_ws_status_once = true; \n"
"            _webinix_fn_id[0] = 1; \n"
"            if(_webinix_log) \n"
"                console.log('Webinix -> Connected'); \n"
"            _webinix_clicks_listener(); \n"
"        }; \n"
"        _webinix_ws.onerror = function () { \n"
"            if(_webinix_log) \n"
"                console.log('Webinix -> Connection Failed'); \n"
"            _webinix_freeze_ui(); \n"
"        }; \n"
"        _webinix_ws.onclose = function (evt) { \n"
"            _webinix_ws_status = false; \n"
"            if(_webinix_close_reason === WEBUI_HEADER_SWITCH) { \n"
"                if(_webinix_log) \n"
"                    console.log('Webinix -> Connection lost -> Navigation to [' + _webinix_close_value + ']'); \n"
"                window.location.replace(_webinix_close_value); \n"
"            } else { \n"
"                if(_webinix_log) \n"
"                    console.log('Webinix -> Connection lost (' + evt.code + ')'); \n"
"                if(!_webinix_log && evt.code != 1005) window.close(); \n"
"                else _webinix_freeze_ui(); \n"
"            } \n"
"        }; \n"
"        _webinix_ws.onmessage = function (evt) { \n"
"                const buffer8 = new Uint8Array(evt.data); \n"
"                if(buffer8.length < 4) return; \n"
"                if(buffer8[0] !== WEBUI_HEADER_SIGNATURE) \n"
"                    return; \n"
"                var len = buffer8.length - 3; \n"
"                if(buffer8[buffer8.length - 1] === 0) \n"
"                   len--; // Null byte (0x00) can break eval() \n"
"                data8 = new Uint8Array(len); \n"
"                for (i = 0; i < len; i++) data8[i] = buffer8[i + 3]; \n"
"                var data8utf8 = new TextDecoder('utf-8').decode(data8); \n"
"                // Process Command \n"
"                if(buffer8[1] === WEBUI_HEADER_CALL_FUNC) { \n"
"                    if(_webinix_log) \n"
"                        console.log('Webinix -> Func Reponse [' + data8utf8 + ']'); \n"
"                    if (_webinix_fn_promise_resolve) { \n"
"                        if(_webinix_log) \n"
"                            console.log('Webinix -> Resolving reponse...'); \n"
"                        _webinix_fn_promise_resolve(data8utf8); \n"
"                        _webinix_fn_promise_resolve = null; \n"
"                    } \n"
"                } else if(buffer8[1] === WEBUI_HEADER_SWITCH) { \n"
"                    _webinix_close(WEBUI_HEADER_SWITCH, data8utf8); \n"
"                } else if(buffer8[1] === WEBUI_HEADER_CLOSE) { \n"
"                    _webinix_close(WEBUI_HEADER_CLOSE); \n"
"                } else if(buffer8[1] === WEBUI_HEADER_JS_QUICK || buffer8[1] === WEBUI_HEADER_JS) { \n"
"                    data8utf8 = data8utf8.replace(/(?:\\r\\n|\\r|\\n)/g, \"\\\\n\"); \n"
"                    if(_webinix_log) \n"
"                        console.log('Webinix -> JS [' + data8utf8 + ']'); \n"
"                    var FunReturn = 'undefined'; \n"
"                    var FunError = false; \n"
"                    try { FunReturn = eval('(() => {' + data8utf8 + '})()'); } catch (e) { FunError = true; FunReturn = e.message } \n"
"                    if(buffer8[1] === WEBUI_HEADER_JS_QUICK) return; \n"
"                    if(typeof FunReturn === 'undefined' || FunReturn === undefined) FunReturn = 'undefined'; \n"
"                    if(_webinix_log && !FunError) console.log('Webinix -> JS -> Return [' + FunReturn + ']'); \n"
"                    if(_webinix_log && FunError) console.log('Webinix -> JS -> Error [' + FunReturn + ']'); \n"
"                    var FunReturn8 = new TextEncoder('utf-8').encode(FunReturn); \n"
"                    var Return8 = new Uint8Array(4 + FunReturn8.length); \n"
"                    Return8[0] = WEBUI_HEADER_SIGNATURE; \n"
"                    Return8[1] = WEBUI_HEADER_JS; \n"
"                    Return8[2] = buffer8[2]; \n"
"                    if(FunError) Return8[3] = 0; \n"
"                    else Return8[3] = 1; \n"
"                    var p = -1; \n"
"                    for (i = 4; i < FunReturn8.length + 4; i++) Return8[i] = FunReturn8[++p]; \n"
"                    if(_webinix_ws_status) _webinix_ws.send(Return8.buffer); \n"
"                } \n"
"        }; \n"
"    } else { \n"
"        alert('Sorry. WebSocket not supported by your Browser.'); \n"
"        if(!_webinix_log) window.close(); \n"
"    } \n"
"} \n"
"function _webinix_clicks_listener() { \n"
"    Object.keys(window).forEach(key=>{ \n"
"        if(/^on(click)/.test(key)) { \n"
"            window.addEventListener(key.slice(2),event=>{ \n"
"                if(event.target.id !== '') { \n"
"                    if(_webinix_has_events || _webinix_bind_list.includes(_webinix_win_num + '/' + event.target.id)) \n"
"                        _webinix_send_click(event.target.id); \n"
"                } \n"
"            }); \n"
"        } \n"
"    }); \n"
"} \n"
"function _webinix_send_click(elem) { \n"
"    if(_webinix_ws_status && elem !== '') { \n"
"        var elem8 = new TextEncoder('utf-8').encode(elem); \n"
"        var packet = new Uint8Array(3 + elem8.length); \n"
"        packet[0] = WEBUI_HEADER_SIGNATURE; \n"
"        packet[1] = WEBUI_HEADER_CLICK; \n"
"        packet[2] = 0; \n"
"        var p = -1; \n"
"        for (i = 3; i < elem8.length + 3; i++) \n"
"            packet[i] = elem8[++p]; \n"
"        _webinix_ws.send(packet.buffer); \n"
"        if(_webinix_log) \n"
"            console.log('Webinix -> Click [' + elem + ']'); \n"
"    } \n"
"} \n"
"function _webinix_send_event_navigation(url) { \n"
"    if(_webinix_ws_status && url !== '') { \n"
"        var url8 = new TextEncoder('utf-8').encode(url); \n"
"        var packet = new Uint8Array(3 + url8.length); \n"
"        packet[0] = WEBUI_HEADER_SIGNATURE; \n"
"        packet[1] = WEBUI_HEADER_SWITCH; \n"
"        packet[2] = 0; \n"
"        var p = -1; \n"
"        for (i = 3; i < url8.length + 3; i++) \n"
"            packet[i] = url8[++p]; \n"
"        _webinix_ws.send(packet.buffer); \n"
"        if(_webinix_log) \n"
"            console.log('Webinix -> Navigation [' + url + ']'); \n"
"    } \n"
"} \n"
"function _webinix_is_external_link(url) { \n"
"    const currentUrl = new URL(window.location.href); \n"
"    const targetUrl = new URL(url, window.location.href); \n"
"    currentUrl.hash = ''; \n"
"    targetUrl.hash = ''; \n"
"    if (url.startsWith('#') || url === currentUrl.href + '#' || currentUrl.href === targetUrl.href) { \n"
"        return false; \n"
"    } \n"
"        return true; \n"
"} \n"
"async function _webinix_fn_promise(fn, value) { \n"
"    if(_webinix_log) \n"
"        console.log('Webinix -> Func [' + fn + '](' + value + ')'); \n"
"    var fn8 = new TextEncoder('utf-8').encode(fn); \n"
"    var value8 = new TextEncoder('utf-8').encode(value); \n"
"    var packet = new Uint8Array(3 + fn8.length + 1 + value8.length); \n"
"    packet[0] = WEBUI_HEADER_SIGNATURE; \n"
"    packet[1] = WEBUI_HEADER_CALL_FUNC; \n"
"    packet[2] = _webinix_fn_id[0]++; \n"
"    var p = 3; \n"
"    for (var i = 0; i < fn8.length; i++) \n"
"        { packet[p] = fn8[i]; p++; } \n"
"    packet[p] = 0; \n"
"    p++; \n"
"    if(value8.length > 0) { \n"
"        for (var i = 0; i < value8.length; i++) \n"
"            { packet[p] = value8[i]; p++; } \n"
"    } else { packet[p] = 0; } \n"
"    return new Promise((resolve) => {_webinix_fn_promise_resolve = resolve; _webinix_ws.send(packet.buffer); }); \n"
"} \n"
" // -- APIs -------------------------- \n"
"function webinix_fn(fn, value) { \n"
"    if(!fn || !_webinix_ws_status) \n"
"        return Promise.resolve(); \n"
"    if(typeof value == 'undefined') \n"
"        var value = ''; \n"
"    if(!_webinix_has_events && !_webinix_bind_list.includes(_webinix_win_num + '/' + fn)) \n"
"        return Promise.resolve(); \n"
"    return _webinix_fn_promise(fn, value); \n"
"} \n"
"function webinix_log(status) { \n"
"    if(status) { \n"
"        console.log('Webinix -> Log Enabled.'); \n"
"        _webinix_log = true; \n"
"    } else { \n"
"        console.log('Webinix -> Log Disabled.'); \n"
"        _webinix_log = false; \n"
"    } \n"
"} \n"
" // -- DOM --------------------------- \n"
"document.addEventListener('keydown', function (e) { \n"
"    // Disable F5 \n"
"    if(e.keyCode === 116) { \n"
"        e.preventDefault(); \n"
"        e.returnValue = false; \n"
"        e.keyCode = 0; \n"
"        return false; \n"
"    } \n"
"}); \n"
"window.onbeforeunload = function () { \n"
"   _webinix_ws.close(); \n"
"}; \n"
"setTimeout(function () { \n"
"    if(!_webinix_ws_status_once) { \n"
"        _webinix_freeze_ui(); \n"
"        alert('Webinix failed to connect to the background application. Please try again.'); \n"
"        if(!_webinix_log) window.close(); \n"
"    } \n"
"}, 1500); \n"
"window.addEventListener('unload', unload_handler, false); \n"
"function unload_handler() { \n"
"    // Unload for 'back' & 'forward' navigation \n"
"    window.removeEventListener('unload', unload_handler, false); \n"
"} \n"
"// Links \n"
"document.addEventListener('click', e => { \n"
"    const attribute = e.target.closest('a'); \n"
"    if(attribute) { \n"
"        const link = attribute.href; \n"
"        if(_webinix_is_external_link(link)) { \n"
"            e.preventDefault(); \n"
"            _webinix_close(WEBUI_HEADER_SWITCH, link); \n"
"        } \n"
"    } \n"
"}); \n"
"if(typeof navigation !== 'undefined') { \n"
"    navigation.addEventListener('navigate', (event) => { \n"
"        const url = new URL(event.destination.url); \n"
"        _webinix_send_event_navigation(url); \n"
"    }); \n"
"} \n"
"document.body.addEventListener('contextmenu', function(event){ event.preventDefault(); }); \n"
"var inputs = document.getElementsByTagName('input'); \n"
"for(var i = 0; i < inputs.length; i++){ inputs[i].addEventListener('contextmenu', function(event){ event.stopPropagation(); });} \n"
"// Load \n"
"window.addEventListener('load', _webinix_start()); \n";

// -- Heap ----------------------------
static const char* webinix_html_served = "<html><head><title>Access Denied</title><style>body{margin:0;background-repeat:no-repeat;background-attachment:fixed;background-color:#FF3CAC;background-image:linear-gradient(225deg,#FF3CAC 0%,#784BA0 45%,#2B86C5 100%);font-family:sans-serif;margin:20px;color:#fff}a{color:#fff}</style></head><body><h2>&#9888; Access Denied</h2><p>You can't access this content<br>because it's already processed.<br><br>The current security policy denies<br>multiple requests.</p><br><a href=\"https://www.webinix.me\"><small>Webinix v" WEBUI_VERSION "<small></a></body></html>";
static const char* webinix_html_res_not_available = "<html><head><title>Resource Not Available</title><style>body{margin:0;background-repeat:no-repeat;background-attachment:fixed;background-color:#FF3CAC;background-image:linear-gradient(225deg,#FF3CAC 0%,#784BA0 45%,#2B86C5 100%);font-family:sans-serif;margin:20px;color:#fff}a{color:#fff}</style></head><body><h2>&#9888; Resource Not Available</h2><p>The requested resource is not available.</p><br><a href=\"https://www.webinix.me\"><small>Webinix v" WEBUI_VERSION "<small></a></body></html>";
static const char* webinix_deno_not_found = "<html><head><title>Deno Not Found</title><style>body{margin:0;background-repeat:no-repeat;background-attachment:fixed;background-color:#FF3CAC;background-image:linear-gradient(225deg,#FF3CAC 0%,#784BA0 45%,#2B86C5 100%);font-family:sans-serif;margin:20px;color:#fff}a{color:#fff}</style></head><body><h2>&#9888; Deno Not Found</h2><p>Deno is not found on this system.<br>Please download it from <a href=\"https://github.com/denoland/deno/releases\">https://github.com/denoland/deno/releases</a></p><br><a href=\"https://www.webinix.me\"><small>Webinix v" WEBUI_VERSION "<small></a></body></html>";
static const char* webinix_nodejs_not_found = "<html><head><title>Node.js Not Found</title><style>body{margin:0;background-repeat:no-repeat;background-attachment:fixed;background-color:#FF3CAC;background-image:linear-gradient(225deg,#FF3CAC 0%,#784BA0 45%,#2B86C5 100%);font-family:sans-serif;margin:20px;color:#fff}a{color:#fff}</style></head><body><h2>&#9888; Node.js Not Found</h2><p>Node.js is not found on this system.<br>Please download it from <a href=\"https://nodejs.org/en/download/\">https://nodejs.org/en/download/</a></p><br><a href=\"https://www.webinix.me\"><small>Webinix v" WEBUI_VERSION "<small></a></body></html>";
static const char* webinix_def_icon = "<svg height=\"24\" width=\"24\" xmlns=\"http://www.w3.org/2000/svg\"><path d=\"M3 2c-1.105 0-2 .9-2 2v16c0 1.1.895 2 2 2h18c1.105 0 2-.9 2-2V4c0-1.1-.895-2-2-2H3z\" fill=\"#2c3e50\"/><path d=\"M3 21c-1.105 0-2-.9-2-2V7h22v12c0 1.1-.895 2-2 2H3z\" fill=\"#34495e\"/><path d=\"M4 4.5v1.1L6 8l-2 2.3v1.1L7 8 4 4.5z\" fill=\"#ecf0f1\"/><path d=\"M3 2a2 2 0 0 0-2 2v12h22V4a2 2 0 0 0-2-2H3z\" fill=\"#34495e\"/><path d=\"M4 5.125V6.25L7 8 4 9.75v1.125L9 8 4 5.125zM9 10v1h5v-1H9z\" fill=\"#ecf0f1\"/></svg>";
static const char* webinix_def_icon_type = "Content-Type: image/svg+xml\r\n";
static const char* webinix_js_empty = "ERR_WEBUI_NO_SCRIPT_FOUND";
static const char* webinix_js_timeout = "ERR_WEBUI_TIMEOUT";
static const char* const webinix_empty_string = ""; // In case the compiler optimization is disabled

// -- Functions -----------------------
bool webinix_run(void* window, const char* script) {

    #ifdef WEBUI_LOG
        printf("[User] webinix_run([%s])... \n", script);
    #endif

    size_t js_len = strlen(script);
    
    if(js_len < 1)
        return false;
    
    // Dereference
    _webinix_window_t* win = (_webinix_window_t*)window;

    // Initializing pipe
    unsigned char run_id = _webinix_get_run_id();
    _webinix_core.run_done[run_id] = false;
    _webinix_core.run_error[run_id] = false;
    if((void*)_webinix_core.run_responses[run_id] != NULL)
        _webinix_free_mem((void*)_webinix_core.run_responses[run_id]);
    
    // Prepare the packet
    size_t packet_len = 3 + js_len; // [header][js]
    char* packet = (char*) _webinix_malloc(packet_len);
    packet[0] = WEBUI_HEADER_SIGNATURE; // Signature
    packet[1] = WEBUI_HEADER_JS_QUICK;  // Type
    packet[2] = run_id;                 // ID
    for(unsigned int i = 0; i < js_len; i++) // Data
        packet[i + 3] = script[i];
    
    // Send packets
    _webinix_window_send(win, packet, packet_len);
    _webinix_free_mem((void*)packet);

    return true;
}

bool webinix_script(void* window, const char* script, unsigned int timeout_second, char* buffer, size_t buffer_length) {

    // Dereference
    _webinix_window_t* win = (_webinix_window_t*)window;

    #ifdef WEBUI_LOG
        printf("[User] webinix_script()... \n");
        printf("[User] webinix_script() -> Script [%s] \n", script);
        printf("[User] webinix_script() -> Response Buffer @ 0x%p \n", buffer);
        printf("[User] webinix_script() -> Response Buffer Size %zu bytes \n", buffer_length);
    #endif

    _webinix_init();

    // Initializing response buffer
    if(buffer_length > 0)
        memset(buffer, 0, buffer_length);    

    size_t js_len = strlen(script);

    if(js_len < 1) {

        if(buffer != NULL && buffer_length > 1)
            snprintf(buffer, buffer_length, "%s", webinix_js_empty);
        return false;
    }

    // Initializing pipe
    unsigned char run_id = _webinix_get_run_id();
    _webinix_core.run_done[run_id] = false;
    _webinix_core.run_error[run_id] = false;
    if((void*)_webinix_core.run_responses[run_id] != NULL)
        _webinix_free_mem((void*)_webinix_core.run_responses[run_id]);

    // Prepare the packet
    size_t packet_len = 3 + js_len;             // [header][js]
    char* packet = (char*) _webinix_malloc(packet_len);
    packet[0] = WEBUI_HEADER_SIGNATURE;         // Signature
    packet[1] = WEBUI_HEADER_JS;                // Type
    packet[2] = run_id;                         // ID
    for(unsigned int i = 0; i < js_len; i++)    // Data
        packet[i + 3] = script[i];
    
    // Send packets
    _webinix_window_send(win, packet, packet_len);
    _webinix_free_mem((void*)packet);

    // Wait for UI response
    if(timeout_second < 1 || timeout_second > 86400) {

        // Wait forever
        for(;;) {

            if(_webinix_core.run_done[run_id])
                break;
            
            _webinix_sleep(1);
        }
    }
    else {

        // Using timeout
        for(unsigned int n = 0; n <= (timeout_second * 1000); n++) {

            if(_webinix_core.run_done[run_id])
                break;
            
            _webinix_sleep(1);
        }
    }

    if(_webinix_core.run_responses[run_id] != NULL) {

        #ifdef WEBUI_LOG
            printf("[User] webinix_script -> Response found [%s] \n", _webinix_core.run_responses[run_id]);
        #endif

        // Response found
        if(buffer != NULL && buffer_length > 1) {

            // Copy response to the user's response buffer
            size_t response_len = strlen(_webinix_core.run_responses[run_id]) + 1;
            size_t bytes_to_cpy = (response_len <= buffer_length ? response_len : buffer_length);
            memcpy(buffer, _webinix_core.run_responses[run_id], bytes_to_cpy);
        }

        _webinix_free_mem((void*)_webinix_core.run_responses[run_id]);

        return _webinix_core.run_error[run_id];
    }

    return false;
}

void* webinix_new_window(void) {

    #ifdef WEBUI_LOG
        printf("[User] webinix_new_window()... \n");
    #endif

    _webinix_init();

    _webinix_window_t* win = (_webinix_window_t*) _webinix_malloc(sizeof(_webinix_window_t));

    // Initialisation
    win->window_number = _webinix_get_new_window_number();
    win->browser_path = (char*) _webinix_malloc(WEBUI_MAX_PATH);
    win->profile_path = (char*) _webinix_malloc(WEBUI_MAX_PATH);
    win->server_root_path = (char*) _webinix_malloc(WEBUI_MAX_PATH);
    sprintf(win->server_root_path, "%s", WEBUI_DEFAULT_PATH);
    
    #ifdef WEBUI_LOG
        printf("[User] webinix_new_window() -> New window @ 0x%p\n", win);
    #endif

    return (void*)win;
}

void webinix_set_kiosk(void* window, bool status) {

    // Dereference
    _webinix_window_t* win = (_webinix_window_t*)window;

    win->kiosk_mode = status;
}

void webinix_close(void* window) {

    // Dereference
    _webinix_window_t* win = (_webinix_window_t*)window;

    #ifdef WEBUI_LOG
        printf("[User] webinix_close()... \n");
    #endif

    _webinix_init();

    if(win->connected) {

        // Prepare packets
        char* packet = (char*) _webinix_malloc(4);
        packet[0] = WEBUI_HEADER_SIGNATURE; // Signature
        packet[1] = WEBUI_HEADER_CLOSE;     // Type
        packet[2] = 0;                      // ID
        packet[3] = 0;                      // Data

        // Send packets
        _webinix_window_send(win, packet, 4);
        _webinix_free_mem((void*)packet);
    }
}

bool webinix_is_shown(void* window) {

    // Dereference
    _webinix_window_t* win = (_webinix_window_t*)window;

    #ifdef WEBUI_LOG
        printf("[User] webinix_is_shown()... \n");
    #endif

    return win->connected;
}

void webinix_set_multi_access(void* window, bool status) {

    // Dereference
    _webinix_window_t* win = (_webinix_window_t*)window;

    #ifdef WEBUI_LOG
        printf("[User] webinix_set_multi_access([%d])... \n", status);
    #endif

    win->multi_access = status;
}

void webinix_set_icon(void* window, const char* icon, const char* icon_type) {

    // Dereference
    _webinix_window_t* win = (_webinix_window_t*)window;

    #ifdef WEBUI_LOG
        printf("[User] webinix_set_icon([%s], [%s])... \n", icon, icon_type);
    #endif

    win->icon = icon;
    win->icon_type = icon_type;
}

bool webinix_show(void* window, const char* content) {

    // Dereference
    _webinix_window_t* win = (_webinix_window_t*)window;

    #ifdef WEBUI_LOG
        printf("[User] webinix_show()... \n");
    #endif

    // Find the best web browser to use
    unsigned int browser = _webinix_core.current_browser != 0 ? _webinix_core.current_browser : _webinix_find_the_best_browser(win);

    // Show the window
    return _webinix_show(win, content, browser);
}

bool webinix_show_browser(void* window, const char* content, unsigned int browser) {

    // Dereference
    _webinix_window_t* win = (_webinix_window_t*)window;

    #ifdef WEBUI_LOG
        printf("[User] webinix_show_browser([%u])... \n", browser);
    #endif

    return _webinix_show(win, content, browser);
}

unsigned int webinix_bind(void* window, const char* element, void (*func)(webinix_event_t* e)) {

    // Dereference
    _webinix_window_t* win = (_webinix_window_t*)window;

    #ifdef WEBUI_LOG
        printf("[User] webinix_bind([%s], [0x%p])... \n", element, func);
    #endif

    _webinix_init();

    int len = 0;
    if(_webinix_is_empty(element))
        win->has_events = true;
    else
        len = strlen(element);

    // [win num][/][element]
    char* webinix_internal_id = _webinix_malloc(3 + 1 + len);
    sprintf(webinix_internal_id, "%u/%s", win->window_number, element);

    unsigned int cb_index = _webinix_get_cb_index(webinix_internal_id);

    if(cb_index > 0) {

        // Replace a reference
        _webinix_core.cb[cb_index] = func;

        _webinix_free_mem((void*)webinix_internal_id);
    }
    else {

        // New reference
        cb_index = _webinix_set_cb_index(webinix_internal_id);

        if(cb_index > 0)
            _webinix_core.cb[cb_index] = func;
        else
            _webinix_free_mem((void*)webinix_internal_id);
    }

    return cb_index;
}

const char* webinix_get_string(webinix_event_t* e) {
    
    #ifdef WEBUI_LOG
        printf("[User] webinix_get_string()... \n");
    #endif

    if(e->data != NULL) {
        size_t len = strlen(e->data);
        if(len > 0 && len <= WEBUI_MAX_BUF)
            return (const char *) e->data;
    }

    return webinix_empty_string;
}

long long int webinix_get_int(webinix_event_t* e) {
    
    #ifdef WEBUI_LOG
        printf("[User] webinix_get_int()... \n");
    #endif

    char *endptr;

    if(e->data != NULL) {
        size_t len = strlen(e->data);
        if(len > 0 && len <= 20) // 64-bit max is -9,223,372,036,854,775,808 (20 character)
            return strtoll((const char *) e->data, &endptr, 10);
    }
    
    return 0;
}

bool webinix_get_bool(webinix_event_t* e) {

    #ifdef WEBUI_LOG
        printf("[User] webinix_get_bool()... \n");
    #endif

    const char* str = webinix_get_string(e);
    if(str[0] == 't' || str[0] == 'T') // true || True
        return true;

    return false;
}

void webinix_return_int(webinix_event_t* e, long long int n) {

    #ifdef WEBUI_LOG
        printf("[User] webinix_return_int([%lld])... \n", n);
    #endif

    // Dereference
    _webinix_window_t* win = (_webinix_window_t*)e->window;

    // Get buffer
    if(win->event_core[e->event_number] == NULL)
        return;
    char** response = &win->event_core[e->event_number]->response;

    // Free
    if(*response != NULL)
        _webinix_free_mem((void*)*response);

    // Int to Str
    // 64-bit max is -9,223,372,036,854,775,808 (20 character)
    char* buf = (char*) _webinix_malloc(20);
    sprintf(buf, "%lld", n);

    // Set response
    *response = buf;
}

void webinix_return_string(webinix_event_t* e, char* s) {

    #ifdef WEBUI_LOG
        printf("[User] webinix_return_string([%s])... \n", s);
    #endif

    if(_webinix_is_empty(s))
        return;

    // Dereference
    _webinix_window_t* win = (_webinix_window_t*)e->window;

    // Get buffer
    if(win->event_core[e->event_number] == NULL)
        return;
    char** response = &win->event_core[e->event_number]->response;
    
    // Free
    if(*response != NULL)
        _webinix_free_mem((void*)*response);

    // Copy Str
    int len = strlen(s);
    char* buf = (char*) _webinix_malloc(len);
    memcpy(buf, s, len);

    // Set response
    *response = buf;
}

void webinix_return_bool(webinix_event_t* e, bool b) {

    #ifdef WEBUI_LOG
        printf("[User] webinix_return_bool([%d])... \n", b);
    #endif

    // Dereference
    _webinix_window_t* win = (_webinix_window_t*)e->window;

    // Get buffer
    if(win->event_core[e->event_number] == NULL)
        return;
    char** response = &win->event_core[e->event_number]->response;

    // Free
    if(*response != NULL)
        _webinix_free_mem((void*)*response);

    // Bool to Str
    int len = 1;
    char* buf = (char*) _webinix_malloc(len);
    sprintf(buf, "%d", b);

    // Set response
    *response = buf;
}

void webinix_exit(void) {

    #ifdef WEBUI_LOG
        printf("[User] webinix_exit()... \n");
    #endif

    _webinix_core.exit_now = true;

    // Let's give other threads more time to 
    // safely exit and finish their cleaning up.
    _webinix_sleep(100);
}

void webinix_wait(void) {

    #ifdef WEBUI_LOG
        printf("[Loop] webinix_wait()... \n");
    #endif

    _webinix_init();

    if(_webinix_core.startup_timeout > 0) {

        #ifdef WEBUI_LOG
            printf("[Loop] webinix_wait() -> Using timeout %u second\n", _webinix_core.startup_timeout);
        #endif

        // Wait for browser to start
        _webinix_wait_for_startup();

        #ifdef WEBUI_LOG
            printf("[Loop] webinix_wait() -> Wait for connected UI...\n");
        #endif

        while(_webinix_core.servers > 0) {

            #ifdef WEBUI_LOG
                // printf("[%u/%u]", _webinix_core.servers, _webinix_core.connections);
            #endif
            _webinix_sleep(50);
        }
    }
    else {

        #ifdef WEBUI_LOG
            printf("[Loop] webinix_wait() -> Infinite wait...\n");
        #endif

        // Infinite wait
        while(!_webinix_core.exit_now)
            _webinix_sleep(50);
    }

    #ifdef WEBUI_LOG
        printf("[Loop] webinix_wait() -> Wait finished.\n");
    #endif

    // Final cleaning
    _webinix_clean();
}

void webinix_set_timeout(unsigned int second) {

    #ifdef WEBUI_LOG
        printf("[User] webinix_set_timeout([%u])... \n", second);
    #endif

    _webinix_init();

    if(second > WEBUI_MAX_TIMEOUT)
        second = WEBUI_MAX_TIMEOUT;

    _webinix_core.startup_timeout = second;
}

void webinix_set_runtime(void* window, unsigned int runtime) {

    // Dereference
    _webinix_window_t* win = (_webinix_window_t*)window;

    #ifdef WEBUI_LOG
        printf("[User] webinix_script_runtime(%u)... \n", runtime);
    #endif

    _webinix_init();

    if(runtime != Deno && runtime != NodeJS)
        win->runtime = None;
    else
        win->runtime = runtime;
}

// -- Interface's Functions ----------------
void _webinix_interface_bind_handler(webinix_event_t* e) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_interface_bind_handler()... \n");
    #endif

    // Generate Webinix internal id
    char* webinix_internal_id = _webinix_generate_internal_id(e->window, e->element);
    unsigned int cb_index = _webinix_get_cb_index(webinix_internal_id);

    // void* window; // Pointer to the window object
    // unsigned int event_type; // Event type
    // char* element; // HTML element ID
    // char* data; // JavaScript data
    // unsigned int event_number; // Internal Webinix

    if(cb_index > 0 && _webinix_core.cb_interface[cb_index] != NULL) {

        #ifdef WEBUI_LOG
            printf("[Core]\t\t_webinix_interface_bind_handler() -> User callback @ 0x%p\n", _webinix_core.cb_interface[cb_index]);
            printf("[Core]\t\t_webinix_interface_bind_handler() -> e->event_type [%u]\n", e->event_type);
            printf("[Core]\t\t_webinix_interface_bind_handler() -> e->element [%s]\n", e->element);
            printf("[Core]\t\t_webinix_interface_bind_handler() -> e->data [%s]\n", e->data);
            printf("[Core]\t\t_webinix_interface_bind_handler() -> e->event_number %d\n", e->event_number);
        #endif

        // Call cb
        _webinix_core.cb_interface[cb_index](e->window, e->event_type, e->element, e->data, e->event_number);
    }

    // Free
    _webinix_free_mem((void*)webinix_internal_id);

    #ifdef WEBUI_LOG
        // Print cb response
        char* response = NULL;
        _webinix_window_t* win = (_webinix_window_t*)e->window;
        if(win->event_core[e->event_number] != NULL)
            response = *(&win->event_core[e->event_number]->response);
        printf("[Core]\t\t_webinix_interface_bind_handler() -> user-callback response [%s]\n", response);
    #endif
}

unsigned int webinix_interface_bind(void* window, const char* element, void (*func)(void*, unsigned int, char*, char*, unsigned int)) {

    // Dereference
    _webinix_window_t* win = (_webinix_window_t*)window;

    #ifdef WEBUI_LOG
        printf("[User] webinix_interface_bind()... \n");
    #endif

    // Bind
    unsigned int cb_index = webinix_bind(win, element, _webinix_interface_bind_handler);
    _webinix_core.cb_interface[cb_index] = func;
    return cb_index;
}

void webinix_interface_set_response(void* window, unsigned int event_number, const char* response) {

    #ifdef WEBUI_LOG
        printf("[User] webinix_interface_set_response()... \n");
        printf("[User] webinix_interface_set_response() -> event_number %d \n", event_number);
        printf("[User] webinix_interface_set_response() -> Response [%s] \n", response);
    #endif

    // Dereference
    _webinix_window_t* win = (_webinix_window_t*)window;

    // Get internal response buffer
    if(win->event_core[event_number] != NULL) {

        char** buffer = NULL;
        buffer = &win->event_core[event_number]->response;

        // Set the response
        size_t len = strlen(response);
        *buffer = (char*) _webinix_malloc(len);
        strcpy(*buffer, response);

        #ifdef WEBUI_LOG
            printf("[User] webinix_interface_set_response() -> Internal buffer [%s] \n", *buffer);
        #endif
    }
}

bool webinix_interface_is_app_running(void) {

    #ifdef WEBUI_LOG
        // printf("[User] webinix_is_app_running()... \n");
    #endif

    static bool app_is_running = true;

    // Stop if already flagged
    if(!app_is_running) return false;

    // Initialization
    if(!_webinix_core.initialized)
        _webinix_init();
    
    // Get app status
    if(_webinix_core.exit_now) {
        app_is_running = false;
    }
    else if(_webinix_core.startup_timeout > 0) {
        if(_webinix_core.servers < 1)
            app_is_running = false;
    }

    // Final cleaning
    if(!app_is_running) {
        #ifdef WEBUI_LOG
            printf("[User] webinix_is_app_running() -> App Stopped.\n");
        #endif
        _webinix_clean();
    }

    return app_is_running;
}

unsigned int webinix_interface_get_window_id(void* window) {

    #ifdef WEBUI_LOG
        printf("[User] webinix_interface_get_window_id()... \n");
    #endif

    // Dereference
    _webinix_window_t* win = (_webinix_window_t*)window;

    return win->window_number;
}

// -- Core's Functions ----------------
bool _webinix_ptr_exist(void* ptr) {
    
    #ifdef WEBUI_LOG
        // printf("[Core]\t\t_webinix_ptr_exist()... \n");
    #endif

    if(ptr == NULL)
        return false;
    
    for(unsigned int i = 0; i < _webinix_core.ptr_position; i++) {

        if(_webinix_core.ptr_list[i] == ptr)
            return true;
    }

    return false;
}

static void _webinix_ptr_add(void* ptr, size_t size) {
    
    #ifdef WEBUI_LOG
        // printf("[Core]\t\t_webinix_ptr_add(0x%p)... \n", ptr);
    #endif

    if(ptr == NULL)
        return;

    if(!_webinix_ptr_exist(ptr)) {

        for(unsigned int i = 0; i < _webinix_core.ptr_position; i++) {

            if(_webinix_core.ptr_list[i] == NULL) {

                #ifdef WEBUI_LOG
                    printf("[Core]\t\t_webinix_ptr_add(0x%p)... Allocate %d bytes\n", ptr, (int)size);
                #endif

                _webinix_core.ptr_list[i] = ptr;
                _webinix_core.ptr_size[i] = size;
                return;
            }
        }

        #ifdef WEBUI_LOG
            printf("[Core]\t\t_webinix_ptr_add(0x%p)... Allocate %d bytes\n", ptr, (int)size);
        #endif

        _webinix_core.ptr_list[_webinix_core.ptr_position] = ptr;
        _webinix_core.ptr_size[_webinix_core.ptr_position] = size;
        _webinix_core.ptr_position++;
        if(_webinix_core.ptr_position >= WEBUI_MAX_ARRAY)
            _webinix_core.ptr_position = (WEBUI_MAX_ARRAY - 1);
    }
}

static void _webinix_free_mem(void* ptr) {
    
    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_free_mem(0x%p)... \n", ptr);
    #endif

    if(ptr == NULL)
        return;

    for(unsigned int i = 0; i < _webinix_core.ptr_position; i++) {

        if(_webinix_core.ptr_list[i] == ptr) {

            #ifdef WEBUI_LOG
                printf("[Core]\t\t_webinix_free_mem(0x%p)... Free %d bytes\n", ptr, (int)_webinix_core.ptr_size[i]);
            #endif

            memset(ptr, 0, _webinix_core.ptr_size[i]);
            free(ptr);

            _webinix_core.ptr_size[i] = 0;
            _webinix_core.ptr_list[i] = NULL;
        }
    }

    for(int i = _webinix_core.ptr_position; i >= 0; i--) {

        if(_webinix_core.ptr_list[i] == NULL) {

            _webinix_core.ptr_position = i;
            break;
        }
    }
}

static void _webinix_free_all_mem(void) {
    
    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_free_all_mem()... \n");
    #endif

    // Makes sure we run this once
    static bool freed = false;
    if(freed) return;
    freed = true;

    void* ptr = NULL;
    for(unsigned int i = 0; i < _webinix_core.ptr_position; i++) {

        ptr = _webinix_core.ptr_list[i];

        if(ptr != NULL) {

            #ifdef WEBUI_LOG
                printf("[Core]\t\t_webinix_free_all_mem()... Free %d bytes @ 0x%p\n", (int)_webinix_core.ptr_size[i], ptr);
            #endif

            memset(ptr, 0, _webinix_core.ptr_size[i]);
            free(ptr);
        }
    }
}

static void _webinix_panic(void) {
    
    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_panic()... \n");
    #endif

    webinix_exit();
    exit(EXIT_FAILURE);
}

size_t _webinix_round_to_memory_block(int size) {

    // If size is negative
    if(size < 4)
        size = 4;

    // If size is already a full block
    // we should return the same block
    size--;

    int block_size = 4;
    while (block_size <= size)
        block_size *= 2;

    return (size_t)block_size;
}

void* _webinix_malloc(int size) {
    
    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_malloc([%d])... \n", size);
    #endif
    
    // Make sure we have the null
    // terminator if it's a string
    size++;

    size = _webinix_round_to_memory_block(size);

    void* block = NULL;
    for(unsigned int i = 0; i < 8; i++) {

        if(size > WEBUI_MAX_BUF)
            size = WEBUI_MAX_BUF;

        block = malloc(size);

        if(block == NULL)
            size++;
        else
            break;
    }

    if(block == NULL) {

        _webinix_panic();
        return NULL;
    }

    memset(block, 0, size);

    _webinix_ptr_add((void*)block, size);

    return block;
}

unsigned int _webinix_get_free_event_core_pos(_webinix_window_t* win) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_get_free_event_core_pos()... \n");
    #endif

    for(unsigned int i = 1; i < WEBUI_MAX_ARRAY; i++) {
        if(win->event_core[i] == NULL)
            return i;
    }

    // Fatal. No free pos found
    // let's use the first pos
    return 0;
}

static void _webinix_sleep(long unsigned int ms) {
    
    #ifdef WEBUI_LOG
        // printf("[Core]\t\t_webinix_sleep([%u])... \n", ms);
    #endif

    #ifdef _WIN32
        Sleep(ms);
    #else
        usleep(ms);
    #endif
}

long _webinix_timer_diff(struct timespec *start, struct timespec *end) {

    #ifdef WEBUI_LOG
        // printf("[Core]\t\t_webinix_timer_diff()... \n");
    #endif

    return (
        (long)(end->tv_sec * 1000) +
        (long)(end->tv_nsec / 1000000)) -
        ((long)(start->tv_sec * 1000) +
        (long)(start->tv_nsec / 1000000)
    );
}

static void _webinix_timer_clock_gettime(struct timespec *spec) {

    #ifdef WEBUI_LOG
        // printf("[Core]\t\t_webinix_timer_clock_gettime()... \n");
    #endif

    #ifdef _WIN32
        __int64 wintime;
        GetSystemTimeAsFileTime((FILETIME*)&wintime);
        wintime      -= ((__int64)116444736000000000);
        spec->tv_sec  = wintime / ((__int64)10000000);
        spec->tv_nsec = wintime % ((__int64)10000000) * 100;
    #else
        clock_gettime(CLOCK_MONOTONIC, spec);
    #endif
}

static void _webinix_timer_start(_webinix_timer_t* t) {

    #ifdef WEBUI_LOG
        // printf("[Core]\t\t_webinix_timer_start()... \n");
    #endif
    
    _webinix_timer_clock_gettime(&t->start);
}

bool _webinix_timer_is_end(_webinix_timer_t* t, unsigned int ms) {

    #ifdef WEBUI_LOG
        // printf("[Core]\t\t_webinix_timer_is_end()... \n");
    #endif
    
    _webinix_timer_clock_gettime(&t->now);

    unsigned int def = (unsigned int) _webinix_timer_diff(&t->start, &t->now);
    if(def > ms)
        return true;
    return false;
}

bool _webinix_is_empty(const char* s) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_is_empty()... \n");
    #endif

    if((s != NULL) && (s[0] != '\0'))
        return false;
    return true;
}

bool _webinix_file_exist_mg(void *ev_data) {
    
    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_file_exist_mg()... \n");
    #endif

    char* file;
    char* full_path;

    struct mg_http_message *hm = (struct mg_http_message *) ev_data;

    // Get file name
    file = (char*) _webinix_malloc(hm->uri.len);
    const char* p = hm->uri.ptr;
    p++; // Skip "/"
    sprintf(file, "%.*s", (int)(hm->uri.len - 1), p);

    // Get full path
    // [current folder][/][file]
    full_path = (char*) _webinix_malloc(strlen(_webinix_core.executable_path) + 1 + strlen(file));
    sprintf(full_path, "%s%s%s", _webinix_core.executable_path, webinix_sep, file);

    bool exist = _webinix_file_exist(full_path);

    _webinix_free_mem((void*)file);
    _webinix_free_mem((void*)full_path);

    return exist;
}

bool _webinix_file_exist(char* file) {
    
    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_file_exist([%s])... \n", file);
    #endif

    if(_webinix_is_empty(file))
        return false;

    if(WEBUI_FILE_EXIST(file, 0) == 0)
        return true;
    return false;
}

const char* _webinix_get_extension(const char*f) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_get_extension()... \n");
    #endif

    if(f == NULL)
        return webinix_empty_string;

    const char*ext = strrchr(f, '.');

    if(ext == NULL || !ext || ext == f)
        return webinix_empty_string;
    return ext + 1;
}

unsigned char _webinix_get_run_id(void) {
    
    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_get_run_id()... \n");
    #endif

    return ++_webinix_core.run_last_id;
}

bool _webinix_socket_test_listen_mg(unsigned int port_num) {
    
    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_socket_test_listen_mg([%u])... \n", port_num);
    #endif

    struct mg_connection *c;
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);

    char url[32];
    sprintf(url, "http://localhost:%u", port_num);

    if((c = mg_http_listen(&mgr, url, NULL, &mgr)) == NULL) {

        // Cannot listen
        mg_mgr_free(&mgr);
        return false;
    }

    // Listening success
    mg_mgr_free(&mgr);

    return true;
}

bool _webinix_port_is_used(unsigned int port_num) {
    
    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_port_is_used([%u])... \n", port_num);
    #endif

    #ifdef _WIN32
        // Listener test
        if(!_webinix_socket_test_listen_win32(port_num))
            return true; // Port is already used
        return false; // Port is not in use
    #else
        // Listener test MG
        if(!_webinix_socket_test_listen_mg(port_num))
            return true; // Port is already used
        return false; // Port is not in use
    #endif
}

static void _webinix_serve_file(_webinix_window_t* win, struct mg_connection *c, void *ev_data) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_serve_file()... \n");
    #endif

    // Serve a normal text based file
    // send with HTTP 200 status code

    struct mg_http_serve_opts opts = {

        .root_dir = win->server_root_path
    };

    mg_http_serve_dir(c, ev_data, &opts);
}

bool _webinix_deno_exist(void) {
    
    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_deno_exist()... \n");
    #endif

    static bool found = false;

    if(found)
        return true;

    if(_webinix_cmd_sync("deno --version", false) == 0) {

        found = true;
        return true;
    }
    else
        return false;
}

bool _webinix_nodejs_exist(void) {
    
    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_nodejs_exist()... \n");
    #endif

    static bool found = false;

    if(found)
        return true;

    if(_webinix_cmd_sync("node -v", false) == 0) {

        found = true;
        return true;
    }
    else
        return false;
}

const char* _webinix_interpret_command(const char* cmd) {
    
    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_interpret_command([%s])... \n", cmd);
    #endif

    // Run the command with redirection of errors to stdout
    // and return the output.

    // Output buffer
    char* out = NULL;

    #ifdef _WIN32
        // Redirect stderr to stdout
        char cmd_with_redirection[512];
        sprintf(cmd_with_redirection, "cmd.exe /c %s 2>&1", cmd);    
        _webinix_system_win32_out(cmd_with_redirection, &out, false);
    #else
        // Redirect stderr to stdout
        char cmd_with_redirection[512];
        sprintf(cmd_with_redirection, "%s 2>&1", cmd);    

        FILE *pipe = WEBUI_POPEN(cmd_with_redirection, "r");

        if(pipe == NULL)
            return NULL;
        
        // Read STDOUT
        out = (char*) _webinix_malloc(WEBUI_CMD_STDOUT_BUF);
        char* line = (char*) _webinix_malloc(1024);
        while(fgets(line, 1024, pipe) != NULL)
            strcat(out, line);
        WEBUI_PCLOSE(pipe);

        // Clean
        _webinix_free_mem((void*)line);
    #endif

    return (const char*)out;
}

static void _webinix_interpret_file(_webinix_window_t* win, struct mg_connection *c, void *ev_data, char* index) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_interpret_file()... \n");
    #endif

    // Interpret the file using JavaScript/TypeScript runtimes
    // and send back the output. otherwise, send the file as a normal text based    

    char* file;
    char* full_path;
    char* query;

    // Get file full path
    if(index != NULL && !_webinix_is_empty(index)) {

        // Parse as index file

        file = index;
        full_path = index;
    }
    else {

        // Parse as other non-index files

        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        // Get file name
        file = (char*) _webinix_malloc(hm->uri.len);
        const char* p = hm->uri.ptr;
        p++; // Skip "/"
        sprintf(file, "%.*s", (int)(hm->uri.len - 1), p);

        // Get full path
        // [current folder][/][file]
        full_path = (char*) _webinix_malloc(strlen(_webinix_core.executable_path) + 1 + strlen(file));
        sprintf(full_path, "%s%s%s", _webinix_core.executable_path, webinix_sep, file);

        if(!_webinix_file_exist(full_path)) {

            // File not exist - 404
            mg_http_reply(
                c, 404,
                "",
                webinix_html_res_not_available
            );

            _webinix_free_mem((void*)file);
            _webinix_free_mem((void*)full_path);
            return;
        }

        // Get query
        query = (char*) _webinix_malloc(hm->query.len);
        sprintf(query, "%.*s", (int)hm->query.len, hm->query.ptr);
    }

    // Get file extension
    const char* extension = _webinix_get_extension(file);

    if(strcmp(extension, "ts") == 0 || strcmp(extension, "js") == 0) {

        // TypeScript / JavaScript

        if(win->runtime == Deno) {

            // Use Deno
            if(_webinix_deno_exist()) {

                // Set command
                // [disable coloring][file]
                char* cmd = (char*) _webinix_malloc(64 + strlen(full_path));
                #ifdef _WIN32
                    sprintf(cmd, "Set NO_COLOR=1 & Set DENO_NO_UPDATE_CHECK=1 & deno run --quiet --allow-all --unstable \"%s\" \"%s\"", full_path, query);
                #else
                    sprintf(cmd, "NO_COLOR=1; DENO_NO_UPDATE_CHECK=1; deno run --quiet --allow-all --unstable \"%s\" \"%s\"", full_path, query);
                #endif

                // Run command
                const char* out = _webinix_interpret_command(cmd);

                if(out != NULL) {

                    // Send Deno output
                    mg_http_reply(
                        c, 200,
                        "",
                        out
                    );
                }
                else {

                    // Deno failed.
                    // Serve as a normal text-based file
                    _webinix_serve_file(win, c, ev_data);
                }

                _webinix_free_mem((void*)cmd);
                _webinix_free_mem((void*)out);
            }
            else {

                // Deno not installed

                mg_http_reply(
                    c, 404,
                    "",
                    webinix_deno_not_found
                );
            }
        }
        else if(win->runtime == NodeJS) {

            // Use Nodejs

            if(_webinix_nodejs_exist()) {

                // Set command
                // [node][file]
                char* cmd = (char*) _webinix_malloc(16 + strlen(full_path));
                sprintf(cmd, "node \"%s\" \"%s\"", full_path, query);

                // Run command
                const char* out = _webinix_interpret_command(cmd);

                if(out != NULL) {

                    // Send Node.js output
                    mg_http_reply(
                        c, 200,
                        "",
                        out
                    );
                }
                else {

                    // Node.js failed.
                    // Serve as a normal text-based file
                    _webinix_serve_file(win, c, ev_data);
                }

                _webinix_free_mem((void*)cmd);
                _webinix_free_mem((void*)out);
            }
            else {

                // Node.js not installed

                mg_http_reply(
                    c, 404,
                    "",
                    webinix_nodejs_not_found
                );
            }
        }
        else {

            // Unknown runtime
            // Serve as a normal text-based file
            _webinix_serve_file(win, c, ev_data);
        }
    }
    else {

        // Unknown file extension
        // Serve as a normal text-based file
        _webinix_serve_file(win, c, ev_data);
    }

    _webinix_free_mem((void*)file);
    _webinix_free_mem((void*)full_path);
}

const char* _webinix_generate_js_bridge(_webinix_window_t* win) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_generate_js_bridge()... \n");
    #endif

    // Calculate the cb size
    size_t cb_mem_size = 64; // To hold 'const _webinix_bind_list = ["elem1", "elem2",];'
    for(unsigned int i = 1; i < WEBUI_MAX_ARRAY; i++)
        if(_webinix_core.html_elements[i] != NULL && !_webinix_is_empty(_webinix_core.html_elements[i]))
            cb_mem_size += strlen(_webinix_core.html_elements[i]) + 3;
    
    // Generate the cb array
    char* event_cb_js_array = (char*) _webinix_malloc(cb_mem_size);
    strcat(event_cb_js_array, "const _webinix_bind_list = [");
    for(unsigned int i = 1; i < WEBUI_MAX_ARRAY; i++) {
        if(_webinix_core.html_elements[i] != NULL && !_webinix_is_empty(_webinix_core.html_elements[i])) {
            strcat(event_cb_js_array, "\"");
            strcat(event_cb_js_array, _webinix_core.html_elements[i]);
            strcat(event_cb_js_array, "\",");
        }
    }
    strcat(event_cb_js_array, "]; \n");

    // Generate the full Webinix JS-Bridge
    size_t len = cb_mem_size + strlen(webinix_javascript_bridge);
    char* js = (char*) _webinix_malloc(len);
    sprintf(js, 
        "_webinix_port = %u; \n_webinix_win_num = %u; \n%s \n%s \n",
        win->server_port, win->window_number, event_cb_js_array, webinix_javascript_bridge
    );

    return js;
}

static void _webinix_server_event_handler(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {

    #ifdef WEBUI_LOG
        // printf("[Core]\t\t_webinix_server_event_handler()... \n");
    #endif

    _webinix_window_t* win = (_webinix_window_t *) fn_data;

    if(ev == MG_EV_HTTP_MSG) {

        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        if(mg_http_match_uri(hm, "/_webinix_ws_connect")) {

            // WebSocket

            #ifdef WEBUI_LOG
                printf("[Core]\t\t_webinix_server_event_handler()... HTML Upgrade to WebSocket\n");
            #endif

            mg_ws_upgrade(c, hm, NULL);
        } 
        else if(mg_http_match_uri(hm, "/webinix.js")) {

            // Webinix JS-Bridge

            #ifdef WEBUI_LOG
                printf("[Core]\t\t_webinix_server_event_handler()... HTML Webinix JS\n");
            #endif

            // Generate JavaScript bridge
            const char* js = _webinix_generate_js_bridge(win);

            // Header
            // Content-Type: text/javascript

            // Send
            mg_http_reply(
                c, 200,
                "",
                js
            );

            _webinix_free_mem((void*)js);
        }
        else if(strncmp(hm->uri.ptr, "/WEBUI/FUNC/", 12) == 0 && hm->uri.len >= 15) {
            
            // Function Call (With response)

            // [/WEBUI/FUNC/ELEMENT_ID/DATA]
            // 0            12

            #ifdef WEBUI_LOG
                printf("[Core]\t\t_webinix_server_event_handler()... CB start\n");
            #endif

            // Copy packet
            size_t len = hm->uri.len;
            char* packet = (char*) _webinix_malloc(len);
            memcpy(packet, hm->uri.ptr, len);

            // Get html element id
            char* element = &packet[12];
            size_t element_len = 0;
            for (size_t i = 12; i < len; i++) {
                if(packet[i] == '/') {
                    packet[i] = '\0';
                    break;
                }
                element_len++;
            }

            // [/WEBUI/FUNC/ELEMENT_ID DATA]
            // 0            12

            // Get data
            char* data = &packet[11 + element_len + 2];
            size_t data_len = strlen(data);

            // Generate Webinix internal id
            char* webinix_internal_id = _webinix_generate_internal_id(win, element);

            // Create new event core to hold the response
            webinix_event_core_t* event_core = (webinix_event_core_t*) _webinix_malloc(sizeof(webinix_event_core_t));
            unsigned int event_core_pos = _webinix_get_free_event_core_pos(win);
            win->event_core[event_core_pos] = event_core;
            char** response = &win->event_core[event_core_pos]->response;

            // Create new event
            webinix_event_t e;
            e.window = win;
            e.event_type = WEBUI_EVENT_CALLBACK;
            e.element = element;
            e.data = data;
            e.event_number = event_core_pos;

            // Call user function
            unsigned int cb_index = _webinix_get_cb_index(webinix_internal_id);
            if(cb_index > 0 && _webinix_core.cb[cb_index] != NULL) {

                // Call user cb
                _webinix_core.cb[cb_index](&e);
            }

            // Check the response
            if(_webinix_is_empty(*response))
                *response = (char*)webinix_empty_string;

            #ifdef WEBUI_LOG
                printf("[Core]\t\t_webinix_server_event_handler()... user-callback response [%s]\n", *response);
            #endif

            // Send response
            mg_http_reply(
                c, 200,
                "",
                *response
            );

            // Free
            _webinix_free_mem((void*)packet);
            _webinix_free_mem((void*)webinix_internal_id);
            _webinix_free_mem((void*)*response);
            _webinix_free_mem((void*)event_core);
        }
        else if(mg_http_match_uri(hm, "/")) {

            // [/]

            if(win->is_embedded_html) {

                // Main HTML

                if(!win->multi_access && win->html_handled) {

                    // Main HTML already handled.
                    // Forbidden 403

                    #ifdef WEBUI_LOG
                        printf("[Core]\t\t_webinix_server_event_handler()... Embedded Index HTML Already Handled (403)\n");
                    #endif

                    // Header
                    // text/html; charset=utf-8

                    mg_http_reply(
                        c, 403,
                        "",
                        webinix_html_served
                    );
                }
                else {

                    // Send main HTML

                    win->html_handled = true;

                    #ifdef WEBUI_LOG
                        printf("[Core]\t\t_webinix_server_event_handler()... Embedded Index HTML\n");
                    #endif

                    char* html = (char*) webinix_empty_string;

                    if(win->html != NULL) {

                        // Generate the full Webinix JS-Bridge
                        const char* js = _webinix_generate_js_bridge(win);

                        // Inject Webinix JS-Bridge into HTML
                        size_t len = strlen(win->html) + strlen(js) + 128;
                        html = (char*) _webinix_malloc(len);
                        sprintf(html, 
                            "%s \n <script type = \"text/javascript\"> \n %s \n </script>",
                            win->html, js
                        );

                        _webinix_free_mem((void*)js);
                    }

                    // Send
                    mg_http_reply(
                        c, 200,
                        "",
                        html
                    );

                    _webinix_free_mem((void*)html);
                }
            }
            else {

                // Serve as index local file

                win->html_handled = true;

                #ifdef WEBUI_LOG
                    printf("[Core]\t\t_webinix_server_event_handler()... Local Index File\n");
                #endif

                // Set full path
                // [Path][Sep][File Name]
                char* index = (char*) _webinix_malloc(strlen(_webinix_core.executable_path) + 1 + 8); 

                // Index.ts
                sprintf(index, "%s%sindex.ts", _webinix_core.executable_path, webinix_sep);
                if(_webinix_file_exist(index)) {

                    // TypeScript Index
                    if(win->runtime != None)
                        _webinix_interpret_file(win, c, ev_data, index);
                    else
                        _webinix_serve_file(win, c, ev_data);

                   _webinix_free_mem((void*)index);
                    return;
                }

                // Index.js
                sprintf(index, "%s%sindex.js", _webinix_core.executable_path, webinix_sep);
                if(_webinix_file_exist(index)) {

                    // JavaScript Index
                    if(win->runtime != None)
                        _webinix_interpret_file(win, c, ev_data, index);
                    else
                        _webinix_serve_file(win, c, ev_data);

                    _webinix_free_mem((void*)index);
                    return;
                }

                _webinix_free_mem((void*)index);
                
                // Index.html
                // Serve as a normal HTML text-based file
                _webinix_serve_file(win, c, ev_data);
            }
        }
        else if(mg_http_match_uri(hm, "/favicon.ico") || mg_http_match_uri(hm, "/favicon.svg")) {

            // Favicon

            if(win->icon != NULL && win->icon_type != NULL) {

                // Custom user icon

                char* icon_header = (char*) _webinix_malloc(strlen(win->icon_type) + 32);
                sprintf(icon_header, "Content-Type: %s\r\n", win->icon_type);

                // User icon
                mg_http_reply(
                    c, 200,
                    icon_header,
                    win->icon
                );
            }
            else if(_webinix_file_exist_mg(ev_data)) {

                // Local icon file
                _webinix_serve_file(win, c, ev_data);
            }
            else {

                // Default embedded icon

                if(mg_http_match_uri(hm, "/favicon.ico")) {

                    mg_http_reply(c, 302, "Location: /favicon.svg\r\n", "");
                }
                else {

                    // TODO: Use webinix_def_icon_type

                    // Header
                    // Content-Type: image/svg+xml
                    
                    // Default icon
                    mg_http_reply(
                        c, 200,
                        webinix_def_icon_type,
                        webinix_def_icon
                    );
                }
            }
        }
        else {

            // [/file]

            if(win->runtime != None) {

                #ifdef WEBUI_LOG
                    printf("[Core]\t\t_webinix_server_event_handler()... Trying to interpret local file\n");
                #endif
                
                _webinix_interpret_file(win, c, ev_data, NULL);
            }
            else {

                #ifdef WEBUI_LOG
                    printf("[Core]\t\t_webinix_server_event_handler()... Text based local file\n");
                #endif
                
                // Serve as a normal text-based file
                _webinix_serve_file(win, c, ev_data);
            }
        }
    }
    else if(ev == MG_EV_WS_MSG) {

        #ifdef WEBUI_LOG
            printf("[Core]\t\t_webinix_server_event_handler()... WebSocket Data\n");
        #endif

        struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;

        // Parse the packet
        _webinix_window_receive(win, wm->data.ptr, wm->data.len);
    }
    else if(ev == MG_EV_WS_OPEN) {

        #ifdef WEBUI_LOG
            printf("[Core]\t\t_webinix_server_event_handler()... WebSocket Connected\n");
        #endif

        int event_type = WEBUI_EVENT_CONNECTED;

        if(!win->connected) {

            // First connection

            win->connected = true; // server thread
            _webinix_core.connections++; // main loop
            _webinix_core.mg_connections[win->window_number] = c; // websocket send func
        }
        else {

            if(win->multi_access) {

                // Multi connections
                win->connections++;
                event_type = WEBUI_EVENT_MULTI_CONNECTION;
            }
            else {

                // UNWANTED Multi connections

                #ifdef WEBUI_LOG
                    printf("[Core]\t\t_webinix_server_event_handler() -> UNWANTED Multi Connections\n");
                #endif

                mg_close_conn(c);
                event_type = WEBUI_EVENT_UNWANTED_CONNECTION;
            }
        }

        // New Event
        if(win->has_events) {

            // Generate Webinix internal id
            char* webinix_internal_id = _webinix_generate_internal_id(win, "");

            _webinix_window_event(
                win,                // Event -> Window
                event_type,         // Event -> Type of this event
                "",                 // Event -> HTML Element
                NULL,               // Event -> User Custom Data
                0,                  // Event -> Event Number
                webinix_internal_id   // Extras -> Webinix Internal ID
            );
        }
    }
    else if(ev == MG_EV_WS_CTL) {

        win->html_handled = false;

        #ifdef WEBUI_LOG
            printf("[Core]\t\t_webinix_server_event_handler()... WebSocket Closed\n");
        #endif

        if(win->connected) {

            if(win->multi_access && win->connections > 0) {

                // Multi connections close
                win->connections--;
            }
            else {

                // Main connection close
                _webinix_core.connections--;  // main loop
                win->connected = false;     // server thread                
            }
        }

        // Events
        if(win->has_events) {

            // Generate Webinix internal id
            char* webinix_internal_id = _webinix_generate_internal_id(win, "");

            _webinix_window_event(
                win,                        // Event -> Window
                WEBUI_EVENT_DISCONNECTED,   // Event -> Type of this event
                "",                         // Event -> HTML Element
                NULL,                       // Event -> User Custom Data
                0,                          // Event -> Event Number
                webinix_internal_id           // Extras -> Webinix Internal ID
            );
        }
    }
}

bool _webinix_browser_create_profile_folder(_webinix_window_t* win, unsigned int browser) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_browser_create_profile_folder(%u)... \n", browser);
    #endif

    const char* temp = _webinix_browser_get_temp_path(browser);

    if(browser == Chrome) {

        // Google Chrome
        sprintf(win->profile_path, "%s%s.Webinix%sWebinixChromeProfile", temp, webinix_sep, webinix_sep);
        return true;
    }
    else if(browser == Edge) {

        // Edge
        sprintf(win->profile_path, "%s%s.Webinix%sWebinixEdgeProfile", temp, webinix_sep, webinix_sep);
        return true;
    }
    else if(browser == Epic) {

        // Epic
        sprintf(win->profile_path, "%s%s.Webinix%sWebinixEpicProfile", temp, webinix_sep, webinix_sep);
        return true;
    }
    else if(browser == Vivaldi) {

        // Vivaldi
        sprintf(win->profile_path, "%s%s.Webinix%sWebinixVivaldiProfile", temp, webinix_sep, webinix_sep);
        return true;
    }
    else if(browser == Brave) {

        // Brave
        sprintf(win->profile_path, "%s%s.Webinix%sWebinixBraveProfile", temp, webinix_sep, webinix_sep);
        return true;
    }
    else if(browser == Yandex) {

        // Yandex
        sprintf(win->profile_path, "%s%s.Webinix%sWebinixYandexProfile", temp, webinix_sep, webinix_sep);
        return true;
    }
    else if(browser == Chromium) {

        // Chromium
        sprintf(win->profile_path, "%s%s.Webinix%sWebinixChromiumProfile", temp, webinix_sep, webinix_sep);
        return true;
    }
    else if(browser == Firefox) {

        // Firefox (We need to create a folder)

        char* profile_name = "WebinixFirefoxProfile";

        char firefox_profile_path[1024];
        sprintf(firefox_profile_path, "%s%s.Webinix%s%s", temp, webinix_sep, webinix_sep, profile_name);
        
        if(!_webinix_folder_exist(firefox_profile_path)) {

            char buf[2048];

            sprintf(buf, "%s -CreateProfile \"Webinix %s\"", win->browser_path, firefox_profile_path);
            _webinix_cmd_sync(buf, false);

            // Creating the browser profile
            for(unsigned int n = 0; n <= (_webinix_core.startup_timeout * 4); n++) {

                if(_webinix_folder_exist(firefox_profile_path))
                    break;
                
                _webinix_sleep(250);
            }

            if(!_webinix_folder_exist(firefox_profile_path))
                return false;

            // prefs.js
            FILE *file;
            sprintf(buf, "%s%sprefs.js", firefox_profile_path, webinix_sep);
            file = fopen(buf, "a");
            if(file == NULL)
                return false;
            fputs("user_pref(\"toolkit.legacyUserProfileCustomizations.stylesheets\", true); ", file);
            fputs("user_pref(\"browser.shell.checkDefaultBrowser\", false); ", file);
            fputs("user_pref(\"browser.tabs.warnOnClose\", false); ", file);
            fclose(file);

            // userChrome.css
            sprintf(buf, "\"%s%schrome%s\"", firefox_profile_path, webinix_sep, webinix_sep);
            if(!_webinix_folder_exist(buf)) {

                sprintf(buf, "mkdir \"%s%schrome%s\"", firefox_profile_path, webinix_sep, webinix_sep);
                _webinix_cmd_sync(buf, false); // Create directory
            }
            sprintf(buf, "%s%schrome%suserChrome.css", firefox_profile_path, webinix_sep, webinix_sep);
            file = fopen(buf, "a");
            if(file == NULL)
                return false;
            #ifdef _WIN32
                fputs(":root{--uc-toolbar-height:32px}:root:not([uidensity=\"compact\"]) {--uc-toolbar-height:38px}#TabsToolbar{visibility:collapse!important}:root:not([inFullscreen]) #nav-bar{margin-top:calc(0px - var(--uc-toolbar-height))}#toolbar-menubar{min-height:unset!important;height:var(--uc-toolbar-height)!important;position:relative}#main-menubar{-moz-box-flex:1;background-color:var(--toolbar-bgcolor,--toolbar-non-lwt-bgcolor);background-clip:padding-box;border-right:30px solid transparent;border-image:linear-gradient(to left,transparent,var(--toolbar-bgcolor,--toolbar-non-lwt-bgcolor) 30px) 20 / 30px}#toolbar-menubar:not([inactive]) {z-index:2}#toolbar-menubar[inactive] > #menubar-items{opacity:0;pointer-events:none;margin-left:var(--uc-window-drag-space-width,0px)}#nav-bar{visibility:collapse}@-moz-document url(chrome://browser/content/browser.xhtml) {:root:not([sizemode=\"fullscreen\"]) > head{display: block;position: fixed;width: calc(200vw - 440px);text-align: left;z-index: 9;pointer-events: none;}head > *{ display: none }head > title{display: -moz-inline-box;padding: 4px;max-width: 50vw;overflow-x: hidden;text-overflow: ellipsis;}}", file);
            #elif __APPLE__
                fputs(":root{--uc-toolbar-height:32px}:root:not([uidensity=\"compact\"]) {--uc-toolbar-height:38px}#TabsToolbar{visibility:collapse!important}:root:not([inFullscreen]) #nav-bar{margin-top:calc(0px - var(--uc-toolbar-height))}#toolbar-menubar{min-height:unset!important;height:var(--uc-toolbar-height)!important;position:relative}#main-menubar{-moz-box-flex:1;background-color:var(--toolbar-bgcolor,--toolbar-non-lwt-bgcolor);background-clip:padding-box;border-right:30px solid transparent;border-image:linear-gradient(to left,transparent,var(--toolbar-bgcolor,--toolbar-non-lwt-bgcolor) 30px) 20 / 30px}#toolbar-menubar:not([inactive]) {z-index:2}#toolbar-menubar[inactive] > #menubar-items{opacity:0;pointer-events:none;margin-left:var(--uc-window-drag-space-width,0px)}#nav-bar{visibility:collapse}@-moz-document url(chrome://browser/content/browser.xhtml) {:root:not([sizemode=\"fullscreen\"]) > head{display: block;position: fixed;width: calc(200vw - 440px);text-align: left;z-index: 9;pointer-events: none;}head > *{ display: none }head > title{display: -moz-inline-box;padding: 4px;max-width: 50vw;overflow-x: hidden;text-overflow: ellipsis;}}", file);
            #else
                fputs(":root{--uc-toolbar-height:32px}:root:not([uidensity=\"compact\"]) {--uc-toolbar-height:38px}#TabsToolbar{visibility:collapse!important}:root:not([inFullscreen]) #nav-bar{margin-top:calc(0px - var(--uc-toolbar-height))}#toolbar-menubar{min-height:unset!important;height:var(--uc-toolbar-height)!important;position:relative}#main-menubar{-moz-box-flex:1;background-color:var(--toolbar-bgcolor,--toolbar-non-lwt-bgcolor);background-clip:padding-box;border-right:30px solid transparent;border-image:linear-gradient(to left,transparent,var(--toolbar-bgcolor,--toolbar-non-lwt-bgcolor) 30px) 20 / 30px}#toolbar-menubar:not([inactive]) {z-index:2}#toolbar-menubar[inactive] > #menubar-items{opacity:0;pointer-events:none;margin-left:var(--uc-window-drag-space-width,0px)}#nav-bar{visibility:collapse}@-moz-document url(chrome://browser/content/browser.xhtml) {:root:not([sizemode=\"fullscreen\"]) > head{display: block;position: fixed;width: calc(200vw - 440px);text-align: left;z-index: 9;pointer-events: none;}head > *{ display: none }head > title{display: -moz-inline-box;padding: 4px;max-width: 50vw;overflow-x: hidden;text-overflow: ellipsis;}}", file);
            #endif
            fclose(file);

            sprintf(win->profile_path, "%s%s%s", temp, webinix_sep, profile_name);
        }

        return true;
    }

    return false;
}

bool _webinix_folder_exist(char* folder) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_folder_exist([%s])... \n", folder);
    #endif

    #if defined(_MSC_VER)
        if(GetFileAttributesA(folder) != INVALID_FILE_ATTRIBUTES)
            return true;
    #else
        DIR* dir = opendir(folder);
        if(dir) {
            closedir(dir);
            return true;
        }
    #endif

    return false;
}

char* _webinix_generate_internal_id(_webinix_window_t* win, const char* element) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_generate_internal_id([%s])... \n", element);
    #endif

    // Generate Webinix internal id
    size_t element_len = strlen(element);
    size_t internal_id_size = 3 + 1 + element_len; // [win num][/][name]
    char* webinix_internal_id = (char*) _webinix_malloc(internal_id_size);
    sprintf(webinix_internal_id, "%u/%s", win->window_number, element);

    return webinix_internal_id;
}

const char* _webinix_browser_get_temp_path(unsigned int browser) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_browser_get_temp_path([%u])... \n", browser);
    #endif

    #ifdef _WIN32
        // Resolve %USERPROFILE%
        #ifdef _MSC_VER
            char* WinUserProfile = NULL;
            size_t sz = 0;
            if(_dupenv_s(&WinUserProfile, &sz, "USERPROFILE") != 0 || WinUserProfile == NULL)
                return webinix_empty_string;
        #else
            char* WinUserProfile = getenv("USERPROFILE");
            if(WinUserProfile == NULL)
                return webinix_empty_string;
        #endif
    #elif __APPLE__
        // Resolve $HOME
        char* MacUserProfile = getenv("HOME");
        if(MacUserProfile == NULL)
            return webinix_empty_string;
    #else
        // Resolve $HOME
        char* LinuxUserProfile = getenv("HOME");
        if(LinuxUserProfile == NULL)
            return webinix_empty_string;
    #endif

    #ifdef _WIN32
        return WinUserProfile;
    #elif __APPLE__
        return MacUserProfile;
    #else
        return LinuxUserProfile;
    #endif
}

bool _webinix_is_google_chrome_folder(const char* folder) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_is_google_chrome_folder([%s])... \n", folder);
    #endif

    char browser_full_path[WEBUI_MAX_PATH];

    // Make sure this folder is Google Chrome setup and not Chromium
    // by checking if `master_preferences` file exist or `initial_preferences`
    // Ref: https://support.google.com/chrome/a/answer/187948?hl=en

    sprintf(browser_full_path, "%s\\master_preferences", folder);
    if(!_webinix_file_exist(browser_full_path)) {

        sprintf(browser_full_path, "%s\\initial_preferences", folder);
        if(!_webinix_file_exist(browser_full_path))
            return false; // This is Chromium or something else
    }

    // Make sure the browser executable file exist
    sprintf(browser_full_path, "%s\\chrome.exe", folder);
    if(!_webinix_file_exist(browser_full_path))
        return false;
    
    return true;
}

bool _webinix_browser_exist(_webinix_window_t* win, unsigned int browser) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_browser_exist([%u])... \n", browser);
    #endif

    // Check if a web browser is installed on this machine

    if(browser == Chrome) {

        // Google Chrome

        static bool ChromeExist = false;
        if(ChromeExist && !_webinix_is_empty(win->browser_path)) return true;

        #ifdef _WIN32

            // Google Chrome on Windows

            char browser_folder[WEBUI_MAX_PATH];

            // Search in `HKEY_LOCAL_MACHINE` (If Google Chrome installed for multi-user)
            if(_webinix_get_windows_reg_value(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\chrome.exe", L"Path", browser_folder)) {

                // Make sure its Google Chrome and not Chromium
                if(_webinix_is_google_chrome_folder(browser_folder)) {

                    // Google Chrome Found (multi-user)
                    sprintf(win->browser_path, "\"%s\\chrome.exe\"", browser_folder);
                    ChromeExist = true;
                    return true;
                }
            }

            // Search in `HKEY_CURRENT_USER` (If Google Chrome installed for one user)
            if(_webinix_get_windows_reg_value(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\chrome.exe", L"Path", browser_folder)) {

                // Make sure its Google Chrome and not Chromium
                if(_webinix_is_google_chrome_folder(browser_folder)) {

                    // Google Chrome Found (one user)
                    sprintf(win->browser_path, "\"%s\\chrome.exe\"", browser_folder);
                    ChromeExist = true;
                    return true;
                }
            }

            return false;

        #elif __APPLE__

            // Google Chrome on macOS
            if(_webinix_cmd_sync("open -R -a \"Google Chrome\"", false) == 0) {

                sprintf(win->browser_path, "open -W \"/Applications/Google Chrome.app\" --args");
                ChromeExist = true;
                return true;
            }
            else
                return false;
        #else

            // Google Chrome on Linux
            if(_webinix_cmd_sync("google-chrome --version", false) == 0) {

                sprintf(win->browser_path, "google-chrome");
                ChromeExist = true;
                return true;
            }
            else if(_webinix_cmd_sync("google-chrome-stable --version", false) == 0) {

                sprintf(win->browser_path, "google-chrome-stable");
                ChromeExist = true;
                return true;
            }
            else
                return false;

        #endif
    }
    else if(browser == Edge) {

        // Edge

        static bool EdgeExist = false;
        if(EdgeExist && !_webinix_is_empty(win->browser_path)) return true;

        #ifdef _WIN32

            // Edge on Windows

            char browser_fullpath[WEBUI_MAX_PATH];

            // Search in `HKEY_LOCAL_MACHINE` (If Edge installed for multi-user)
            if(_webinix_get_windows_reg_value(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\msedge.exe", L"", browser_fullpath)) {

                // Make sure the browser executable file exist
                if(_webinix_file_exist(browser_fullpath)) {

                    // Edge Found (multi-user)
                    sprintf(win->browser_path, "\"%s\"", browser_fullpath);
                    EdgeExist = true;
                    return true;
                }
            }

            // Search in `HKEY_CURRENT_USER` (If Edge installed for one user)
            if(_webinix_get_windows_reg_value(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\msedge.exe", L"", browser_fullpath)) {

                // Make sure the browser executable file exist
                if(_webinix_file_exist(browser_fullpath)) {

                    // Edge Found (one user)
                    sprintf(win->browser_path, "\"%s\"", browser_fullpath);
                    EdgeExist = true;
                    return true;
                }
            }

            return false;

        #elif __APPLE__

            // Edge on macOS
            return false;

        #else

            // Edge on Linux
            if(_webinix_cmd_sync("microsoft-edge-stable --version", false) == 0) {

                sprintf(win->browser_path, "microsoft-edge-stable");
                EdgeExist = true;
                return true;
            }
            else if(_webinix_cmd_sync("microsoft-edge-dev --version", false) == 0) {

                sprintf(win->browser_path, "microsoft-edge-dev");
                EdgeExist = true;
                return true;
            }
            else
                return false;

        #endif
    }
    else if(browser == Epic) {

        // Epic Privacy Browser

        static bool EpicExist = false;
        if(EpicExist && !_webinix_is_empty(win->browser_path)) return true;

        #ifdef _WIN32

            // Epic on Windows

            char browser_fullpath[WEBUI_MAX_PATH];

            // Search in `HKEY_CURRENT_USER` (If Epic installed for one user)
            if(_webinix_get_windows_reg_value(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\epic.exe", L"", browser_fullpath)) {

                // Make sure the browser executable file exist
                if(_webinix_file_exist(browser_fullpath)) {

                    // Epic Found (one user)
                    sprintf(win->browser_path, "\"%s\"", browser_fullpath);
                    EpicExist = true;
                    return true;
                }
            }

            // Search in `HKEY_LOCAL_MACHINE` (If Epic installed for multi-user)
            if(_webinix_get_windows_reg_value(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\epic.exe", L"", browser_fullpath)) {

                // Make sure the browser executable file exist
                if(_webinix_file_exist(browser_fullpath)) {

                    // Epic Found (multi-user)
                    sprintf(win->browser_path, "\"%s\"", browser_fullpath);
                    EpicExist = true;
                    return true;
                }
            }

            return false;

        #elif __APPLE__

            // Epic on macOS
            if(_webinix_cmd_sync("open -R -a \"Epic\"", false) == 0) {

                sprintf(win->browser_path, "open -W \"/Applications/Epic.app\" --args");
                EpicExist = true;
                return true;
            }
            else
                return false;
        #else

            // Epic on Linux
            if(_webinix_cmd_sync("epic --version", false) == 0) {

                sprintf(win->browser_path, "epic");
                EpicExist = true;
                return true;
            }
            else
                return false;
        #endif
    }
    else if(browser == Vivaldi) {

        // Vivaldi Browser

        static bool VivaldiExist = false;
        if(VivaldiExist && !_webinix_is_empty(win->browser_path)) return true;

        #ifdef _WIN32

            // Vivaldi on Windows

            char browser_fullpath[WEBUI_MAX_PATH];

            // Search in `HKEY_LOCAL_MACHINE` (If Vivaldi installed for multi-user)
            if(_webinix_get_windows_reg_value(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\vivaldi.exe", L"", browser_fullpath)) {

                // Make sure the browser executable file exist
                if(_webinix_file_exist(browser_fullpath)) {

                    // Vivaldi Found (multi-user)
                    sprintf(win->browser_path, "\"%s\"", browser_fullpath);
                    VivaldiExist = true;
                    return true;
                }
            }

            // Search in `HKEY_CURRENT_USER` (If Vivaldi installed for one user)
            if(_webinix_get_windows_reg_value(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\vivaldi.exe", L"", browser_fullpath)) {

                // Make sure the browser executable file exist
                if(_webinix_file_exist(browser_fullpath)) {

                    // Vivaldi Found (one user)
                    sprintf(win->browser_path, "\"%s\"", browser_fullpath);
                    VivaldiExist = true;
                    return true;
                }
            }

            return false;

        #elif __APPLE__

            // Vivaldi on macOS
            if(_webinix_cmd_sync("open -R -a \"Vivaldi\"", false) == 0) {

                sprintf(win->browser_path, "open -W \"/Applications/Vivaldi.app\" --args");
                VivaldiExist = true;
                return true;
            }
            else
                return false;
        #else

            // Vivaldi on Linux
            if(_webinix_cmd_sync("vivaldi --version", false) == 0) {

                sprintf(win->browser_path, "vivaldi");
                VivaldiExist = true;
                return true;
            }
            else
                return false;
        #endif
    }
    else if(browser == Brave) {

        // Brave Browser

        static bool BraveExist = false;
        if(BraveExist && !_webinix_is_empty(win->browser_path)) return true;

        #ifdef _WIN32

            // Brave on Windows

            char browser_fullpath[WEBUI_MAX_PATH];

            // Search in `HKEY_LOCAL_MACHINE` (If Brave installed for multi-user)
            if(_webinix_get_windows_reg_value(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\brave.exe", L"", browser_fullpath)) {

                // Make sure the browser executable file exist
                if(_webinix_file_exist(browser_fullpath)) {

                    // Brave Found (multi-user)
                    sprintf(win->browser_path, "\"%s\"", browser_fullpath);
                    BraveExist = true;
                    return true;
                }
            }

            // Search in `HKEY_CURRENT_USER` (If Brave installed for one user)
            if(_webinix_get_windows_reg_value(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\brave.exe", L"", browser_fullpath)) {

                // Make sure the browser executable file exist
                if(_webinix_file_exist(browser_fullpath)) {

                    // Brave Found (one user)
                    sprintf(win->browser_path, "\"%s\"", browser_fullpath);
                    BraveExist = true;
                    return true;
                }
            }

            return false;

        #elif __APPLE__

            // Brave on macOS
            if(_webinix_cmd_sync("open -R -a \"Brave\"", false) == 0) {

                sprintf(win->browser_path, "open -W \"/Applications/Brave.app\" --args");
                BraveExist = true;
                return true;
            }
            else
                return false;
        #else

            // Brave on Linux
            if(_webinix_cmd_sync("brave --version", false) == 0) {

                sprintf(win->browser_path, "brave");
                BraveExist = true;
                return true;
            }
            else
                return false;
        #endif
    }
    else if(browser == Firefox) {

        // Firefox

        static bool FirefoxExist = false;
        if(FirefoxExist && !_webinix_is_empty(win->browser_path)) return true;
        
        #ifdef _WIN32
        
            // Firefox on Windows

            char browser_fullpath[WEBUI_MAX_PATH];

            // Search in `HKEY_LOCAL_MACHINE` (If Firefox installed for multi-user)
            if(_webinix_get_windows_reg_value(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\firefox.exe", L"", browser_fullpath)) {

                // Make sure the browser executable file exist
                if(_webinix_file_exist(browser_fullpath)) {

                    // Firefox Found (multi-user)
                    sprintf(win->browser_path, "\"%s\"", browser_fullpath);
                    FirefoxExist = true;
                    return true;
                }
            }

            // Search in `HKEY_CURRENT_USER` (If Firefox installed for one user)
            if(_webinix_get_windows_reg_value(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\firefox.exe", L"", browser_fullpath)) {

                // Make sure the browser executable file exist
                if(_webinix_file_exist(browser_fullpath)) {

                    // Firefox Found (one user)
                    sprintf(win->browser_path, "\"%s\"", browser_fullpath);
                    FirefoxExist = true;
                    return true;
                }
            }

            return false;

        #elif __APPLE__
            
            // Firefox on macOS
            if(_webinix_cmd_sync("open -R -a \"firefox\"", false) == 0) {

                sprintf(win->browser_path, "open -W \"/Applications/Firefox.app\" --args");
                FirefoxExist = true;
                return true;
            }
            else
                return false;
        #else

            // Firefox on Linux

            if(_webinix_cmd_sync("firefox -v", false) == 0) {

                sprintf(win->browser_path, "firefox");
                FirefoxExist = true;
                return true;
            }
            else
                return false;

        #endif

    }
    else if(browser == Yandex) {

        // Yandex Browser

        static bool YandexExist = false;
        if(YandexExist && !_webinix_is_empty(win->browser_path)) return true;

        #ifdef _WIN32

            // Yandex on Windows

            char browser_fullpath[WEBUI_MAX_PATH];

            // Search in `HKEY_CURRENT_USER` (If Yandex installed for one user)
            if(_webinix_get_windows_reg_value(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\browser.exe", L"", browser_fullpath)) {

                // Make sure the browser executable file exist
                if(_webinix_file_exist(browser_fullpath)) {

                    // Yandex Found (one user)
                    sprintf(win->browser_path, "\"%s\"", browser_fullpath);
                    YandexExist = true;
                    return true;
                }
            }

            // Search in `HKEY_LOCAL_MACHINE` (If Yandex installed for multi-user)
            if(_webinix_get_windows_reg_value(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\browser.exe", L"", browser_fullpath)) {

                // Make sure the browser executable file exist
                if(_webinix_file_exist(browser_fullpath)) {

                    // Yandex Found (multi-user)
                    sprintf(win->browser_path, "\"%s\"", browser_fullpath);
                    YandexExist = true;
                    return true;
                }
            }

            return false;

        #elif __APPLE__

            // Yandex on macOS
            if(_webinix_cmd_sync("open -R -a \"Yandex\"", false) == 0) {

                sprintf(win->browser_path, "open -W \"/Applications/Yandex.app\" --args");
                YandexExist = true;
                return true;
            }
            else
                return false;
        #else

            // Yandex on Linux
            if(_webinix_cmd_sync("yandex-browser --version", false) == 0) {

                sprintf(win->browser_path, "yandex-browser");
                YandexExist = true;
                return true;
            }
            else
                return false;
        #endif
    }
    else if(browser == Chromium) {

        // The Chromium Projects

        static bool ChromiumExist = false;
        if(ChromiumExist && !_webinix_is_empty(win->browser_path)) return true;

        #ifdef _WIN32

            // Chromium on Windows

            char browser_folder[WEBUI_MAX_PATH];

            // Search in `HKEY_CURRENT_USER` (If Chromium installed for one user)
            if(_webinix_get_windows_reg_value(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\chrome.exe", L"Path", browser_folder)) {

                // Make sure its Chromium and not Google Chrome
                if(!_webinix_is_google_chrome_folder(browser_folder)) {

                    // Chromium Found (one user)
                    sprintf(win->browser_path, "\"%s\\chrome.exe\"", browser_folder);
                    ChromiumExist = true;
                    return true;
                }
            }

            // Search in `HKEY_LOCAL_MACHINE` (If Chromium installed for multi-user)
            if(_webinix_get_windows_reg_value(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\chrome.exe", L"Path", browser_folder)) {

                // Make sure its Chromium and not Google Chrome
                if(!_webinix_is_google_chrome_folder(browser_folder)) {

                    // Chromium Found (multi-user)
                    sprintf(win->browser_path, "\"%s\\chrome.exe\"", browser_folder);
                    ChromiumExist = true;
                    return true;
                }
            }

            return false;

        #elif __APPLE__

            // Chromium on macOS
            if(_webinix_cmd_sync("open -R -a \"Chromium\"", false) == 0) {

                sprintf(win->browser_path, "open -W \"/Applications/Chromium.app\" --args");
                ChromiumExist = true;
                return true;
            }
            else
                return false;
        #else

            // Chromium on Linux
            if(_webinix_cmd_sync("chromium-browser --version", false) == 0) {

                sprintf(win->browser_path, "chromium-browser");
                ChromiumExist = true;
                return true;
            }
            else if(_webinix_cmd_sync("chromium --version", false) == 0) {

                sprintf(win->browser_path, "chromium");
                ChromiumExist = true;
                return true;
            }
            else
                return false;
        #endif
    }

    return false;
}

static void _webinix_clean(void) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_clean()... \n");
    #endif

    static bool cleaned = false;
    if(cleaned) return;
    cleaned = true;

    // Let's give other threads more time to safely exit
    // and finish their cleaning up.
    _webinix_sleep(120);

    // TODO: Add option to let the user decide if
    // Webinix should delete the web browser profile
    // folder or not.

    // Free all non-freed memory allocations
    _webinix_free_all_mem();
}

int _webinix_cmd_sync(char* cmd, bool show) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_cmd_sync()... \n");
    #endif

    // Run sync command and
    // return the exit code

    char buf[1024];

    #ifdef _WIN32
        sprintf(buf, "cmd /c \"%s\" > nul 2>&1", cmd);
        #ifdef WEBUI_LOG
            printf("[Core]\t\t_webinix_cmd_sync() -> Running [%s] \n", buf);
        #endif
        return _webinix_system_win32(buf, show);
    #else
        sprintf(buf, "%s >>/dev/null 2>>/dev/null ", cmd);
        #ifdef WEBUI_LOG
            printf("[Core]\t\t_webinix_cmd_sync() -> Running [%s] \n", buf);
        #endif
        int r =  system(buf);
        r = (r != -1 && r != 127 && WIFEXITED(r)) ? WEXITSTATUS(r) : -1;
        return r;
    #endif
}

int _webinix_cmd_async(char* cmd, bool show) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_cmd_async()... \n");
    #endif

    // Run a async command
    // and return immediately

    char buf[1024];
    int res = 0;

    // Asynchronous command
    #ifdef _WIN32
        sprintf(buf, "START \"\" %s", cmd);
        res = _webinix_cmd_sync(buf, show);
    #else
        sprintf(buf, "%s >>/dev/null 2>>/dev/null &", cmd);
        res = _webinix_cmd_sync(buf, show);
    #endif

    return res;
}

int _webinix_run_browser(_webinix_window_t* win, char* cmd) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_run_browser()... \n");
    #endif

    // Run a async command
    return _webinix_cmd_async(cmd, false);
}

bool _webinix_browser_start_chrome(_webinix_window_t* win, const char* address) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_browser_start_chrome([%s])... \n", address);
    #endif
    
    // -- Google Chrome ----------------------

    if(win->current_browser != 0 && win->current_browser != Chrome)
        return false;

    if(!_webinix_browser_exist(win, Chrome))
        return false;
    
    if(!_webinix_browser_create_profile_folder(win, Chrome))
        return false;
    
    char arg[1024];

    char kiosk_arg[24] = "--app=";
    if(win->kiosk_mode)
        sprintf(kiosk_arg, "--chrome-frame --kiosk ");

    sprintf(arg, " --user-data-dir=\"%s\" --no-first-run --disable-gpu --disable-software-rasterizer --no-proxy-server --safe-mode --disable-extensions --disable-background-mode --disable-plugins --disable-plugins-discovery --disable-translate --bwsi --disable-sync --disable-sync-preferences %s", win->profile_path, kiosk_arg);

    char full[1024];
    sprintf(full, "%s%s\"%s\"", win->browser_path, arg, address);

    if(_webinix_run_browser(win, full) == 0) {

        win->current_browser = Chrome;
        _webinix_core.current_browser = Chrome;
        return true;
    }
    else
        return false;
}

bool _webinix_browser_start_edge(_webinix_window_t* win, const char* address) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_browser_start_edge([%s])... \n", address);
    #endif

    // -- Microsoft Edge ----------------------

    if(win->current_browser != 0 && win->current_browser != Edge)
        return false;

    if(!_webinix_browser_exist(win, Edge))
        return false;
    
    if(!_webinix_browser_create_profile_folder(win, Edge))
        return false;

    char arg[1024];
    
    char kiosk_arg[24] = "--app=";
    if(win->kiosk_mode)
        sprintf(kiosk_arg, "--chrome-frame --kiosk ");

    sprintf(arg, " --user-data-dir=\"%s\" --no-first-run --disable-gpu --disable-software-rasterizer --no-proxy-server --safe-mode --disable-extensions --disable-background-mode --disable-plugins --disable-plugins-discovery --disable-translate --bwsi --disable-sync --disable-sync-preferences %s", win->profile_path, kiosk_arg);

    char full[1024];
    sprintf(full, "%s%s\"%s\"", win->browser_path, arg, address);

    if(_webinix_run_browser(win, full) == 0) {

        win->current_browser = Edge;
        _webinix_core.current_browser = Edge;
        return true;
    }
    else
        return false;
}

bool _webinix_browser_start_epic(_webinix_window_t* win, const char* address) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_browser_start_epic([%s])... \n", address);
    #endif

    // -- Epic Privacy Browser ----------------------

    if(win->current_browser != 0 && win->current_browser != Epic)
        return false;

    if(!_webinix_browser_exist(win, Epic))
        return false;
    
    if(!_webinix_browser_create_profile_folder(win, Epic))
        return false;

    char arg[1024];

    char kiosk_arg[24] = "--app=";
    if(win->kiosk_mode)
        sprintf(kiosk_arg, "--chrome-frame --kiosk ");

    sprintf(arg, " --user-data-dir=\"%s\" --no-first-run --disable-gpu --disable-software-rasterizer --no-proxy-server --safe-mode --disable-extensions --disable-background-mode --disable-plugins --disable-plugins-discovery --disable-translate --bwsi --disable-sync --disable-sync-preferences %s", win->profile_path, kiosk_arg);

    char full[1024];
    sprintf(full, "%s%s\"%s\"", win->browser_path, arg, address);

    if(_webinix_run_browser(win, full) == 0) {

        win->current_browser = Epic;
        _webinix_core.current_browser = Epic;
        return true;
    }
    else
        return false;
}

bool _webinix_browser_start_vivaldi(_webinix_window_t* win, const char* address) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_browser_start_vivaldi([%s])... \n", address);
    #endif

    // -- Vivaldi Browser ----------------------

    if(win->current_browser != 0 && win->current_browser != Vivaldi)
        return false;

    if(!_webinix_browser_exist(win, Vivaldi))
        return false;
    
    if(!_webinix_browser_create_profile_folder(win, Vivaldi))
        return false;

    char arg[1024];

    char kiosk_arg[24] = "--app=";
    if(win->kiosk_mode)
        sprintf(kiosk_arg, "--chrome-frame --kiosk ");

    sprintf(arg, " --user-data-dir=\"%s\" --no-first-run --disable-gpu --disable-software-rasterizer --no-proxy-server --safe-mode --disable-extensions --disable-background-mode --disable-plugins --disable-plugins-discovery --disable-translate --bwsi --disable-sync --disable-sync-preferences %s", win->profile_path, kiosk_arg);

    char full[1024];
    sprintf(full, "%s%s\"%s\"", win->browser_path, arg, address);

    if(_webinix_run_browser(win, full) == 0) {

        win->current_browser = Vivaldi;
        _webinix_core.current_browser = Vivaldi;
        return true;
    }
    else
        return false;
}

bool _webinix_browser_start_brave(_webinix_window_t* win, const char* address) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_browser_start_brave([%s])... \n", address);
    #endif

    // -- Brave Browser ----------------------

    if(win->current_browser != 0 && win->current_browser != Brave)
        return false;

    if(!_webinix_browser_exist(win, Brave))
        return false;
    
    if(!_webinix_browser_create_profile_folder(win, Brave))
        return false;

    char arg[1024];

    char kiosk_arg[24] = "--app=";
    if(win->kiosk_mode)
        sprintf(kiosk_arg, "--chrome-frame --kiosk ");

    sprintf(arg, " --user-data-dir=\"%s\" --no-first-run --disable-gpu --disable-software-rasterizer --no-proxy-server --safe-mode --disable-extensions --disable-background-mode --disable-plugins --disable-plugins-discovery --disable-translate --bwsi --disable-sync --disable-sync-preferences %s", win->profile_path, kiosk_arg);

    char full[1024];
    sprintf(full, "%s%s\"%s\"", win->browser_path, arg, address);

    if(_webinix_run_browser(win, full) == 0) {

        win->current_browser = Brave;
        _webinix_core.current_browser = Brave;
        return true;
    }
    else
        return false;
}

bool _webinix_browser_start_firefox(_webinix_window_t* win, const char* address) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_browser_start_firefox([%s])... \n", address);
    #endif

    // -- Mozilla Firefox ----------------------

    if(win->current_browser != 0 && win->current_browser != Firefox)
        return false;

    if(!_webinix_browser_exist(win, Firefox))
        return false;

    if(!_webinix_browser_create_profile_folder(win, Firefox))
        return false;

    char full[1024];

    char kiosk_arg[16] = {0};
    if(win->kiosk_mode)
        sprintf(kiosk_arg, "-kiosk");
    
    sprintf(full, "%s -P Webinix -purgecaches -new-window %s %s", win->browser_path, address, kiosk_arg);

    if(_webinix_run_browser(win, full) == 0) {

        win->current_browser = Firefox;
        _webinix_core.current_browser = Firefox;
        return true;
    }
    else
        return false;
}

bool _webinix_browser_start_yandex(_webinix_window_t* win, const char* address) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_browser_start_yandex([%s])... \n", address);
    #endif

    // -- Yandex Browser ----------------------

    if(win->current_browser != 0 && win->current_browser != Yandex)
        return false;

    if(!_webinix_browser_exist(win, Yandex))
        return false;
    
    if(!_webinix_browser_create_profile_folder(win, Yandex))
        return false;

    char arg[1024];

    char kiosk_arg[24] = "--app=";
    if(win->kiosk_mode)
        sprintf(kiosk_arg, "--chrome-frame --kiosk ");

    sprintf(arg, " --user-data-dir=\"%s\" --no-first-run --disable-gpu --disable-software-rasterizer --no-proxy-server --safe-mode --disable-extensions --disable-background-mode --disable-plugins --disable-plugins-discovery --disable-translate --bwsi --disable-sync --disable-sync-preferences %s", win->profile_path, kiosk_arg);

    char full[1024];
    sprintf(full, "%s%s\"%s\"", win->browser_path, arg, address);

    if(_webinix_run_browser(win, full) == 0) {

        win->current_browser = Yandex;
        _webinix_core.current_browser = Yandex;
        return true;
    }
    else
        return false;
}

bool _webinix_browser_start_chromium(_webinix_window_t* win, const char* address) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_browser_start_chromium([%s])... \n", address);
    #endif
    
    // -- The Chromium Projects -------------------

    if (win->current_browser != 0 && win->current_browser != Chromium)
        return false;

    if (!_webinix_browser_exist(win, Chromium))
        return false;
    
    if (!_webinix_browser_create_profile_folder(win, Chromium))
        return false;
    
    char arg[1024];

    char kiosk_arg[24] = "--app=";
    if(win->kiosk_mode)
        sprintf(kiosk_arg, "--chrome-frame --kiosk ");

    sprintf(arg, " --user-data-dir=\"%s\" --no-first-run --disable-gpu --disable-software-rasterizer --no-proxy-server --safe-mode --disable-extensions --disable-background-mode --disable-plugins --disable-plugins-discovery --disable-translate --bwsi --disable-sync --disable-sync-preferences %s", win->profile_path, kiosk_arg);

    char full[1024];
    sprintf(full, "%s%s\"%s\"", win->browser_path, arg, address);

    if (_webinix_run_browser(win, full) == 0) {

        win->current_browser = Chromium;
        _webinix_core.current_browser = Chromium;
        return true;
    }
    else
        return false;
}

bool _webinix_browser_start(_webinix_window_t* win, const char* address, unsigned int browser) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_browser_start([%s], [%u])... \n", address, browser);
    #endif

    // Non existing browser
    if(browser > 10)
        return false;
    
    // Current browser used in the last opened window
    if(browser == AnyBrowser && _webinix_core.current_browser != 0)
        browser = _webinix_core.current_browser;

    // TODO: Convert address from [/...] to [file://...]

    if(browser != 0) {

        // Open the window using the user specified browser

        if(browser == Chrome)
            return _webinix_browser_start_chrome(win, address);
        else if(browser == Edge)
            return _webinix_browser_start_edge(win, address);
        else if(browser == Epic)
            return _webinix_browser_start_epic(win, address);
        else if(browser == Vivaldi)
            return _webinix_browser_start_vivaldi(win, address);
        else if(browser == Brave)
            return _webinix_browser_start_brave(win, address);
        else if(browser == Firefox)
            return _webinix_browser_start_firefox(win, address);
        else if(browser == Yandex)
            return _webinix_browser_start_yandex(win, address);
        else if(browser == Chromium)
            return _webinix_browser_start_chromium(win, address);
        else
            return false;
    }
    else if(win->current_browser != 0) {

        // Open the window using the same web browser used for this current window

        if(win->current_browser == Chrome)
            return _webinix_browser_start_chrome(win, address);
        else if(win->current_browser == Edge)
            return _webinix_browser_start_edge(win, address);
        else if(win->current_browser == Epic)
            return _webinix_browser_start_epic(win, address);
        else if(win->current_browser == Vivaldi)
            return _webinix_browser_start_vivaldi(win, address);
        else if(win->current_browser == Brave)
            return _webinix_browser_start_brave(win, address);
        else if(win->current_browser == Firefox)
            return _webinix_browser_start_firefox(win, address);
        else if(win->current_browser == Yandex)
            return _webinix_browser_start_yandex(win, address);
        else if(browser == Chromium)
            return _webinix_browser_start_chromium(win, address);
        else
            return false;
    }
    else {

        // Open the window using the default OS browser

        // #1 - Chrome - Works perfectly
        // #2 - Edge - Works perfectly like Chrome
        // #3 - Epic - Works perfectly like Chrome
        // #4 - Vivaldi - Works perfectly like Chrome
        // #5 - Brave - Shows a policy notification in the first run
        // #6 - Firefox - Does not support App-Mode like Chrome (Looks not great)
        // #7 - Yandex - Shows a big welcome window in the first run
        // #8 - Chromium - Some Anti-Malware shows a false alert when using ungoogled-chromium-binaries

        #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
            // Windows
            if(!_webinix_browser_start_chrome(win, address))
                if(!_webinix_browser_start_edge(win, address))
                    if(!_webinix_browser_start_epic(win, address))
                        if(!_webinix_browser_start_vivaldi(win, address))
                            if(!_webinix_browser_start_brave(win, address))
                                if(!_webinix_browser_start_firefox(win, address))
                                    if(!_webinix_browser_start_yandex(win, address))
                                        if(!_webinix_browser_start_chromium(win, address))
                                            return false;
        #elif __APPLE__
            // macOS
            if(!_webinix_browser_start_chrome(win, address))
                if(!_webinix_browser_start_edge(win, address))
                    if(!_webinix_browser_start_epic(win, address))
                        if(!_webinix_browser_start_vivaldi(win, address))
                            if(!_webinix_browser_start_brave(win, address))
                                if(!_webinix_browser_start_firefox(win, address))
                                    if(!_webinix_browser_start_yandex(win, address))
                                        if(!_webinix_browser_start_chromium(win, address))
                                            return false;
        #else
            // Linux
            if(!_webinix_browser_start_chrome(win, address))
                if(!_webinix_browser_start_edge(win, address))
                    if(!_webinix_browser_start_epic(win, address))
                        if(!_webinix_browser_start_vivaldi(win, address))
                            if(!_webinix_browser_start_brave(win, address))
                                if(!_webinix_browser_start_firefox(win, address))
                                    if(!_webinix_browser_start_yandex(win, address))
                                        if(!_webinix_browser_start_chromium(win, address))
                                            return false;
        #endif
    }

    return true;
}

bool _webinix_set_root_folder(_webinix_window_t* win, const char* path) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_set_root_folder([%s])... \n", path);
    #endif

    if((path == NULL) || (strlen(path) > WEBUI_MAX_PATH))
        return false;

    win->is_embedded_html = true;

    if(_webinix_is_empty(path))
        sprintf(win->server_root_path, "%s", WEBUI_DEFAULT_PATH);
    else
        sprintf(win->server_root_path, "%s", path);
    
    webinix_set_multi_access(win, true);

    return true;
}

bool _webinix_is_process_running(const char* process_name) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_is_process_running([%s])... \n", process_name);
    #endif

    bool isRunning = false;

    #ifdef _WIN32
        // Microsoft Windows
        #ifndef WEBUI_NO_TLHELPER32
            HANDLE hSnapshot;
            PROCESSENTRY32 pe32;
            hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if(hSnapshot == INVALID_HANDLE_VALUE)
                return false;
            pe32.dwSize = sizeof(PROCESSENTRY32);
            if(!Process32First(hSnapshot, &pe32)) {
                CloseHandle(hSnapshot);
                return false;
            }
            do {
                if (strcmp(pe32.szExeFile, process_name) == 0) {
                    isRunning = true;
                    break;
                }
            }
            while (Process32Next(hSnapshot, &pe32));
            CloseHandle(hSnapshot);
        #endif
    #elif __linux__
        // Linux
        DIR *dir;
        struct dirent *entry;
        char status_path[WEBUI_MAX_PATH];
        char line[WEBUI_MAX_PATH];
        dir = opendir("/proc");
        if (!dir)
            return false; // Unable to open /proc
        while ((entry = readdir(dir))) {
            if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) {
                snprintf(status_path, sizeof(status_path), "/proc/%s/status", entry->d_name);
                FILE *status_file = fopen(status_path, "r");
                if (status_file) {
                    while (fgets(line, sizeof(line), status_file)) {
                        if (strncmp(line, "Name:", 5) == 0) {
                            char proc_name[WEBUI_MAX_PATH];
                            sscanf(line, "Name: %s", proc_name);
                            if (strcmp(proc_name, process_name) == 0) {
                                isRunning = true;
                                fclose(status_file);
                                goto _close_dir;
                            }
                            break;
                        }
                    }
                    fclose(status_file);
                }
            }
        }
        _close_dir:
            closedir(dir);
    #else
        // macOS
        int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};
        struct kinfo_proc *procs = NULL;
        size_t size;
        if (sysctl(mib, 4, NULL, &size, NULL, 0) < 0)
            return false; // Failed to get process list size
        procs = (struct kinfo_proc *)malloc(size);
        if (!procs)
            return false; // Failed to allocate memory for process list
        if (sysctl(mib, 4, procs, &size, NULL, 0) < 0) {
            free(procs);
            return false; // Failed to get process list
        }
        size_t count = size / sizeof(struct kinfo_proc);
        for (size_t i = 0; i < count; i++) {
            if (strcmp(procs[i].kp_proc.p_comm, process_name) == 0) {
                isRunning = true;
                break;
            }
        }
        free(procs);
    #endif

    return isRunning;    
}

unsigned int _webinix_find_the_best_browser(_webinix_window_t* win) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_find_the_best_browser()... \n");
    #endif

    // #1 - Chrome - Works perfectly
    // #2 - Edge - Works perfectly like Chrome
    // #3 - Epic - Works perfectly like Chrome
    // #4 - Vivaldi - Works perfectly like Chrome
    // #5 - Brave - Shows a policy notification in the first run
    // #6 - Firefox - Does not support App-Mode like Chrome (Looks not great)
    // #7 - Yandex - Shows a big welcome window in the first run
    // #8 - Chromium - Some Anti-Malware shows a false alert when using ungoogled-chromium-binaries

    // To save memory, let's search if a web browser is already running

    #ifdef _WIN32
        // Microsoft Windows
        if(_webinix_is_process_running("chrome.exe") && _webinix_browser_exist(win, Chrome)) return Chrome;
        else if(_webinix_is_process_running("msedge.exe") && _webinix_browser_exist(win, Edge)) return Edge;
        else if(_webinix_is_process_running("epic.exe") && _webinix_browser_exist(win, Epic)) return Epic;
        else if(_webinix_is_process_running("vivaldi.exe") && _webinix_browser_exist(win, Vivaldi)) return Vivaldi;
        else if(_webinix_is_process_running("brave.exe") && _webinix_browser_exist(win, Brave)) return Brave;
        else if(_webinix_is_process_running("firefox.exe") && _webinix_browser_exist(win, Firefox)) return Firefox;
        else if(_webinix_is_process_running("browser.exe") && _webinix_browser_exist(win, Yandex)) return Yandex;
        // Chromium check is never reached if Google Chrome is installed
        // due to duplicate process name `chrome.exe`
        else if(_webinix_is_process_running("chrome.exe") && _webinix_browser_exist(win, Chromium)) return Chromium;
    #elif __linux__
        // Linux
        if(_webinix_is_process_running("chrome") && _webinix_browser_exist(win, Chrome)) return Chrome;
        else if(_webinix_is_process_running("msedge") && _webinix_browser_exist(win, Edge)) return Edge;
        // Epic...
        else if(_webinix_is_process_running("vivaldi-bin") && _webinix_browser_exist(win, Vivaldi)) return Vivaldi;
        else if(_webinix_is_process_running("brave") && _webinix_browser_exist(win, Brave)) return Brave;
        else if(_webinix_is_process_running("firefox") && _webinix_browser_exist(win, Firefox)) return Firefox;
        else if(_webinix_is_process_running("yandex_browser") && _webinix_browser_exist(win, Yandex)) return Yandex;
        // Chromium check is never reached if Google Chrome is installed
        // due to duplicate process name `chrome`
        else if(_webinix_is_process_running("chrome") && _webinix_browser_exist(win, Chromium)) return Chromium;
    #else
        // macOS
        if(_webinix_is_process_running("Google Chrome") && _webinix_browser_exist(win, Chrome)) return Chrome;
        else if(_webinix_is_process_running("Epic") && _webinix_browser_exist(win, Epic)) return Epic;
        else if(_webinix_is_process_running("Vivaldi") && _webinix_browser_exist(win, Vivaldi)) return Vivaldi;
        else if(_webinix_is_process_running("Brave") && _webinix_browser_exist(win, Brave)) return Brave;
        else if(_webinix_is_process_running("Firefox") && _webinix_browser_exist(win, Firefox)) return Firefox;
        else if(_webinix_is_process_running("Yandex") && _webinix_browser_exist(win, Yandex)) return Yandex;
        else if(_webinix_is_process_running("Chromium") && _webinix_browser_exist(win, Chromium)) return Chromium;
    #endif

    return AnyBrowser;
}

bool _webinix_show(_webinix_window_t* win, const char* content, unsigned int browser) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_show([%u])... \n", browser);
    #endif

    if(_webinix_is_empty(content))
        return false;

    // Some wrappers does not guarantee `content` to
    // stay valid, so, let's make our copy right now
    size_t content_len = strlen(content);
    const char* content_cpy = (const char*)_webinix_malloc(content_len);
    memcpy((char*)content_cpy, content, content_len);

    if(strstr(content_cpy, "<html")) {

        // Embedded HTML
        #ifdef WEBUI_LOG
            printf("[Core]\t\t_webinix_show() -> Embedded HTML:\n");
            printf("- - -[HTML]- - - - - - - - - -\n%s\n- - - - - - - - - - - - - - - -\n", content_cpy);
        #endif
        
        return _webinix_show_window(win, content_cpy, true, browser);
    }
    else {

        // File
        #ifdef WEBUI_LOG
            printf("[User] webinix_show() -> File: [%s]\n", content_cpy);
        #endif

        if(content_len > WEBUI_MAX_PATH || strstr(content_cpy, "<"))
            return false;
        
        return _webinix_show_window(win, content_cpy, false, browser);
    }
}

bool _webinix_show_window(_webinix_window_t* win, const char* content, bool is_embedded_html, unsigned int browser) {

    #ifdef WEBUI_LOG
        if(is_embedded_html)
            printf("[Core]\t\t_webinix_show_window(HTML, [%u])... \n", browser);
        else
            printf("[Core]\t\t_webinix_show_window(FILE, [%u])... \n", browser);
    #endif

    _webinix_init();

    char* url = NULL;
    unsigned int port = (win->server_port == 0 ? _webinix_get_free_port() : win->server_port);

    // Initialization
    if(win->html != NULL)
        _webinix_free_mem((void*)win->html);
    if(win->url != NULL)
        _webinix_free_mem((void*)win->url);

    if(is_embedded_html) {

        // Show a window using the embedded HTML
        win->is_embedded_html = true;
        win->html = (content == NULL ? webinix_empty_string : content);

        // Generate the URL
        size_t url_len = 32; // [http][domain][port]
        url = (char*) _webinix_malloc(url_len);
        sprintf(url, "http://localhost:%u", port);
    }
    else {

        // Show a window using a local file
        win->is_embedded_html = false;
        win->html = NULL;

        // Generate the URL
        size_t url_len = 32 + strlen(content); // [http][domain][port][file]
        url = (char*) _webinix_malloc(url_len);
        sprintf(url, "http://localhost:%u/%s", port, content);
    }

    // Set URL
    win->url = url;
    win->server_port = port;
    
    if(!webinix_is_shown(win)) {

        // Start a new window

        // Run browser
        if(!_webinix_browser_start(win, win->url, browser)) {

            // Browser not available
            _webinix_free_mem((void*)win->html);
            _webinix_free_mem((void*)win->url);
            _webinix_free_port(win->server_port);
            return false;
        }
        
        // New server thread
        #ifdef _WIN32
            HANDLE thread = CreateThread(NULL, 0, _webinix_server_start, (void*)win, 0, NULL);
            win->server_thread = thread;
            if(thread != NULL)
                CloseHandle(thread);
        #else
            pthread_t thread;
            pthread_create(&thread, NULL, &_webinix_server_start, (void*)win);
            pthread_detach(thread);
            win->server_thread = thread;
        #endif
    }
    else {

        // Refresh an existing running window

        // Prepare packets
        size_t packet_len = 3 + strlen(url); // [header][url]
        char* packet = (char*) _webinix_malloc(packet_len);
        packet[0] = WEBUI_HEADER_SIGNATURE; // Signature
        packet[1] = WEBUI_HEADER_SWITCH;    // Type
        packet[2] = 0;                      // ID
        for(unsigned int i = 0; i < strlen(win->url); i++) // URL
            packet[i + 3] = win->url[i];

        // Send the packet
        _webinix_window_send(win, packet, packet_len);
        _webinix_free_mem((void*)packet);
    }

    return true;
}

static void _webinix_window_event(_webinix_window_t* win, int event_type, char* element, char* data, unsigned int event_number, char* webinix_internal_id) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_window_event([%s], [%s])... \n", webinix_internal_id, element);
    #endif

    // Create a thread, and call the used cb function
    // no need to wait for the response. This is fire
    // and forget.

    // Create a new CB args struct
    _webinix_cb_arg_t* arg = (_webinix_cb_arg_t*) _webinix_malloc(sizeof(_webinix_cb_arg_t));
    
    // Event
    arg->window = win;
    arg->event_type = event_type;
    arg->element = element;
    arg->data = data;
    arg->event_number = event_number;
    // Extras
    arg->webinix_internal_id = webinix_internal_id;

    // fire and forget.
    #ifdef _WIN32
        HANDLE user_fun_thread = CreateThread(NULL, 0, _webinix_cb, (void*)arg, 0, NULL);
        if(user_fun_thread != NULL)
            CloseHandle(user_fun_thread); 
    #else
        pthread_t thread;
        pthread_create(&thread, NULL, &_webinix_cb, (void*)arg);
        pthread_detach(thread);
    #endif
}

static void _webinix_window_send(_webinix_window_t* win, char* packet, size_t packets_size) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_window_send()... \n");
        printf("[Core]\t\t_webinix_window_send() -> %d bytes \n", (int)packets_size);
        printf("[Core]\t\t_webinix_window_send() -> [ ");
            _webinix_print_hex(packet, 3);
        printf("]\n");
        printf("[Core]\t\t_webinix_window_send() -> [%.*s] \n", (int)(packets_size - 3), (const char*)&packet[3]);
    #endif
    
    if(!win->connected ||
        _webinix_core.mg_connections[win->window_number] == NULL ||
        packet == NULL ||
        packets_size < 4)
        return;

    struct mg_connection* c = _webinix_core.mg_connections[win->window_number];
    mg_ws_send(
        c, 
        packet, 
        packets_size, 
        WEBSOCKET_OP_BINARY
    );
}

bool _webinix_get_data(const char* packet, size_t packet_len, unsigned int pos, size_t* data_len, char** data) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_get_data()... \n");
    #endif

    if((pos + 1) > packet_len) {

        *data = (char*)webinix_empty_string;
        *data_len = 0;
        return false;
    }

    // Calculat the data part size
    size_t data_size = strlen(&packet[pos]);
    if(data_size < 1) {

        *data = (char*)webinix_empty_string;
        *data_len = 0;
        return false;
    }

    // Allocat mem
    *data = (char*) _webinix_malloc(data_size);

    // Copy data part
    char* p = *data;
    unsigned int j = pos;
    for(unsigned int i = 0; i < data_size; i++) {

        memcpy(p, &packet[j], 1);
        p++;
        j++;
    }

    // Check data size
    *data_len = strlen(*data);
    if(*data_len < 1) {

        _webinix_free_mem((void*)data);
        *data = (char*)webinix_empty_string;
        *data_len = 0;
        return false;
    }

    return true;
}

static void _webinix_window_receive(_webinix_window_t* win, const char* packet, size_t len) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_window_receive()... \n");
    #endif

    if((unsigned char) packet[0] != WEBUI_HEADER_SIGNATURE || len < 4)
        return;

    if((unsigned char) packet[1] == WEBUI_HEADER_CLICK) {

        // Click Event

        // 0: [Signature]
        // 1: [Type]
        // 2: 
        // 3: [Data]

        // Get html element id
        char* element;
        size_t element_len;
        if(!_webinix_get_data(packet, len, 3, &element_len, &element))
            return;
        
        #ifdef WEBUI_LOG
            printf("[Core]\t\t_webinix_window_receive() -> WEBUI_HEADER_CLICK \n");
            printf("[Core]\t\t_webinix_window_receive() -> %d bytes \n", (int)element_len);
            printf("[Core]\t\t_webinix_window_receive() -> [%s] \n", element);
        #endif

        // Generate Webinix internal id
        char* webinix_internal_id = _webinix_generate_internal_id(win, element);

        _webinix_window_event(
            win,                        // Event -> Window
            WEBUI_EVENT_MOUSE_CLICK,    // Event -> Type of this event
            element,                    // Event -> HTML Element
            NULL,                       // Event -> User Custom Data
            0,                          // Event -> Event Number
            webinix_internal_id           // Extras -> Webinix Internal ID
        );
    }
    else if((unsigned char) packet[1] == WEBUI_HEADER_JS) {

        // JS Result

        // 0: [Signature]
        // 1: [Type]
        // 2: [ID]
        // 3: [Error]
        // 4: [Data]

        // Get pipe id
        unsigned char run_id = packet[2];
        if(run_id < 0x01) {

            // Fatal.
            // The pipe ID is not valid
            // we can't send the ready signal to webinix_script()
            return;
        }

        // Get data part
        char* data;
        size_t data_len;
        bool data_status = _webinix_get_data(packet, len, 4, &data_len, &data);

        // Get js-error
        bool error = true;
        if((unsigned char) packet[3] == 0x00)
            error = false;

        #ifdef WEBUI_LOG
            printf("[Core]\t\t_webinix_window_receive() -> WEBUI_HEADER_JS \n");
            printf("[Core]\t\t_webinix_window_receive() -> run_id = 0x%02x \n", run_id);
            printf("[Core]\t\t_webinix_window_receive() -> error = 0x%02x \n", error);
            printf("[Core]\t\t_webinix_window_receive() -> %d bytes of data\n", (int)data_len);
            printf("[Core]\t\t_webinix_window_receive() -> data = [%s] @ 0x%p\n", data, data);
        #endif

        // Initialize pipe
        if((void*)_webinix_core.run_responses[run_id] != NULL)
            _webinix_free_mem((void*)_webinix_core.run_responses[run_id]);

        // Set pipe
        if(data_status && data_len > 0) {

            _webinix_core.run_error[run_id] = error;
            _webinix_core.run_responses[run_id] = data;
        }
        else {

            // Empty Result
            _webinix_core.run_error[run_id] = error;
            _webinix_core.run_responses[run_id] = webinix_empty_string;
        }

        // Send ready signal to webinix_script()
        _webinix_core.run_done[run_id] = true;
    }
    else if((unsigned char) packet[1] == WEBUI_HEADER_SWITCH) {

        // Navigation Event

        // 0: [Signature]
        // 1: [Type]
        // 2: 
        // 3: [URL]

        // Events
        if(win->has_events) {

            // Get URL
            char* url;
            size_t url_len;
            if(!_webinix_get_data(packet, len, 3, &url_len, &url))
                return;
            
            #ifdef WEBUI_LOG
                printf("[Core]\t\t_webinix_window_receive() -> WEBUI_HEADER_SWITCH \n");
                printf("[Core]\t\t_webinix_window_receive() -> %d bytes \n", (int)url_len);
                printf("[Core]\t\t_webinix_window_receive() -> [%s] \n", url);
            #endif

            // Generate Webinix internal id
            char* webinix_internal_id = _webinix_generate_internal_id(win, "");

            _webinix_window_event(
                win,                    // Event -> Window
                WEBUI_EVENT_NAVIGATION, // Event -> Type of this event
                "",                     // Event -> HTML Element
                url,                    // Event -> User Custom Data
                0,                      // Event -> Event Number
                webinix_internal_id       // Extras -> Webinix Internal ID
            );
        }
    }
    else if((unsigned char) packet[1] == WEBUI_HEADER_CALL_FUNC) {

        // Function Call

        // 0: [Signature]
        // 1: [Type]
        // 2: [Call ID]
        // 3: [Element ID, Null, Data]
        
        // Get html element id
        char* element;
        size_t element_len;
        if(!_webinix_get_data(packet, len, 3, &element_len, &element))
            return;

        // Get data
        char* data;
        size_t data_len;
        _webinix_get_data(packet, len, (3 + element_len + 1), &data_len, (char **) &data);
        
        #ifdef WEBUI_LOG
            printf("[Core]\t\t_webinix_window_receive() -> WEBUI_HEADER_CALL_FUNC \n");
            printf("[Core]\t\t_webinix_window_receive() -> Call ID: [0x%02x] \n", packet[2]);
            printf("[Core]\t\t_webinix_window_receive() -> Element: [%s] \n", element);
            printf("[Core]\t\t_webinix_window_receive() -> Data size: %llu Bytes \n", data_len);
            printf("[Core]\t\t_webinix_window_receive() -> Data: [%s] \n", data);
        #endif

        // Generate Webinix internal id
        char* webinix_internal_id = _webinix_generate_internal_id(win, element);

        // Create new event core to hold the response
        webinix_event_core_t* event_core = (webinix_event_core_t*) _webinix_malloc(sizeof(webinix_event_core_t));
        unsigned int event_core_pos = _webinix_get_free_event_core_pos(win);
        win->event_core[event_core_pos] = event_core;
        char** response = &win->event_core[event_core_pos]->response;

        // Create new event
        webinix_event_t e;
        e.window = win;
        e.event_type = WEBUI_EVENT_CALLBACK;
        e.element = element;
        e.data = data;
        e.event_number = event_core_pos;

        // Call user function
        unsigned int cb_index = _webinix_get_cb_index(webinix_internal_id);
        if(cb_index > 0 && _webinix_core.cb[cb_index] != NULL) {

            // Call user cb
            _webinix_core.cb[cb_index](&e);
        }

        // Check the response
        if(_webinix_is_empty(*response))
            *response = (char*)webinix_empty_string;

        #ifdef WEBUI_LOG
            printf("[Core]\t\t_webinix_window_receive()... user-callback response [%s]\n", *response);
        #endif

        // 0: [Signature]
        // 1: [Type]
        // 2: [Call ID]
        // 3: [Data]

        // Prepare response packet
        size_t response_len = strlen(*response);
        size_t response_packet_len = 3 + response_len + 1;
        char* response_packet = (char*) _webinix_malloc(response_packet_len);
        response_packet[0] = WEBUI_HEADER_SIGNATURE;    // Signature
        response_packet[1] = WEBUI_HEADER_CALL_FUNC;    // Type
        response_packet[2] = packet[2];                 // Call ID
        for(unsigned int i = 0; i < response_len; i++)  // Data
            response_packet[3 + i] = (*response)[i];

        // Send response packet
        _webinix_window_send(win, response_packet, response_packet_len);
        _webinix_free_mem((void*)response_packet);

        // Free
        _webinix_free_mem((void*)element);
        _webinix_free_mem((void*)data);
        _webinix_free_mem((void*)webinix_internal_id);
        _webinix_free_mem((void*)*response);
        _webinix_free_mem((void*)event_core);
    }
}

char* _webinix_get_current_path(void) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_get_current_path()... \n");
    #endif

    char* path = (char*) _webinix_malloc(WEBUI_MAX_PATH);
    if(WEBUI_GET_CURRENT_DIR (path, WEBUI_MAX_PATH) == NULL)
        path[0] = 0x00;

    return path;
}

static void _webinix_free_port(unsigned int port) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_free_port([%u])... \n", port);
    #endif

    for(unsigned int i = 0; i < WEBUI_MAX_ARRAY; i++) {
        if(_webinix_core.used_ports[i] == port) {
            _webinix_core.used_ports[i] = 0;
            break;
        }
    }
}

static void _webinix_wait_for_startup(void) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_wait_for_startup()... \n");
    #endif

    if(_webinix_core.connections > 0)
        return;

    // Wait for the first connection
    for(unsigned int n = 0; n <= (_webinix_core.startup_timeout * 10); n++) {

        if(_webinix_core.connections > 0)
            break;
        
        _webinix_sleep(50);
    }

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_wait_for_startup() -> Finish.\n");
    #endif
}

unsigned int _webinix_get_new_window_number(void) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_get_new_window_number()... \n");
    #endif

    return ++_webinix_core.last_window;
}

unsigned int _webinix_get_free_port(void) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_get_free_port()... \n");
    #endif

    #ifdef _WIN32
        srand((unsigned int)time(NULL));
    #else
        srand(time(NULL));
    #endif

    unsigned int port = (rand() % (WEBUI_MAX_PORT + 1 - WEBUI_MIN_PORT)) + WEBUI_MIN_PORT;

    for(unsigned int i = WEBUI_MIN_PORT; i <= WEBUI_MAX_PORT; i++) {

        // Search [port] in [_webinix_core.used_ports]
        bool found = false;
        for(unsigned int j = 0; j < WEBUI_MAX_ARRAY; j++) {
            if(_webinix_core.used_ports[j] == port) {
                found = true;
                break;
            }
        }

        if(found)
            // Port used by local window
            port = (rand() % (WEBUI_MAX_PORT + 1 - WEBUI_MIN_PORT)) + WEBUI_MIN_PORT;
        else {
            if(_webinix_port_is_used(port))
                // Port used by an external app
                port = (rand() % (WEBUI_MAX_PORT + 1 - WEBUI_MIN_PORT)) + WEBUI_MIN_PORT;
            else
                // Port is free
                break;
        }
    }

    // Add
    for(unsigned int i = 0; i < WEBUI_MAX_ARRAY; i++) {
        if(_webinix_core.used_ports[i] == 0) {
            _webinix_core.used_ports[i] = port;
            break;
        }
    }

    return port;
}

static void _webinix_init(void) {

    if(_webinix_core.initialized)
        return;    

    #ifdef WEBUI_LOG
        printf("[Core]\t\tWebinix v%s \n", WEBUI_VERSION);
        printf("[Core]\t\t_webinix_init()... \n");
    #endif

    // Initializing
    memset(&_webinix_core, 0, sizeof(_webinix_core_t));

    _webinix_core.initialized     = true;
    _webinix_core.startup_timeout = WEBUI_DEF_TIMEOUT;
    _webinix_core.executable_path = _webinix_get_current_path();
}

unsigned int _webinix_get_cb_index(char* webinix_internal_id) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_get_cb_index([%s])... \n", webinix_internal_id);
    #endif

    if(webinix_internal_id != NULL) {

        for(unsigned int i = 1; i < WEBUI_MAX_ARRAY; i++) {

            if(_webinix_core.html_elements[i] != NULL && !_webinix_is_empty(_webinix_core.html_elements[i])) 
                if(strcmp(_webinix_core.html_elements[i], webinix_internal_id) == 0)
                    return i;
        }
    }

    return 0;
}

unsigned int _webinix_set_cb_index(char* webinix_internal_id) {

    #ifdef WEBUI_LOG
        printf("[Core]\t\t_webinix_set_cb_index([%s])... \n", webinix_internal_id);
    #endif

    // Add
    for(unsigned int i = 1; i < WEBUI_MAX_ARRAY; i++) {

        if(_webinix_is_empty(_webinix_core.html_elements[i])) {

            _webinix_core.html_elements[i] = webinix_internal_id;

            return i;
        }
    }

    return 0;
}

#ifdef WEBUI_LOG
    static void _webinix_print_hex(const char* data, size_t len) {

        for(size_t i = 0; i < len; i++) {

            printf("0x%02X ", (unsigned char) *data);
            data++;
        }
    }
#endif

WEBUI_SERVER_START
{
    _webinix_window_t* win = (_webinix_window_t*) arg;
    
    #ifdef WEBUI_LOG
        printf("[Core]\t\t[Thread] _webinix_server_start(%s)... \n", win->url);
    #endif

    // Initialization
    _webinix_core.servers++;
    win->server_running = true;
    if(_webinix_core.startup_timeout < 1)
        _webinix_core.startup_timeout = 0;
    if(_webinix_core.startup_timeout > 30)
        _webinix_core.startup_timeout = 30;

    // Start Server
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    _webinix_core.mg_mgrs[win->window_number] = &mgr;

    if(mg_http_listen(&mgr, win->url, _webinix_server_event_handler, (void*)win) != NULL) {

        if(_webinix_core.startup_timeout > 0) {

            #ifdef WEBUI_LOG
                printf("[Core]\t\t[Thread] _webinix_server_start()... Listening Success\n");
                printf("[Core]\t\t[Thread] _webinix_server_start()... Timeout is %u seconds\n", _webinix_core.startup_timeout);
            #endif

            bool stop = false;

            while(!stop) {

                if(!win->connected) {

                    // Wait for first connection
                    _webinix_timer_t timer_1;
                    _webinix_timer_start(&timer_1);
                    for(;;) {

                        // Stop if window is connected
                        mg_mgr_poll(&mgr, 1);
                        if(win->connected)
                            break;

                        // Stop if timer is finished
                        if(_webinix_timer_is_end(&timer_1, (_webinix_core.startup_timeout * 1000)))
                            break;
                    }

                    if(!win->connected && win->html_handled) {

                        // At this moment the browser is already started and HTML
                        // is already handled, so, let's wait more time to give
                        // the WebSocket an extra one second to connect.
                        
                        _webinix_timer_t timer_2;
                        _webinix_timer_start(&timer_2);
                        for(;;) {

                            // Stop if window is connected
                            mg_mgr_poll(&mgr, 1);
                            if(win->connected)
                                break;

                            // Stop if timer is finished
                            if(_webinix_timer_is_end(&timer_2, 1000))
                                break;
                        }
                    }
                    
                    if(!win->connected)
                        stop = true; // First run failed
                }
                else {

                    // UI is connected

                    while(!stop) {

                        // Wait forever for disconnection

                        mg_mgr_poll(&mgr, 1);

                        // Exit signal
                        if(_webinix_core.exit_now) {
                            stop = true;
                            break;
                        }

                        if(!win->connected) {

                            // The UI is just get disconnected
                            // probably the user did a refresh
                            // let's wait for re-connection...

                            _webinix_timer_t timer_3;
                            _webinix_timer_start(&timer_3);
                            for(;;) {

                                // Stop if window is re-connected
                                mg_mgr_poll(&mgr, 1);
                                if(win->connected)
                                    break;

                                // Stop if timer is finished
                                if(_webinix_timer_is_end(&timer_3, 600))
                                    break;
                            }

                            if(!win->connected) {

                                stop = true;
                                break;
                            }
                        }
                    }
                }
            }
        }

        // Let's check the flag again, there is a change that
        // the flag has ben changed during the first loop for
        // example when set_timeout() get called later
        if(_webinix_core.startup_timeout == 0) {

            #ifdef WEBUI_LOG
                printf("[Core]\t\t[Thread] _webinix_server_start(%s)... Listening Success -> Infinite Loop... \n", win->url);
            #endif

            // Wait forever
            for(;;) {

                mg_mgr_poll(&mgr, 1);
                if(_webinix_core.exit_now)
                    break;
            }
        }
    }
    else {

        #ifdef WEBUI_LOG
            printf("[Core]\t\t[Thread] _webinix_server_start(%s)... Listening failed\n", win->url);
        #endif
    }

    // Stop server
    mg_mgr_free(&mgr);
    _webinix_core.servers--;

    #ifdef WEBUI_LOG
        printf("[Core]\t\t[Thread] _webinix_server_start()... Server Stop.\n");
    #endif

    // Clean
    win->server_running = false;
    win->html_handled = false;
    win->connected = false;
    _webinix_core.mg_mgrs[win->window_number] = NULL;
    _webinix_core.mg_connections[win->window_number] = NULL;
    _webinix_free_port(win->server_port);

    THREAD_RETURN
}

WEBUI_CB
{
    #ifdef WEBUI_LOG
        printf("[Core]\t\t[Thread] _webinix_cb()... \n");
    #endif

    _webinix_cb_arg_t* arg = (_webinix_cb_arg_t*) _arg;

    // Event
    webinix_event_t e;
    e.window = arg->window;
    e.event_type = arg->event_type;
    e.element = arg->element;
    e.data = arg->data;
    e.event_number = arg->event_number;

    // Check for all events-bind functions
    if(arg->window->has_events) {

        char* events_id = _webinix_generate_internal_id(arg->window, "");
        unsigned int events_cb_index = _webinix_get_cb_index(events_id);
        _webinix_free_mem((void*)events_id);

        if(events_cb_index > 0 && _webinix_core.cb[events_cb_index] != NULL) {

            // Call user all events cb
            _webinix_core.cb[events_cb_index](&e);
        }
    }

    // Check for the regular bind functions
    if(arg->element != NULL && !_webinix_is_empty(arg->element)) {

        unsigned int cb_index = _webinix_get_cb_index(arg->webinix_internal_id);
        if(cb_index > 0 && _webinix_core.cb[cb_index] != NULL) {

            // Call user cb
            _webinix_core.cb[cb_index](&e);
        }
    }

    #ifdef WEBUI_LOG
        printf("[Core]\t\t[Thread] _webinix_cb()... Stop.\n");
    #endif    

    // Free event
    _webinix_free_mem((void*)arg->element);
    _webinix_free_mem((void*)arg->data);
    // Free event extras
    _webinix_free_mem((void*)arg->webinix_internal_id);
    _webinix_free_mem((void*)arg);

    THREAD_RETURN
}

#ifdef _WIN32

    bool _webinix_socket_test_listen_win32(unsigned int port_num) {
    
        #ifdef WEBUI_LOG
            printf("[Core]\t\t_webinix_socket_test_listen_win32([%u])... \n", port_num);
        #endif

        WSADATA wsaData;
        unsigned int iResult;
        SOCKET ListenSocket = INVALID_SOCKET;
        struct addrinfo *result = NULL;
        struct addrinfo hints;

        // Initialize Winsock
        iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
        if(iResult != 0) {
            // WSAStartup failed
            return false;
        }
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        // Resolve the server address and port
        char the_port[16];
        sprintf(&the_port[0], "%u", port_num);
        iResult = getaddrinfo("127.0.0.1", &the_port[0], &hints, &result);
        if(iResult != 0) {
            // WSACleanup();
            return false;
        }

        // Create a SOCKET for the server to listen for client connections.
        ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if(ListenSocket == INVALID_SOCKET) {
            freeaddrinfo(result);
            // WSACleanup();
            return false;
        }

        // Setup the TCP listening socket
        iResult = bind(ListenSocket, result->ai_addr, (unsigned int)result->ai_addrlen);
        if(iResult == SOCKET_ERROR) {
            freeaddrinfo(result);
            closesocket(ListenSocket);
            shutdown(ListenSocket, SD_BOTH);
            // WSACleanup();
            return false;
        }

        // Clean
        freeaddrinfo(result);
        closesocket(ListenSocket);
        shutdown(ListenSocket, SD_BOTH);
        // WSACleanup();

        // Listening Success
        return true;
    }

    int _webinix_system_win32_out(const char *cmd, char **output, bool show) {

        #ifdef WEBUI_LOG
            printf("[Core]\t\t_webinix_system_win32_out()... \n");
        #endif

        // Ini
        *output = NULL;
        if(cmd == NULL)
            return -1;

        // Return
        DWORD Return = 0;
        
        // Flags
        DWORD CreationFlags = CREATE_NO_WINDOW;
        if(show)
            CreationFlags = SW_SHOW;

        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;
        HANDLE stdout_read, stdout_write;
        if (!CreatePipe(&stdout_read, &stdout_write, &sa, 0)) {
            return -1;
        }
        if (!SetHandleInformation(stdout_read, HANDLE_FLAG_INHERIT, 0)) {
            CloseHandle(stdout_read);
            CloseHandle(stdout_write);
            return -1;
        }

        STARTUPINFOA si;
        ZeroMemory(&si, sizeof(STARTUPINFOA));
        si.cb = sizeof(STARTUPINFOA);
        si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
        si.wShowWindow = SW_HIDE;
        si.hStdOutput = stdout_write;
        si.hStdError = stdout_write;

        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

        if (!CreateProcessA(
            NULL,           // No module name (use cmd line)
            (LPSTR)cmd,     // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            TRUE,           // Set handle inheritance to FALSE
            CreationFlags,  // Creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &si,            // Pointer to STARTUP INFO structure
            &pi))           // Pointer to PROCESS_INFORMATION structure
        {
            CloseHandle(stdout_read);
            CloseHandle(stdout_write);
            return -1;
        }
        CloseHandle(stdout_write);

        SetFocus(pi.hProcess);
        WaitForSingleObject(pi.hProcess, INFINITE);
        GetExitCodeProcess(pi.hProcess, &Return);

        DWORD bytes_read;
        char buffer[WEBUI_CMD_STDOUT_BUF];
        size_t output_size = 0;

        while (ReadFile(stdout_read, buffer, WEBUI_CMD_STDOUT_BUF, &bytes_read, NULL) && bytes_read > 0) {

            char *new_output = realloc(*output, output_size + bytes_read + 1);
            if (new_output == NULL) {
                free(*output);
                CloseHandle(stdout_read);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                return -1;
            }

            *output = new_output;
            memcpy(*output + output_size, buffer, bytes_read);
            output_size += bytes_read;
        }

        if (*output != NULL)
            (*output)[output_size] = '\0';

        CloseHandle(stdout_read);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        if(Return == 0)
            return 0;
        else
            return -1;
    }

    int _webinix_system_win32(char* cmd, bool show) {

        #ifdef WEBUI_LOG
            printf("[Core]\t\t_webinix_system_win32()... \n");
        #endif

        DWORD Return = 0;
        DWORD CreationFlags = CREATE_NO_WINDOW;

        /*
        We should not kill this process, because may had many child
        process of other Webinix app instances. Unfortunately, this is
        how modern browsers save memory by combine all windows into one
        single parent process, and we can't control this behavior.

        // Automatically close the browser process when the
        // parent process (this app) get closed. If this fail
        // webinix.js will try to close the window.
        HANDLE JobObject = CreateJobObject(NULL, NULL);
        JOB_OBJECT_EXTENDED_LIMIT_INFORMATION ExtendedInfo = { 0 };
        ExtendedInfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION | JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        SetInformationJobObject(JobObject, JobObjectExtendedLimitInformation, &ExtendedInfo, sizeof(ExtendedInfo));
        */

        if(show)
            CreationFlags = SW_SHOW;

        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        if(!CreateProcessA(
            NULL,               // No module name (use command line)
            cmd,                // Command line
            NULL,               // Process handle not inheritable
            NULL,               // Thread handle not inheritable
            FALSE,              // Set handle inheritance to FALSE
            CreationFlags,      // Creation flags
            NULL,               // Use parent's environment block
            NULL,               // Use parent's starting directory 
            &si,                // Pointer to STARTUP INFO structure
            &pi))               // Pointer to PROCESS_INFORMATION structure
        {
            // CreateProcess failed
            return -1;
        }

        SetFocus(pi.hProcess);
        // AssignProcessToJobObject(JobObject, pi.hProcess);
        WaitForSingleObject(pi.hProcess, INFINITE);
        GetExitCodeProcess(pi.hProcess, &Return);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        if(Return == 0)
            return 0;
        else
            return -1;
    }

    bool _webinix_get_windows_reg_value(HKEY key, LPCWSTR reg, LPCWSTR value_name, char value[WEBUI_MAX_PATH]) {

        #ifdef WEBUI_LOG
            printf("[Core]\t\t_webinix_get_windows_reg_value([%s])... \n", reg);
        #endif

        HKEY hKey;

        if(RegOpenKeyExW(key, reg, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {

            DWORD VALUE_TYPE;
            BYTE VALUE_DATA[WEBUI_MAX_PATH];
            DWORD VALUE_SIZE = sizeof(VALUE_DATA);

            // If `value_name` is empty then it will read the "(default)" reg-key
            if(RegQueryValueExW(hKey, value_name, NULL, &VALUE_TYPE, VALUE_DATA, &VALUE_SIZE) == ERROR_SUCCESS) {

                if(VALUE_TYPE == REG_SZ)
                    sprintf(value, "%S", (LPCWSTR)VALUE_DATA);
                else if(VALUE_TYPE == REG_DWORD)
                    sprintf(value, "%u", *((DWORD *)VALUE_DATA));
                
                RegCloseKey(hKey);
                return true;
            }
        }

        return false;
    }

    BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
        return true;
    }
    
#endif
