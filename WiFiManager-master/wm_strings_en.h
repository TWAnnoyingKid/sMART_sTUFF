/**
 * wm_strings_en.h
 * engligh strings for
 * WiFiManager, a library for the ESP8266/Arduino platform
 * for configuration of WiFi credentials using a Captive Portal
 *
 * @author Creator tzapu
 * @author tablatronix
 * @version 0.0.0
 * @license MIT
 */

#ifndef _WM_STRINGS_EN_H_
#define _WM_STRINGS_EN_H_


#ifndef WIFI_MANAGER_OVERRIDE_STRINGS
// !!! ABOVE WILL NOT WORK if you define in your sketch, must be build flag, if anyone one knows how to order includes to be able to do this it would be neat.. I have seen it done..

// strings files must include a consts file!
#include "wm_consts_en.h" // include constants, tokens, routes

const char WM_LANGUAGE[] PROGMEM = "en-US"; // i18n lang code

const char HTTP_HEAD_START[]       PROGMEM = "<!DOCTYPE html>"
"<html lang='en'><head>"
"<meta name='format-detection' content='telephone=no'>"
"<meta charset='UTF-8'>"
"<meta  name='viewport' content='width=device-width,initial-scale=1,user-scalable=no'/>"
"<title>{v}</title>";

const char HTTP_SCRIPT[]           PROGMEM = "<script>function c(l){"
"document.getElementById('s').value=l.getAttribute('data-ssid')||l.innerText||l.textContent;"
"p = l.nextElementSibling.classList.contains('l');"
"document.getElementById('p').disabled = !p;"
"if(p)document.getElementById('p').focus();};"
"function f() {var x = document.getElementById('p');x.type==='password'?x.type='text':x.type='password';}"
"</script>"; // @todo add button states, disable on click , show ack , spinner etc

const char HTTP_HEAD_END[]         PROGMEM = "</head><body bgcolor='#e9e1d4';class='{c}'><div class='wrap'>"; // {c} = _bodyclass
// example of embedded logo, base64 encoded inline, No styling here
// const char HTTP_ROOT_MAIN[]        PROGMEM = "<img title=' alt=' src='data:image/png;base64,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' /><h1>{v}</h1><h3>sMART sTUFF</h3>";
const char HTTP_ROOT_MAIN[]        PROGMEM = "<h1>sMART <img title=' alt=' src='data:image/png;base64,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' /> sTUFF</h1><h4 align='center'>懶人生活 智慧家居</h4>";

const char * const HTTP_PORTAL_MENU[] PROGMEM = {
"<form action='/wifi'    method='get'><button>網路設置</button></form><br/>\n", // MENU_WIFI
"<form action='/0wifi'   method='get'><button>網路設置 (無掃描)</button></form><br/>\n", // MENU_WIFINOSCAN
"<form action='/info'    method='get'></form><br/>\n", // MENU_INFO
"<form action='/param'   method='get'><button>Setup</button></form><br/>\n",//MENU_PARAM
"<form action='/close'   method='get'></form><br/>\n", // MENU_CLOSE
"<form action='/restart' method='get'><button color='#e9e1d4';>Restart</button></form><br/>\n",// MENU_RESTART
"<form action='/exit'    method='get'></form><br/>\n",  // MENU_EXIT
"<form action='/erase'   method='get'></form><br/>\n", // MENU_ERASE
"<form action='/update'  method='get'></form><br/>\n",// MENU_UPDATE
"<hr><br/>" // MENU_SEP
};

