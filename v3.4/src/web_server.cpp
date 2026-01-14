#include "web_server.h"
#include "esp_log.h"

WebServerHandler::WebServerHandler(CXN0102I2C& cxn, EEPROMHandler& eeprom)
    : _server(80), _cxn(cxn), _eeprom(eeprom) {
    // Initialize settings with defaults
    _settings.pan = 0;
    _settings.tilt = 0;
    _settings.flip = 0;
    _settings.txPower = 34;
    _settings.lang = 0;
    _settings.brightness = 128;
    _settings.contrast = 128;
    _settings.hue = 128;
    _settings.saturation = 128;
    _settings.sharpness = 128;
    _settings.hueU = 128;
    _settings.hueV = 128;
    _settings.satU = 128;
    _settings.satV = 128;
}

void WebServerHandler::init() {
    setupRoutes();
    _server.begin();
    Serial.println("[WebServer] HTTP Server started.");
}

void WebServerHandler::updateSettings(const Settings& settings) {
    _settings = settings;
}

void WebServerHandler::setupRoutes() {
    // Main page
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/html", buildMainPageHtml());
    });

    // Commands by index
    _server.on("/command", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (!request->hasParam("cmd")) {
            request->send(400, "text/plain", "Missing cmd parameter");
            return;
        }
        String cmdStr = request->getParam("cmd")->value();
        int cmdIndex = cmdStr.toInt();
        _cxn.sendCommand(cmdIndex);
        request->send(200, "text/plain", "Command executed");
    });

    // Keystone (also persist)
    _server.on("/keystone", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (!request->hasParam("pan") || !request->hasParam("tilt") || !request->hasParam("flip")) {
            request->send(400, "text/plain", "Missing parameters");
            return;
        }
        _settings.pan  = clamp(request->getParam("pan")->value().toInt(),  -30, 30);
        _settings.tilt = clamp(request->getParam("tilt")->value().toInt(), -20, 20);
        _settings.flip = request->getParam("flip")->value().toInt();
        if (_settings.flip < 0 || _settings.flip > 3) _settings.flip = 0;

        _cxn.sendKeystoneAndFlip(_settings.pan, _settings.tilt, _settings.flip);
        _eeprom.saveSettings(_settings);
        request->send(200, "text/plain", "Keystone and Flip updated");
    });

    // Custom command
    _server.on("/custom_command", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (!request->hasParam("cmd")) {
            request->send(400, "text/plain", "Missing cmd parameter");
            return;
        }
        String customCmd = request->getParam("cmd")->value();
        if (customCmd.length() % 2 != 0 || customCmd.length() > 50) {
            request->send(400, "text/plain", "Invalid command format");
            return;
        }
        _cxn.sendCustomCommand(customCmd.c_str());
        request->send(200, "text/plain", "Custom command sent");
    });

    // Set Tx Power (apply & persist)
    _server.on("/set_tx_power", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (!request->hasParam("power")) {
            request->send(400, "text/plain", "Missing power parameter");
            return;
        }
        _settings.txPower = request->getParam("power")->value().toInt();
        _eeprom.saveSettings(_settings);
        request->send(200, "text/plain", "Transmit Power set to " + String(_settings.txPower / 4.0) + " dBm");
    });

    // Ping indicator
    _server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "OK");
    });

    // Get all persisted settings
    _server.on("/get_settings", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String json = "{";
        json += "\"pan\":" + String(_settings.pan) + ",";
        json += "\"tilt\":" + String(_settings.tilt) + ",";
        json += "\"flip\":" + String(_settings.flip) + ",";
        json += "\"txPower\":" + String(_settings.txPower) + ",";
        json += "\"lang\":\"" + String((_settings.lang==0)?"en":"zh") + "\",";
        json += "\"brightness\":" + String(_settings.brightness) + ",";
        json += "\"contrast\":"   + String(_settings.contrast)   + ",";
        json += "\"hueU\":"       + String(_settings.hueU)        + ",";
        json += "\"hueV\":"       + String(_settings.hueV)        + ",";
        json += "\"satU\":"       + String(_settings.satU)        + ",";
        json += "\"satV\":"       + String(_settings.satV)        + ",";
        json += "\"sharpness\":"  + String(_settings.sharpness);
        json += "}";
        request->send(200, "application/json", json);
    });

    // Set settings (any subset)
    _server.on("/set_settings", HTTP_GET, [this](AsyncWebServerRequest *request) {
        bool changed = false;
        if (request->hasParam("pan"))  { 
            _settings.pan  = clamp(request->getParam("pan")->value().toInt(), -30, 30); 
            changed = true; 
        }
        if (request->hasParam("tilt")) { 
            _settings.tilt = clamp(request->getParam("tilt")->value().toInt(), -20, 20); 
            changed = true; 
        }
        if (request->hasParam("flip")) { 
            _settings.flip = request->getParam("flip")->value().toInt(); 
            if (_settings.flip < 0 || _settings.flip > 3) _settings.flip = 0; 
            changed = true; 
        }
        if (request->hasParam("txPower")) { 
            _settings.txPower = request->getParam("txPower")->value().toInt(); 
            changed = true; 
        }
        if (request->hasParam("lang")) {
            String l = request->getParam("lang")->value();
            _settings.lang = (l == "zh") ? 1 : 0;
            changed = true;
        }
        if (changed) {
            _cxn.sendKeystoneAndFlip(_settings.pan, _settings.tilt, _settings.flip);
            _eeprom.saveSettings(_settings);
        }
        request->send(200, "text/plain", "OK");
    });

    // Set language explicitly
    _server.on("/set_lang", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (!request->hasParam("lang")) {
            request->send(400, "text/plain", "Missing lang");
            return;
        }
        String l = request->getParam("lang")->value();
        _settings.lang = (l == "zh") ? 1 : 0;
        _eeprom.saveSettings(_settings);
        request->send(200, "text/plain", "Lang updated");
    });

    // Set Picture Quality
    _server.on("/set_pq", HTTP_GET, [this](AsyncWebServerRequest *request) {
        bool pqChanged = false;
        if (request->hasParam("brightness")) {
            _settings.brightness = request->getParam("brightness")->value().toInt();
            _cxn.setBrightness(_settings.brightness);
            pqChanged = true;
        }
        if (request->hasParam("contrast")) {
            _settings.contrast = request->getParam("contrast")->value().toInt();
            _cxn.setContrast(_settings.contrast);
            pqChanged = true;
        }
        if (request->hasParam("hueU") && request->hasParam("hueV")) {
            _settings.hueU = request->getParam("hueU")->value().toInt();
            _settings.hueV = request->getParam("hueV")->value().toInt();
            _cxn.setHue(_settings.hueU, _settings.hueV);
            pqChanged = true;
        }
        if (request->hasParam("satU") && request->hasParam("satV")) {
            _settings.satU = request->getParam("satU")->value().toInt();
            _settings.satV = request->getParam("satV")->value().toInt();
            _cxn.setSaturation(_settings.satU, _settings.satV);
            pqChanged = true;
        }
        if (request->hasParam("sharpness")) {
            _settings.sharpness = request->getParam("sharpness")->value().toInt();
            _cxn.setSharpness(_settings.sharpness);
            pqChanged = true;
        }
        if (pqChanged) {
            _eeprom.saveSettings(_settings);
        }
        request->send(200, "text/plain", "PQ updated");
    });

    // Factory reset
    _server.on("/factory_reset", [this](AsyncWebServerRequest *request) {
        _cxn.factoryReset();
        request->send(200, "text/plain", "Factory reset command sent.");
    });

    // Save all parameters
    _server.on("/save_all", [this](AsyncWebServerRequest *request) {
        _cxn.saveAllParameters();
        request->send(200, "text/plain", "Save all command sent.");
    });

    // Get device info
    _server.on("/get_info", [this](AsyncWebServerRequest *request) {
        _cxn.requestDeviceInfo();
        request->send(200, "text/plain", "Info commands sent.");
    });

    // Get temperature
    _server.on("/get_temperature", [this](AsyncWebServerRequest *request) {
        CXN0102I2C::TemperatureData data = _cxn.getTemperature();
        String json = "{\"result\":" + String(data.result) +
                      ",\"temperature\":" + String(data.temperature) +
                      ",\"mute_threshold\":" + String(data.muteThreshold) +
                      ",\"stop_threshold\":" + String(data.stopThreshold) + "}";
        request->send(200, "application/json", json);
    });

    // Test pattern
    _server.on("/test_pattern", [this](AsyncWebServerRequest *request) {
        int pattern = request->hasParam("type") ? request->getParam("type")->value().toInt() : 0;
        _cxn.sendTestPattern(pattern);
        request->send(200, "text/plain", "Test pattern command sent.");
    });

    // Clear EEPROM
    _server.on("/clear_eeprom", [this](AsyncWebServerRequest *request) {
        _eeprom.clearEEPROM();
        request->send(200, "text/plain", "EEPROM cleared.");
    });
}