// const char HTTP_PORTAL_OPTIONS[]   PROGMEM = strcat(HTTP_PORTAL_MENU[0] , HTTP_PORTAL_MENU[3] , HTTP_PORTAL_MENU[7]);
const char HTTP_PORTAL_OPTIONS[]   PROGMEM = "";
const char HTTP_ITEM_QI[]          PROGMEM = "<div role='img' aria-label='{r}%' title='{r}%' class='q q-{q} {i} {h}'></div>"; // rssi icons
const char HTTP_ITEM_QP[]          PROGMEM = "<div class='q {h}'>{r}%</div>"; // rssi percentage {h} = hidden showperc pref
const char HTTP_ITEM[]             PROGMEM = "<div><a href='#p' onclick='c(this)' data-ssid='{V}'>{v}</a>{qi}{qp}</div>"; // {q} = HTTP_ITEM_QI, {r} = HTTP_ITEM_QP
// const char HTTP_ITEM[]            PROGMEM = "<div><a href='#p' onclick='c(this)'>{v}</a> {R} {r}% {q} {e}</div>"; // test all tokens

const char HTTP_FORM_START[]       PROGMEM = "<form method='POST' action='{v}'>";
const char HTTP_FORM_WIFI[]        PROGMEM = "<label for='s'>WIFI ID</label><input id='s' name='s' maxlength='32' autocorrect='off' autocapitalize='none' placeholder='{v}'><br/><label for='p'>密碼</label><input id='p' name='p' maxlength='64' type='password' placeholder='{p}'><input type='checkbox' onclick='f()'> 顯示密碼";
const char HTTP_FORM_WIFI_END[]    PROGMEM = "";
const char HTTP_FORM_STATIC_HEAD[] PROGMEM = "<hr><br/>";
const char HTTP_FORM_END[]         PROGMEM = "<br/><br/><button type='submit'>保存</button></form>";
const char HTTP_FORM_LABEL[]       PROGMEM = "<label for='{i}'>{t}</label>";
const char HTTP_FORM_PARAM_HEAD[]  PROGMEM = "<hr><br/>";
const char HTTP_FORM_PARAM[]       PROGMEM = "<br/><input id='{i}' name='{n}' maxlength='{l}' value='{v}' {c}>\n"; // do not remove newline!

const char HTTP_SCAN_LINK[]        PROGMEM = "<br/><form action='/wifi?refresh=1' method='POST'><button name='refresh' value='1'>刷新</button></form>";
const char HTTP_SAVED[]            PROGMEM = "<div class='msg'>儲存設置中...</div>";
const char HTTP_PARAMSAVED[]       PROGMEM = "<div class='msg S'>已保存<br/></div>";
const char HTTP_END[]              PROGMEM = "</div></body></html>";
const char HTTP_ERASEBTN[]         PROGMEM = "<br/><form action='/erase' method='get'><button class='D'>清除WiFi配置</button></form>";
const char HTTP_UPDATEBTN[]        PROGMEM = "<br/><form action='/update' method='get'><button>更新</button></form>";
const char HTTP_BACKBTN[]          PROGMEM = "<hr><br/><form action='/' method='get'><button>返回</button></form>";

const char HTTP_STATUS_ON[]        PROGMEM = "<div class='msg S'><strong>已連接</strong> to {v}<br/><em><small>with IP {i}</small></em></div>";
const char HTTP_STATUS_OFF[]       PROGMEM = "<div class='msg {c}'><strong>未連接</strong> to {v}{r}</div>"; // {c=class} {v=ssid} {r=status_off}
const char HTTP_STATUS_OFFPW[]     PROGMEM = "<br/>憑證錯誤"; // STATION_WRONG_PASSWORD,  no eps32
const char HTTP_STATUS_OFFNOAP[]   PROGMEM = "<br/>找不到AP";   // WL_NO_SSID_AVAIL
const char HTTP_STATUS_OFFFAIL[]   PROGMEM = "<br/>Could not connect"; // WL_CONNECT_FAILED
const char HTTP_STATUS_NONE[]      PROGMEM = "<div class='msg'></div>";
const char HTTP_BR[]               PROGMEM = "<br/>";

const char HTTP_STYLE[]            PROGMEM = "<style>"
".c,body{text-align:center;font-family:verdana}div,input,select{padding:5px;font-size:1em;margin:auto ;box-sizing:border-box}"
"input,button,select,.msg{border-radius:.3rem;width: 100%}input[type=radio],input[type=checkbox]{width:auto}"
"button,input[type='button'],input[type='submit']{cursor:pointer;border:0;background-color:#917b65;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%}"
"input[type='file']{border:1px solid #1fa3ec}"
".wrap {text-align:left;display:inline-block;min-width:260px;max-width:500px}"
// links
"a{color:#000;font-weight:700;text-decoration:none}a:hover{color:#1fa3ec;text-decoration:underline}"
// quality icons
".q{height:16px;margin:0;padding:0 5px;text-align:right;min-width:38px;float:right}.q.q-0:after{background-position-x:0}.q.q-1:after{background-position-x:-16px}.q.q-2:after{background-position-x:-32px}.q.q-3:after{background-position-x:-48px}.q.q-4:after{background-position-x:-64px}.q.l:before{background-position-x:-80px;padding-right:5px}.ql .q{float:left}.q:after,.q:before{content:'';width:16px;height:16px;display:inline-block;background-repeat:no-repeat;background-position: 16px 0;"
"background-image:url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAGAAAAAQCAMAAADeZIrLAAAAJFBMVEX///8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADHJj5lAAAAC3RSTlMAIjN3iJmqu8zd7vF8pzcAAABsSURBVHja7Y1BCsAwCASNSVo3/v+/BUEiXnIoXkoX5jAQMxTHzK9cVSnvDxwD8bFx8PhZ9q8FmghXBhqA1faxk92PsxvRc2CCCFdhQCbRkLoAQ3q/wWUBqG35ZxtVzW4Ed6LngPyBU2CobdIDQ5oPWI5nCUwAAAAASUVORK5CYII=');}"
// icons @2x media query (32px rescaled)
"@media (-webkit-min-device-pixel-ratio: 2),(min-resolution: 192dpi){.q:before,.q:after {"
"background-image:url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAALwAAAAgCAMAAACfM+KhAAAALVBMVEX///8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADAOrOgAAAADnRSTlMAESIzRGZ3iJmqu8zd7gKjCLQAAACmSURBVHgB7dDBCoMwEEXRmKlVY3L//3NLhyzqIqSUggy8uxnhCR5Mo8xLt+14aZ7wwgsvvPA/ofv9+44334UXXngvb6XsFhO/VoC2RsSv9J7x8BnYLW+AjT56ud/uePMdb7IP8Bsc/e7h8Cfk912ghsNXWPpDC4hvN+D1560A1QPORyh84VKLjjdvfPFm++i9EWq0348XXnjhhT+4dIbCW+WjZim9AKk4UZMnnCEuAAAAAElFTkSuQmCC');"
"background-size: 95px 16px;}}"
// msg callouts
".msg h4{margin-top:0;margin-bottom:5px;color='#e9e1d4'}.msg.P{border-left-color:#1fa3ec}.msg.P h4{color:#1fa3ec}.msg.D{border-left-color:#dc3630}.msg.D h4{color:#dc3630}.msg.S{border-left-color: #5cb85c}.msg.S h4{color: #5cb85c}"
// lists
"dt{font-weight:bold}dd{margin:0;padding:0 0 0.5em 0;min-height:12px}"
"td{vertical-align: top;}"
".h{display:none}"
"button{transition: 0s opacity;transition-delay: 3s;transition-duration: 0s;cursor: pointer}"
"button.D{background-color:#dc3630}"
"button:active{opacity:50% !important;cursor:wait;transition-delay: 0s}"
// invert
"body.invert,body.invert a,body.invert h1 {background-color:#060606;color:#fff;}"
"body.invert .msg{color:#fff;background-color:#282828;border-top:1px solid #555;border-right:1px solid #555;border-bottom:1px solid #555;}"
"body.invert .q[role=img]{-webkit-filter:invert(1);filter:invert(1);}"
":disabled {opacity: 0.5;}"
"</style>";