String WebServerHandler::buildMainPageHtml() {
    // HTML/CSS/JavaScript content - using simple string concatenation
    // to avoid raw string literal issues
    String page = "<!DOCTYPE html><html><head>";
    page += "<meta charset='utf-8'/>";
    page += "<meta name='viewport' content='width=device-width,initial-scale=1'/>";
    page += "<title>CXN0102 Controller v3.4 (Author vx:samzhangxian)</title>";
    page += "<style>";
    page += "body{font-family:Arial,system-ui,-apple-system;text-align:center;background-color:#121212;color:#ffffff;margin:0;padding:0 8px 32px;}";
    page += "h1,h2{font-weight:600;}";
    page += ".button{font-size:1.0rem;padding:0.5rem 1rem;margin:0.25rem;background-color:#333333;color:#ffffff;border:none;border-radius:6px;cursor:pointer;}";
    page += ".button:active{transform:translateY(1px);}";
    page += ".grid{display:grid;gap:8px;justify-content:center;}";
    page += ".slider-container{margin:1rem auto;max-width:520px;}";
    page += "label{display:inline-block;margin:0.25rem 0.5rem;}";
    page += "input[type='range']{width:220px;vertical-align:middle;}";
    page += "select{background-color:#333333;color:#ffffff;border:none;border-radius:6px;padding:6px 8px;}";
    page += "#statusIndicator{width:14px;height:14px;border-radius:50%;background-color:red;position:fixed;top:10px;left:10px;box-shadow:0 0 10px rgba(0,0,0,0.5);}";
    page += ".topbar{position:fixed;top:8px;right:8px;}";
    page += ".card{background:#1c1c1c;border-radius:10px;padding:12px;margin:10px auto;max-width:820px;}";
    page += ".muted{opacity:.8;font-size:.9rem;}";
    page += "</style>";

    page += "</head><body>";

    // Status dot
    page += "<div id='statusIndicator'></div>";

    // Lang selector
    page += "<div class='topbar'><select id='langSelect' onchange='onLangChange()'>";
    page += "<option value='en'>English</option>";
    page += "<option value='zh'>中文</option>";
    page += "</select></div>";

    page += "<h1 id='headerH1'>CXN0102 Controller</h1>";

    // Basic controls
    page += "<div class='card'><h2 id='basicControlsHeader'>Basic Controls</h2>";
    page += "<div class='grid'>";
    page += "<button id='btnStartInput' class='button' onclick='sendCommand(1)'>Start Input</button>";
    page += "<button id='btnStopInput' class='button' onclick='sendCommand(2)'>Stop Input</button>";
    page += "<button id='btnReboot' class='button' onclick='sendCommand(3)'>Reboot</button>";
    page += "<button id='btnShutdown' class='button' onclick='sendCommand(4)'>Shutdown</button>";
    page += "</div></div>";

    // Optical Axis
    page += "<div class='card'><h2 id='opticalAxisHeader'>Optical Axis Adjustment</h2>";
    page += "<div class='grid'>";
    page += "<button id='btnOpticalEnter' class='button' onclick='sendCommand(5)'>Enter/Next</button>";
    page += "<button id='btnOpticalPlus' class='button' onclick='sendCommand(6)'>+</button>";
    page += "<button id='btnOpticalMinus' class='button' onclick='sendCommand(7)'>-</button>";
    page += "<button id='btnOpticalExit' class='button' onclick='sendCommand(9)'>Exit (Save)</button>";
    page += "</div></div>";

    // Bi-Phase
    page += "<div class='card'><h2 id='biPhaseHeader'>Bi-Phase Adjustment</h2>";
    page += "<div class='grid'>";
    page += "<button id='btnBiPhaseEnter' class='button' onclick='sendCommand(10)'>Enter/Next</button>";
    page += "<button id='btnBiPhasePlus' class='button' onclick='sendCommand(11)'>+</button>";
    page += "<button id='btnBiPhaseMinus' class='button' onclick='sendCommand(12)'>-</button>";
    page += "<button id='btnBiPhaseExit' class='button' onclick='sendCommand(14)'>Exit (Save)</button>";
    page += "</div></div>";

    // Keystone + Flip
    page += "<div class='card'><h2 id='keystoneHeader'>Keystone Adjustment</h2>";
    page += "<div class='slider-container'>";
    page += "<label id='labelHorizontal'>Horizontal: <input type='range' min='-30' max='30' id='pan'></label>";
    page += "<label id='labelVertical'>Vertical: <input type='range' min='-20' max='20' id='tilt'></label>";
    page += "<label id='labelFlipMode'>Flip Mode: <select id='flip'>";
    page += "<option id='optionFlipNone' value='0'>None</option>";
    page += "<option id='optionFlipHorizontal' value='1'>Horizontal</option>";
    page += "<option id='optionFlipVertical' value='2'>Vertical</option>";
    page += "<option id='optionFlipBoth' value='3'>Both</option>";
    page += "</select></label>";
    page += "<button id='btnKeystoneApply' class='button' onclick='applyKeystone()'>Apply</button>";
    page += "</div></div>";

    // Picture Quality
    page += "<div class='card'><h2 id='pqHeader'>Picture Quality Adjustment</h2>";
    page += "<div class='slider-container'>";
    page += "<label id='labelBrightness'>Brightness: <input type='range' min='0' max='255' id='brightness'></label>";
    page += "<label id='labelContrast'>Contrast: <input type='range' min='0' max='255' id='contrast'></label>";
    page += "<label id='labelHueU'>Hue U: <input type='range' min='0' max='255' id='hueU'></label>";
    page += "<label id='labelHueV'>Hue V: <input type='range' min='0' max='255' id='hueV'></label>";
    page += "<label id='labelSaturationU'>Saturation U: <input type='range' min='0' max='255' id='satU'></label>";
    page += "<label id='labelSaturationV'>Saturation V: <input type='range' min='0' max='255' id='satV'></label>";
    page += "<label id='labelSharpness'>Sharpness: <input type='range' min='0' max='255' id='sharpness'></label>";
    page += "<button id='btnPQApply' class='button' onclick='applyPQ()'>Apply</button>";
    page += "</div></div>";

    // Custom I2C
    page += "<div class='card'><h2 id='customI2CHeader'>Custom I2C Command(eg.0b0100 for shutdown)</h2>";
    page += "<input type='text' id='customCmd' placeholder='Enter hex command'/> ";
    page += "<button id='btnSendCustom' class='button' onclick='sendCustomCommand()'>Send</button>";
    page += "<div class='muted' id='customHint'></div>";
    page += "</div>";

    // WiFi TX Power
    page += "<div class='card'><h2 id='wifiTxHeader'>WiFi Transmit Power</h2>";
    page += "<label id='labelSelectPower' for='txPower'>Select Power (dBm):</label> ";
    page += "<select id='txPower'>";
    page += "<option id='option78' value='78'>19.5 dBm (≈90mW)</option>";
    page += "<option id='option76' value='76'>19 dBm (≈79mW)</option>";
    page += "<option id='option74' value='74'>18.5 dBm (≈71mW)</option>";
    page += "<option id='option68' value='68'>17 dBm (≈50mW)</option>";
    page += "<option id='option60' value='60'>15 dBm (≈32mW)</option>";
    page += "<option id='option52' value='52'>13 dBm (≈20mW)</option>";
    page += "<option id='option44' value='44'>11 dBm (≈12mW)</option>";
    page += "<option id='option34' value='34'>8.5 dBm (≈7mW)</option>";
    page += "<option id='option28' value='28'>7 dBm (≈5mW)</option>";
    page += "<option id='option20' value='20'>5 dBm (≈3mW)</option>";
    page += "<option id='option8' value='8'>2 dBm (≈1.6mW)</option>";
    page += "<option id='optionMinus4' value='-4'>-1 dBm (≈0.8mW)</option>";
    page += "</select> ";
    page += "<button id='btnTxApply' class='button' onclick='applyTx()'>Apply</button>";
    page += "</div>";

    // System
    page += "<div class='card'><h2 id='systemHeader'>System</h2>";
    page += "<div style='margin-bottom:8px;'><span id='temperatureLabel'>温度:</span> <span id='temperatureValue'>--</span> ℃</div>";
    page += "<button id='btnFactoryReset' class='button' onclick='factoryReset()'>恢复出厂设置</button>";
    page += "<button id='btnSaveAll' class='button' onclick='saveAllParams()'>保存所有参数</button>";
    page += "<button id='btnGetDeviceInfo' class='button' onclick='getDeviceInfo()'>获取设备信息</button>";
    page += "<button id='btnClearEEPROM' class='button' onclick='clearEEPROM()'>清空EEPROM</button>";
    page += "<label id='labelTestPattern' for='testPattern'>测试图案: <select id='testPattern'><option value='0'>停止</option><option value='1'>色条</option><option value='2'>网格</option></select></label>";
    page += "<button id='btnTestPattern' class='button' onclick='sendTestPattern()'>输出测试图案</button>";
    page += "</div>";

    // JavaScript
    page += "<script>";
    page += "function checkConnection(){fetch('/ping').then(r=>{document.getElementById('statusIndicator').style.backgroundColor=r.ok?'green':'red';}).catch(()=>{document.getElementById('statusIndicator').style.backgroundColor='red';});}";
    page += "setInterval(checkConnection,1000);";
    page += "function sendCommand(cmd){fetch(`/command?cmd=${cmd}`).then(r=>r.text()).then(console.log);}";
    page += "function sendCustomCommand(){const cmd=document.getElementById('customCmd').value.trim();if(!cmd){alert('Please enter a command!');return;}fetch(`/custom_command?cmd=${cmd}`).then(resp=>resp.text()).then(t=>alert(t));}";
    page += "async function applyKeystone(){const pan=document.getElementById('pan').value;const tilt=document.getElementById('tilt').value;const flip=document.getElementById('flip').value;await fetch(`/keystone?pan=${pan}&tilt=${tilt}&flip=${flip}`);await fetch(`/set_settings?pan=${pan}&tilt=${tilt}&flip=${flip}`);}";
    page += "async function applyTx(){const power=document.getElementById('txPower').value;await fetch(`/set_tx_power?power=${power}`);await fetch(`/set_settings?txPower=${power}`);alert('Transmit Power applied.');}";
    page += "async function applyPQ(){const b=document.getElementById('brightness').value;const c=document.getElementById('contrast').value;const hueU=document.getElementById('hueU').value;const hueV=document.getElementById('hueV').value;const satU=document.getElementById('satU').value;const satV=document.getElementById('satV').value;const sh=document.getElementById('sharpness').value;await fetch(`/set_pq?brightness=${b}&contrast=${c}&hueU=${hueU}&hueV=${hueV}&satU=${satU}&satV=${satV}&sharpness=${sh}`);alert('Picture Quality updated.');}";
    page += "var languages={\"en\":{\"title\":\"CXN0102 Controller v3.4 (Author vx:samzhangxian)\",\"h1\":\"CXN0102 Controller\",\"basicControls\":\"Basic Controls\",\"startInput\":\"Start Input\",\"stopInput\":\"Stop Input\",\"reboot\":\"Reboot\",\"shutdown\":\"Shutdown\",\"opticalAxisAdjustment\":\"Optical Axis Adjustment\",\"enter\":\"Enter/Next\",\"exitSave\":\"Exit (Save)\",\"biPhaseAdjustment\":\"Bi-Phase Adjustment\",\"keystoneAdjustment\":\"Keystone Adjustment\",\"horizontal\":\"Horizontal:\",\"vertical\":\"Vertical:\",\"flipMode\":\"Flip Mode:\",\"none\":\"None\",\"horizontalOption\":\"Horizontal\",\"verticalOption\":\"Vertical\",\"both\":\"Both\",\"apply\":\"Apply\",\"pqAdjustment\":\"Picture Quality Adjustment\",\"brightness\":\"Brightness:\",\"contrast\":\"Contrast:\",\"hueU\":\"Hue U:\",\"hueV\":\"Hue V:\",\"saturationU\":\"Saturation U:\",\"saturationV\":\"Saturation V:\",\"sharpness\":\"Sharpness:\",\"customI2C\":\"Custom I2C Command (eg.0b0100 for shutdown)\",\"enterHexCmd\":\"Enter hex command\",\"send\":\"Send\",\"wifiTransmitPower\":\"WiFi Transmit Power\",\"selectPower\":\"Select Power (dBm):\",\"option78\":\"19.5 dBm (≈90mW)\",\"option76\":\"19 dBm (≈79mW)\",\"option74\":\"18.5 dBm (≈71mW)\",\"option68\":\"17 dBm (≈50mW)\",\"option60\":\"15 dBm (≈32mW)\",\"option52\":\"13 dBm (≈20mW)\",\"option44\":\"11 dBm (≈12mW)\",\"option34\":\"8.5 dBm (≈7mW)\",\"option28\":\"7 dBm (≈5mW)\",\"option20\":\"5 dBm (≈3mW)\",\"option8\":\"2 dBm (≈1.6mW)\",\"optionMinus4\":\"-1 dBm (≈0.8mW)\",\"system\":\"System\",\"factoryReset\":\"Factory Reset\",\"saveAll\":\"Save All Parameters\",\"getDeviceInfo\":\"Get Device Info\",\"clearEEPROM\":\"Clear EEPROM\",\"testPattern\":\"Test Pattern:\",\"testPatternStop\":\"Stop\",\"testPatternColorBar\":\"Color Bar\",\"testPatternGrid\":\"Grid\",\"testPatternBtn\":\"Output Test Pattern\",\"factoryResetAlert\":\"Factory reset command sent.\",\"saveAllAlert\":\"Save all command sent.\",\"getDeviceInfoAlert\":\"Device info:\\n\",\"getDeviceInfoFail\":\"Failed to get device info!\",\"testPatternAlert\":\"Test pattern command sent.\",\"temperature\":\"Temperature:\"},\"zh\":{\"title\":\"CXN0102 控制器 v3.4 (作者 vx:samzhangxian)\",\"h1\":\"CXN0102 控制器\",\"basicControls\":\"基本控制功能\",\"startInput\":\"开始输入\",\"stopInput\":\"停止输入\",\"reboot\":\"重启\",\"shutdown\":\"关机\",\"opticalAxisAdjustment\":\"光轴调整\",\"enter\":\"进入/切换下一项\",\"exitSave\":\"退出（保存）\",\"biPhaseAdjustment\":\"双相位调整\",\"keystoneAdjustment\":\"梯形校正\",\"horizontal\":\"水平:\",\"vertical\":\"垂直:\",\"flipMode\":\"翻转模式:\",\"none\":\"无\",\"horizontalOption\":\"水平\",\"verticalOption\":\"垂直\",\"both\":\"双向\",\"apply\":\"应用\",\"pqAdjustment\":\"画质调整\",\"brightness\":\"亮度:\",\"contrast\":\"对比度:\",\"hueU\":\"色调U:\",\"hueV\":\"色调V:\",\"saturationU\":\"饱和度U:\",\"saturationV\":\"饱和度V:\",\"sharpness\":\"锐度:\",\"customI2C\":\"自定义 I2C 命令（例如：0b0100 关机）\",\"enterHexCmd\":\"输入十六进制命令\",\"send\":\"发送\",\"wifiTransmitPower\":\"WiFi 发射功率\",\"selectPower\":\"选择功率 (dBm):\",\"option78\":\"19.5 dBm (约90毫瓦)\",\"option76\":\"19 dBm (约79毫瓦)\",\"option74\":\"18.5 dBm (约71毫瓦)\",\"option68\":\"17 dBm (约50毫瓦)\",\"option60\":\"15 dBm (约32毫瓦)\",\"option52\":\"13 dBm (约20毫瓦)\",\"option44\":\"11 dBm (约12毫瓦)\",\"option34\":\"8.5 dBm (约7毫瓦)\",\"option28\":\"7 dBm (约5毫瓦)\",\"option20\":\"5 dBm (约3毫瓦)\",\"option8\":\"2 dBm (约1.6毫瓦)\",\"optionMinus4\":\"-1 dBm (约0.8毫瓦)\",\"system\":\"系统\",\"factoryReset\":\"恢复出厂设置\",\"saveAll\":\"保存所有参数\",\"getDeviceInfo\":\"获取设备信息\",\"clearEEPROM\":\"清空EEPROM\",\"testPattern\":\"测试图案：\",\"testPatternStop\":\"停止\",\"testPatternColorBar\":\"色条\",\"testPatternGrid\":\"网格\",\"testPatternBtn\":\"输出测试图案\",\"factoryResetAlert\":\"已发送恢复出厂设置命令。\",\"saveAllAlert\":\"已发送保存所有参数命令。\",\"getDeviceInfoAlert\":\"设备信息：\\n\",\"getDeviceInfoFail\":\"获取设备信息失败！\",\"testPatternAlert\":\"已发送测试图案命令。\",\"temperature\":\"温度:\"}};";
    page += "function switchLanguage(){var lang=document.getElementById('langSelect').value;var dict=languages[lang];document.title=dict.title;document.getElementById('headerH1').innerText=dict.h1;document.getElementById('basicControlsHeader').innerText=dict.basicControls;document.getElementById('btnStartInput').innerText=dict.startInput;document.getElementById('btnStopInput').innerText=dict.stopInput;document.getElementById('btnReboot').innerText=dict.reboot;document.getElementById('btnShutdown').innerText=dict.shutdown;document.getElementById('opticalAxisHeader').innerText=dict.opticalAxisAdjustment;document.getElementById('btnOpticalEnter').innerText=dict.enter;document.getElementById('btnOpticalExit').innerText=dict.exitSave;document.getElementById('biPhaseHeader').innerText=dict.biPhaseAdjustment;document.getElementById('btnBiPhaseEnter').innerText=dict.enter;document.getElementById('btnBiPhaseExit').innerText=dict.exitSave;document.getElementById('keystoneHeader').innerText=dict.keystoneAdjustment;document.getElementById('labelHorizontal').childNodes[0].nodeValue=dict.horizontal+' ';document.getElementById('labelVertical').childNodes[0].nodeValue=dict.vertical+' ';document.getElementById('labelFlipMode').childNodes[0].nodeValue=dict.flipMode+' ';document.getElementById('optionFlipNone').innerText=dict.none;document.getElementById('optionFlipHorizontal').innerText=dict.horizontalOption;document.getElementById('optionFlipVertical').innerText=dict.verticalOption;document.getElementById('optionFlipBoth').innerText=dict.both;document.getElementById('btnKeystoneApply').innerText=dict.apply;document.getElementById('pqHeader').innerText=dict.pqAdjustment;document.getElementById('labelBrightness').childNodes[0].nodeValue=dict.brightness+' ';document.getElementById('labelContrast').childNodes[0].nodeValue=dict.contrast+' ';document.getElementById('labelHueU').childNodes[0].nodeValue=dict.hueU+' ';document.getElementById('labelHueV').childNodes[0].nodeValue=dict.hueV+' ';document.getElementById('labelSaturationU').childNodes[0].nodeValue=dict.saturationU+' ';document.getElementById('labelSaturationV').childNodes[0].nodeValue=dict.saturationV+' ';document.getElementById('labelSharpness').childNodes[0].nodeValue=dict.sharpness+' ';document.getElementById('btnPQApply').innerText=dict.apply;document.getElementById('customI2CHeader').innerText=dict.customI2C;document.getElementById('customCmd').placeholder=dict.enterHexCmd;document.getElementById('btnSendCustom').innerText=dict.send;document.getElementById('wifiTxHeader').innerText=dict.wifiTransmitPower;document.getElementById('labelSelectPower').innerText=dict.selectPower;document.getElementById('option78').innerText=dict.option78;document.getElementById('option76').innerText=dict.option76;document.getElementById('option74').innerText=dict.option74;document.getElementById('option68').innerText=dict.option68;document.getElementById('option60').innerText=dict.option60;document.getElementById('option52').innerText=dict.option52;document.getElementById('option44').innerText=dict.option44;document.getElementById('option34').innerText=dict.option34;document.getElementById('option28').innerText=dict.option28;document.getElementById('option20').innerText=dict.option20;document.getElementById('option8').innerText=dict.option8;document.getElementById('optionMinus4').innerText=dict.optionMinus4;document.getElementById('btnTxApply').innerText=dict.apply;document.getElementById('systemHeader').innerText=dict.system;document.getElementById('btnFactoryReset').innerText=dict.factoryReset;document.getElementById('btnSaveAll').innerText=dict.saveAll;document.getElementById('btnGetDeviceInfo').innerText=dict.getDeviceInfo;document.getElementById('btnClearEEPROM').innerText=dict.clearEEPROM;document.getElementById('labelTestPattern').childNodes[0].nodeValue=dict.testPattern;document.getElementById('testPattern').options[0].text=dict.testPatternStop;document.getElementById('testPattern').options[1].text=dict.testPatternColorBar;document.getElementById('testPattern').options[2].text=dict.testPatternGrid;document.getElementById('btnTestPattern').innerText=dict.testPatternBtn;document.getElementById('temperatureLabel').innerText=dict.temperature;}";
    page += "async function onLangChange(){const lang=document.getElementById('langSelect').value;await fetch(`/set_lang?lang=${lang}`);switchLanguage();}";
    page += "async function loadSettings(){try{const r=await fetch('/get_settings');const s=await r.json();document.getElementById('pan').value=s.pan;document.getElementById('tilt').value=s.tilt;document.getElementById('flip').value=s.flip;document.getElementById('txPower').value=s.txPower;document.getElementById('langSelect').value=s.lang;document.getElementById('brightness').value=s.brightness;document.getElementById('contrast').value=s.contrast;document.getElementById('hueU').value=s.hueU??s.hue;document.getElementById('hueV').value=s.hueV??s.hue;document.getElementById('satU').value=s.satU??s.saturation;document.getElementById('satV').value=s.satV??s.saturation;document.getElementById('sharpness').value=s.sharpness;switchLanguage();}catch(e){console.error(e);}}";
    page += "function factoryReset(){fetch('/factory_reset').then(r=>r.text()).then(()=>alert(languages[document.getElementById('langSelect').value].factoryResetAlert));}";
    page += "function saveAllParams(){fetch('/save_all').then(r=>r.text()).then(()=>alert(languages[document.getElementById('langSelect').value].saveAllAlert));}";
    page += "function clearEEPROM(){fetch('/clear_eeprom').then(r=>r.text()).then(()=>alert('EEPROM cleared.'));}";
    page += "async function getDeviceInfo(){try{const lang=document.getElementById('langSelect').value;const dict=languages[lang];const r=await fetch('/get_device_info');const info=await r.json();let msg=dict.getDeviceInfoAlert+'\\n';if(info.temperature)msg+=`${dict.temperature}${info.temperature.current}°C (${info.temperature.lower}~${info.temperature.upper}°C)\\n`;if(info.runtime)msg+=`运行时间:${info.runtime}秒\\n`;if(info.version)msg+=`固件版本:${info.version.firmware}\\n参数版本:${info.version.parameter}\\n数据版本:${info.version.data}\\n`;if(info.lot_number)msg+=`生产批号:${info.lot_number}\\n`;if(info.serial_number)msg+=`序列号:${info.serial_number}\\n`;alert(msg);}catch(e){const lang=document.getElementById('langSelect').value;alert(languages[lang].getDeviceInfoFail);console.error(e);}}";
    page += "function sendTestPattern(){const lang=document.getElementById('langSelect').value;fetch(`/test_pattern?type=${document.getElementById('testPattern').value}`).then(r=>r.text()).then(()=>alert(languages[lang].testPatternAlert));}";
    page += "async function updateTemperature(){try{const r=await fetch('/get_temperature');const t=await r.json();document.getElementById('temperatureValue').innerText=(typeof t.temperature==='number')?t.temperature:'--';}catch(e){document.getElementById('temperatureValue').innerText='--';}}";
    page += "setInterval(updateTemperature,10000);";
    page += "document.addEventListener('DOMContentLoaded',()=>{loadSettings();updateTemperature();});";
    page += "</script>";

    page += "</body></html>";
    return page;
}