#ifndef WM_NOHELP
const char HTTP_HELP[]             PROGMEM =
 "<br/><h3>Available pages</h3><hr>"
 "<table class='table'>"
 "<thead><tr><th>Page</th><th>Function</th></tr></thead><tbody>"
 "<tr><td><a href='/'>/</a></td>"
 "<td>Menu page.</td></tr>"
 "<tr><td><a href='/wifi'>/wifi</a></td>"
 "<td>Show WiFi scan results and enter WiFi configuration.(/0wifi noscan)</td></tr>"
 "<tr><td><a href='/wifisave'>/wifisave</a></td>"
 "<td>Save WiFi configuration information and configure device. Needs variables supplied.</td></tr>"
 "<tr><td><a href='/param'>/param</a></td>"
 "<td>Parameter page</td></tr>"
 "<tr><td><a href='/info'>/info</a></td>"
 "<td>Information page</td></tr>"
 "<tr><td><a href='/u'>/u</a></td>"
 "<td>OTA Update</td></tr>"
 "<tr><td><a href='/close'>/close</a></td>"
 "<td>Close the captiveportal popup, config portal will remain active</td></tr>"
 "<tr><td>/exit</td>"
 "<td>Exit Config portal, config portal will close</td></tr>"
 "<tr><td>/restart</td>"
 "<td>Reboot the device</td></tr>"
 "<tr><td>/erase</td>"
 "<td>Erase WiFi configuration and reboot device. Device will not reconnect to a network until new WiFi configuration data is entered.</td></tr>"
 "</table>"
 "<p/>Github <a href='https://github.com/tzapu/WiFiManager'>https://github.com/tzapu/WiFiManager</a>.";
#else
const char HTTP_HELP[]             PROGMEM = "";
#endif

const char HTTP_UPDATE[] PROGMEM = "Upload new firmware<br/><form method='POST' action='u' enctype='multipart/form-data' onchange=\"(function(el){document.getElementById('uploadbin').style.display = el.value=='' ? 'none' : 'initial';})(this)\"><input type='file' name='update' accept='.bin,application/octet-stream'><button id='uploadbin' type='submit' class='h D'>Update</button></form><small><a href='http://192.168.4.1/update' target='_blank'>* May not function inside captive portal, open in browser http://192.168.4.1</a><small>";
const char HTTP_UPDATE_FAIL[] PROGMEM = "<div class='msg D'><strong>Update failed!</strong><Br/>Reboot device and try again</div>";
const char HTTP_UPDATE_SUCCESS[] PROGMEM = "<div class='msg S'><strong>Update successful.  </strong> <br/> Device rebooting now...</div>";

#ifdef WM_JSTEST
const char HTTP_JS[] PROGMEM =
"<script>function postAjax(url, data, success) {"
"    var params = typeof data == 'string' ? data : Object.keys(data).map("
"            function(k){ return encodeURIComponent(k) + '=' + encodeURIComponent(data[k]) }"
"        ).join('&');"
"    var xhr = window.XMLHttpRequest ? new XMLHttpRequest() : new ActiveXObject(\"Microsoft.XMLHTTP\");"
"    xhr.open('POST', url);"
"    xhr.onreadystatechange = function() {"
"        if (xhr.readyState>3 && xhr.status==200) { success(xhr.responseText); }"
"    };"
"    xhr.setRequestHeader('X-Requested-With', 'XMLHttpRequest');"
"    xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');"
"    xhr.send(params);"
"    return xhr;}"
"postAjax('/status', 'p1=1&p2=Hello+World', function(data){ console.log(data); });"
"postAjax('/status', { p1: 1, p2: 'Hello World' }, function(data){ console.log(data); });"
"</script>";
#endif

// Info html
// @todo remove html elements from progmem, repetetive strings
#ifdef ESP32
	const char HTTP_INFO_esphead[]    PROGMEM = "<h3>esp32</h3><hr><dl>";
	const char HTTP_INFO_chiprev[]    PROGMEM = "<dt>Chip rev</dt><dd>{1}</dd>";
  	const char HTTP_INFO_lastreset[]  PROGMEM = "<dt>Last reset reason</dt><dd>CPU0: {1}<br/>CPU1: {2}</dd>";
  	const char HTTP_INFO_aphost[]     PROGMEM = "<dt>Access point hostname</dt><dd>{1}</dd>";
    const char HTTP_INFO_psrsize[]    PROGMEM = "<dt>PSRAM Size</dt><dd>{1} bytes</dd>";
	const char HTTP_INFO_temp[]       PROGMEM = "<dt>Temperature</dt><dd>{1} C&deg; / {2} F&deg;</dd>";
    const char HTTP_INFO_hall[]       PROGMEM = "<dt>Hall</dt><dd>{1}</dd>";
#else
	const char HTTP_INFO_esphead[]    PROGMEM = "<h3>esp8266</h3><hr><dl>";
	const char HTTP_INFO_fchipid[]    PROGMEM = "<dt>Flash chip ID</dt><dd>{1}</dd>";
	const char HTTP_INFO_corever[]    PROGMEM = "<dt>Core version</dt><dd>{1}</dd>";
	const char HTTP_INFO_bootver[]    PROGMEM = "<dt>Boot version</dt><dd>{1}</dd>";
	const char HTTP_INFO_lastreset[]  PROGMEM = "<dt>Last reset reason</dt><dd>{1}</dd>";
	const char HTTP_INFO_flashsize[]  PROGMEM = "<dt>Real flash size</dt><dd>{1} bytes</dd>";
#endif

const char HTTP_INFO_memsmeter[]  PROGMEM = "<br/><progress value='{1}' max='{2}'></progress></dd>";
const char HTTP_INFO_memsketch[]  PROGMEM = "<dt>Memory - Sketch size</dt><dd>Used / Total bytes<br/>{1} / {2}";
const char HTTP_INFO_freeheap[]   PROGMEM = "<dt>Memory - Free heap</dt><dd>{1} bytes available</dd>";
const char HTTP_INFO_wifihead[]   PROGMEM = "<br/><h3>WiFi</h3><hr>";
const char HTTP_INFO_uptime[]     PROGMEM = "<dt>Uptime</dt><dd>{1} mins {2} secs</dd>";
const char HTTP_INFO_chipid[]     PROGMEM = "<dt>Chip ID</dt><dd>{1}</dd>";
const char HTTP_INFO_idesize[]    PROGMEM = "<dt>Flash size</dt><dd>{1} bytes</dd>";
const char HTTP_INFO_sdkver[]     PROGMEM = "<dt>SDK version</dt><dd>{1}</dd>";
const char HTTP_INFO_cpufreq[]    PROGMEM = "<dt>CPU frequency</dt><dd>{1}MHz</dd>";
const char HTTP_INFO_apip[]       PROGMEM = "<dt>Access point IP</dt><dd>{1}</dd>";
const char HTTP_INFO_apmac[]      PROGMEM = "<dt>Access point MAC</dt><dd>{1}</dd>";
const char HTTP_INFO_apssid[]     PROGMEM = "<dt>Access point SSID</dt><dd>{1}</dd>";
const char HTTP_INFO_apbssid[]    PROGMEM = "<dt>BSSID</dt><dd>{1}</dd>";
const char HTTP_INFO_stassid[]    PROGMEM = "<dt>Station SSID</dt><dd>{1}</dd>";
const char HTTP_INFO_staip[]      PROGMEM = "<dt>Station IP</dt><dd>{1}</dd>";
const char HTTP_INFO_stagw[]      PROGMEM = "<dt>Station gateway</dt><dd>{1}</dd>";
const char HTTP_INFO_stasub[]     PROGMEM = "<dt>Station subnet</dt><dd>{1}</dd>";
const char HTTP_INFO_dnss[]       PROGMEM = "<dt>DNS Server</dt><dd>{1}</dd>";
const char HTTP_INFO_host[]       PROGMEM = "<dt>Hostname</dt><dd>{1}</dd>";
const char HTTP_INFO_stamac[]     PROGMEM = "<dt>Station MAC</dt><dd>{1}</dd>";
const char HTTP_INFO_conx[]       PROGMEM = "<dt>Connected</dt><dd>{1}</dd>";
const char HTTP_INFO_autoconx[]   PROGMEM = "<dt>Autoconnect</dt><dd>{1}</dd>";

const char HTTP_INFO_aboutver[]     PROGMEM = "<dt>WiFiManager</dt><dd>{1}</dd>";
const char HTTP_INFO_aboutarduino[] PROGMEM = "<dt>Arduino</dt><dd>{1}</dd>";
const char HTTP_INFO_aboutsdk[]     PROGMEM = "<dt>ESP-SDK/IDF</dt><dd>{1}</dd>";
const char HTTP_INFO_aboutdate[]    PROGMEM = "<dt>Build date</dt><dd>{1}</dd>";

const char S_brand[]              PROGMEM = "WiFiManager";
const char S_debugPrefix[]        PROGMEM = "*wm:";
const char S_y[]                  PROGMEM = "Yes";
const char S_n[]                  PROGMEM = "No";
const char S_enable[]             PROGMEM = "Enabled";
const char S_disable[]            PROGMEM = "Disabled";
const char S_GET[]                PROGMEM = "GET";
const char S_POST[]               PROGMEM = "POST";
const char S_NA[]                 PROGMEM = "Unknown";
const char S_passph[]             PROGMEM = "********";
const char S_titlewifisaved[]     PROGMEM = "Credentials saved";
const char S_titlewifisettings[]  PROGMEM = "Settings saved";
const char S_titlewifi[]          PROGMEM = "Config ESP";
const char S_titleinfo[]          PROGMEM = "Info";
const char S_titleparam[]         PROGMEM = "Setup";
const char S_titleparamsaved[]    PROGMEM = "Setup saved";
const char S_titleexit[]          PROGMEM = "Exit";
const char S_titlereset[]         PROGMEM = "Reset";
const char S_titleerase[]         PROGMEM = "Erase";
const char S_titleclose[]         PROGMEM = "Close";
const char S_options[]            PROGMEM = "options";
const char S_nonetworks[]         PROGMEM = "No networks found. Refresh to scan again.";
const char S_staticip[]           PROGMEM = "Static IP";
const char S_staticgw[]           PROGMEM = "Static gateway";
const char S_staticdns[]          PROGMEM = "Static DNS";
const char S_subnet[]             PROGMEM = "Subnet";
const char S_exiting[]            PROGMEM = "Exiting";
const char S_resetting[]          PROGMEM = "Module will reset in a few seconds.";
const char S_closing[]            PROGMEM = "You can close the page, portal will continue to run";
const char S_error[]              PROGMEM = "An error occured";
const char S_notfound[]           PROGMEM = "File not found\n\n";
const char S_uri[]                PROGMEM = "URI: ";
const char S_method[]             PROGMEM = "\nMethod: ";
const char S_args[]               PROGMEM = "\nArguments: ";
const char S_parampre[]           PROGMEM = "param_";

// debug strings
const char D_HR[]                 PROGMEM = "--------------------";


// softap ssid default prefix
#ifdef ESP8266
    const char S_ssidpre[]        PROGMEM = "ESP";
#elif defined(ESP32)
    const char S_ssidpre[]        PROGMEM = "ESP32";
#else
    const char S_ssidpre[]        PROGMEM = "WM";
#endif

// END WIFI_MANAGER_OVERRIDE_STRINGS
#endif

#endif
