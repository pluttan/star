#include "web_server.h"
#include "config.h"
#include <WiFi.h>
#include <Update.h>

LEDWebServer::LEDWebServer(LEDEffects* effects, uint16_t port)
    : server(port), effects(effects), onSettingsChange(nullptr) {}

void LEDWebServer::setOnSettingsChange(void (*callback)()) {
    onSettingsChange = callback;
}

void LEDWebServer::begin() {
    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/api/status", HTTP_GET, [this]() { handleGetStatus(); });
    server.on("/api/effect", HTTP_POST, [this]() { handleSetEffect(); });
    server.on("/api/brightness", HTTP_POST, [this]() { handleSetBrightness(); });
    server.on("/api/speed", HTTP_POST, [this]() { handleSetSpeed(); });
    server.on("/api/color", HTTP_POST, [this]() { handleSetColor(); });
    server.on("/api/autocycle", HTTP_POST, [this]() { handleSetAutoCycle(); });
    server.on("/api/palette", HTTP_POST, [this]() { handleSetPalette(); });
    server.on("/api/reboot", HTTP_POST, [this]() { handleReboot(); });
    server.on("/api/debug", HTTP_GET, [this]() { handleDebug(); });
    
    // HTTP OTA Update endpoint
    server.on("/api/update", HTTP_POST, 
        [this]() { handleUpdateEnd(); },
        [this]() { handleUpdateUpload(); }
    );
    
    server.onNotFound([this]() { handleNotFound(); });
    server.begin();
    Serial.println("HTTP server started");
}

void LEDWebServer::handleClient() { server.handleClient(); }

void LEDWebServer::handleRoot() { server.send(200, "text/html", generateHTML()); }

void LEDWebServer::handleGetStatus() {
    uint8_t r, g, b;
    effects->getColor(&r, &g, &b);
    
    String json = "{\"effect\":" + String(effects->getEffect());
    json += ",\"effectName\":\"" + String(EFFECT_NAMES[effects->getEffect()]) + "\"";
    json += ",\"brightness\":" + String(effects->getBrightness());
    json += ",\"speed\":" + String(effects->getSpeed());
    json += ",\"color\":{\"r\":" + String(r) + ",\"g\":" + String(g) + ",\"b\":" + String(b) + "}";
    json += ",\"autoCycle\":" + String(effects->getAutoCycle() ? "true" : "false");
    json += ",\"cycleTime\":" + String(effects->getCycleTime());
    json += ",\"paletteSize\":" + String(effects->getPaletteSize());
    json += ",\"palette\":[";
    for (int i = 0; i < effects->getPaletteSize(); i++) {
        uint8_t pr, pg, pb;
        effects->getPaletteColor(i, &pr, &pg, &pb);
        json += "{\"r\":" + String(pr) + ",\"g\":" + String(pg) + ",\"b\":" + String(pb) + "}";
        if (i < effects->getPaletteSize() - 1) json += ",";
    }
    json += "]";
    json += ",\"effectCount\":" + String(EFFECT_COUNT) + "}";
    
    server.send(200, "application/json", json);
}

void LEDWebServer::handleSetEffect() {
    if (server.hasArg("value")) {
        int effect = server.arg("value").toInt();
        if (effect >= 0 && effect < EFFECT_COUNT) {
            effects->setEffect((Effect)effect);
            if (onSettingsChange) onSettingsChange();
            server.send(200, "application/json", "{\"ok\":1}");
            return;
        }
    }
    server.send(400, "application/json", "{\"error\":1}");
}

void LEDWebServer::handleSetBrightness() {
    if (server.hasArg("value")) {
        effects->setBrightness(server.arg("value").toInt());
        if (onSettingsChange) onSettingsChange();
        server.send(200, "application/json", "{\"ok\":1}");
        return;
    }
    server.send(400, "application/json", "{\"error\":1}");
}

void LEDWebServer::handleSetSpeed() {
    if (server.hasArg("value")) {
        effects->setSpeed(server.arg("value").toInt());
        if (onSettingsChange) onSettingsChange();
        server.send(200, "application/json", "{\"ok\":1}");
        return;
    }
    server.send(400, "application/json", "{\"error\":1}");
}

void LEDWebServer::handleSetColor() {
    if (server.hasArg("r") && server.hasArg("g") && server.hasArg("b")) {
        effects->setColor(server.arg("r").toInt(), server.arg("g").toInt(), server.arg("b").toInt());
        if (onSettingsChange) onSettingsChange();
        server.send(200, "application/json", "{\"ok\":1}");
        return;
    }
    server.send(400, "application/json", "{\"error\":1}");
}

void LEDWebServer::handleSetAutoCycle() {
    if (server.hasArg("enabled")) {
        effects->setAutoCycle(server.arg("enabled") == "1");
    }
    if (server.hasArg("time")) {
        effects->setCycleTime(server.arg("time").toInt());
    }
    if (onSettingsChange) onSettingsChange();
    server.send(200, "application/json", "{\"ok\":1}");
}

void LEDWebServer::handleSetPalette() {
    // Set palette size
    if (server.hasArg("size")) {
        effects->setPaletteSize(server.arg("size").toInt());
    }
    // Set individual color
    if (server.hasArg("index") && server.hasArg("r") && server.hasArg("g") && server.hasArg("b")) {
        int idx = server.arg("index").toInt();
        effects->setPaletteColor(idx, server.arg("r").toInt(), server.arg("g").toInt(), server.arg("b").toInt());
    }
    if (onSettingsChange) onSettingsChange();
    server.send(200, "application/json", "{\"ok\":1}");
}

void LEDWebServer::handleReboot() {
    server.send(200, "application/json", "{\"rebooting\":true}");
    delay(500);
    ESP.restart();
}

void LEDWebServer::handleDebug() {
    String json = "{";
    json += "\"freeHeap\":" + String(ESP.getFreeHeap());
    json += ",\"heapSize\":" + String(ESP.getHeapSize());
    json += ",\"chipModel\":\"" + String(ESP.getChipModel()) + "\"";
    json += ",\"cpuFreq\":" + String(ESP.getCpuFreqMHz());
    json += ",\"flashSize\":" + String(ESP.getFlashChipSize());
    json += ",\"sketchSize\":" + String(ESP.getSketchSize());
    json += ",\"freeSketchSpace\":" + String(ESP.getFreeSketchSpace());
    json += ",\"uptime\":" + String(millis() / 1000);
    json += ",\"wifiRSSI\":" + String(WiFi.RSSI());
    json += ",\"wifiIP\":\"" + WiFi.localIP().toString() + "\"";
    #if OTA_ENABLED
    json += ",\"otaEnabled\":true";
    #else
    json += ",\"otaEnabled\":false";
    #endif
    json += ",\"otaError\":\"" + otaError + "\"";
    json += ",\"otaState\":\"" + otaState + "\"";
    json += ",\"otaProgress\":" + String(otaProgress);
    json += "}";
    server.send(200, "application/json", json);
}

void LEDWebServer::handleNotFound() { server.send(404, "text/plain", "404"); }

void LEDWebServer::handleUpdateUpload() {
    HTTPUpload& upload = server.upload();
    
    if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("HTTP OTA: Start %s\n", upload.filename.c_str());
        otaState = "Uploading";
        otaProgress = 0;
        otaError = "";
        
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            otaError = "Begin Failed: " + String(Update.errorString());
            otaState = "Failed";
            Serial.println(otaError);
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            otaError = "Write Failed: " + String(Update.errorString());
            otaState = "Failed";
            Serial.println(otaError);
        } else {
            otaProgress = (Update.progress() * 100) / Update.size();
            Serial.printf("HTTP OTA Progress: %u%%\r", otaProgress);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
            otaState = "Success";
            otaProgress = 100;
            Serial.printf("\nHTTP OTA: Success! %u bytes\n", upload.totalSize);
        } else {
            otaError = "End Failed: " + String(Update.errorString());
            otaState = "Failed";
            Serial.println(otaError);
        }
    }
}

void LEDWebServer::handleUpdateEnd() {
    if (Update.hasError()) {
        server.send(500, "application/json", "{\"error\":\"" + otaError + "\"}");
    } else {
        server.send(200, "application/json", "{\"ok\":true,\"message\":\"Rebooting...\"}");
        delay(500);
        ESP.restart();
    }
}

String LEDWebServer::generateHTML() {
    String html = R"(<!DOCTYPE html><html><head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>LED Star</title><style>
*{margin:0;padding:0;box-sizing:border-box}
:root{--bg:#0a0a18;--card:rgba(30,30,60,0.85);--accent:#6366f1;--text:#fff;--text2:#a0a0c0}
body{font-family:system-ui,sans-serif;background:var(--bg);min-height:100vh;color:var(--text);padding:16px}
.c{max-width:500px;margin:0 auto}
h1{text-align:center;font-size:1.8rem;margin-bottom:20px;background:linear-gradient(135deg,#fff,#8888ff);-webkit-background-clip:text;-webkit-text-fill-color:transparent}
.card{background:var(--card);border-radius:16px;padding:20px;margin-bottom:16px}
h2{font-size:1rem;color:var(--text2);margin-bottom:14px}
.effects{display:flex;flex-wrap:wrap;gap:8px}
.group{width:100%;margin-bottom:10px}
.group-title{font-size:0.75rem;color:var(--text2);margin-bottom:6px;text-transform:uppercase;letter-spacing:1px}
.btn{background:rgba(255,255,255,0.08);border:1px solid rgba(255,255,255,0.1);border-radius:10px;padding:10px 14px;color:var(--text);font-size:0.8rem;cursor:pointer;transition:all 0.2s}
.btn:hover{background:rgba(99,102,241,0.3)}
.btn.on{background:linear-gradient(135deg,var(--accent),#8b5cf6);border-color:transparent}
.ctrl{margin-bottom:18px}
.ctrl:last-child{margin-bottom:0}
.ctrl label{display:flex;justify-content:space-between;align-items:center;margin-bottom:8px;font-size:0.9rem;color:var(--text2)}
.val{background:rgba(99,102,241,0.25);padding:2px 10px;border-radius:12px;font-size:0.85rem;color:var(--accent)}
input[type=range]{width:100%;height:6px;border-radius:3px;background:rgba(255,255,255,0.1);outline:none;-webkit-appearance:none}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:20px;height:20px;border-radius:50%;background:linear-gradient(135deg,var(--accent),#8b5cf6);cursor:pointer}
.colors{display:flex;gap:6px;flex-wrap:wrap}
.colors input[type=color]{width:60px;height:40px;border:none;border-radius:10px;cursor:pointer;background:transparent}
.colors input[type=color]::-webkit-color-swatch-wrapper{padding:0}
.colors input[type=color]::-webkit-color-swatch{border:2px solid rgba(255,255,255,0.2);border-radius:8px}
.preset{width:32px;height:32px;border-radius:50%;border:2px solid rgba(255,255,255,0.2);cursor:pointer;transition:transform 0.2s}
.preset:hover{transform:scale(1.15)}
.toggle{display:flex;align-items:center;gap:12px}
.switch{position:relative;width:50px;height:26px;background:rgba(255,255,255,0.1);border-radius:13px;cursor:pointer;transition:background 0.3s}
.switch.on{background:var(--accent)}
.switch::after{content:'';position:absolute;top:3px;left:3px;width:20px;height:20px;border-radius:50%;background:#fff;transition:left 0.3s}
.switch.on::after{left:27px}
.time-input{display:flex;align-items:center;gap:8px;margin-top:10px}
.time-input input{width:70px;background:rgba(255,255,255,0.1);border:1px solid rgba(255,255,255,0.1);border-radius:8px;padding:8px;color:var(--text);font-size:0.9rem;text-align:center}
.time-input span{color:var(--text2);font-size:0.85rem}
</style></head><body><div class="c">
<h1>⭐ LED Star</h1>
<div class="card"><h2>Effects</h2><div id="fx"></div></div>
<div class="card"><h2>Settings</h2>
<div class="ctrl"><label>Brightness <span class="val" id="bv">150</span></label><input type="range" id="br" min="0" max="255" value="150"></div>
<div class="ctrl"><label>Speed <span class="val" id="sv">50</span></label><input type="range" id="sp" min="1" max="255" value="50"></div>
<div class="ctrl"><label>Color</label><div class="colors"><input type="color" id="col" value="#ff0000">
<div class="preset" style="background:#ff0000" data-c="#ff0000"></div>
<div class="preset" style="background:#00ff00" data-c="#00ff00"></div>
<div class="preset" style="background:#0066ff" data-c="#0066ff"></div>
<div class="preset" style="background:#ffff00" data-c="#ffff00"></div>
<div class="preset" style="background:#ff00ff" data-c="#ff00ff"></div>
<div class="preset" style="background:#00ffff" data-c="#00ffff"></div>
<div class="preset" style="background:#ffffff" data-c="#ffffff"></div>
<div class="preset" style="background:#ff8800" data-c="#ff8800"></div>
</div></div>
</div>
<div class="card"><h2>Auto Cycle</h2>
<div class="toggle"><div class="switch" id="cycle"></div><span id="cycleLabel">Off</span></div>
<div class="time-input"><input type="number" id="cycleTime" min="5" max="300" value="30"><span>sec per effect</span></div>
</div>
<div class="card"><h2>Palette Colors</h2>
<div id="palette" class="colors"></div>
<div style="margin-top:10px"><button class="btn" id="addCol">+ Add</button><button class="btn" id="remCol">- Remove</button></div>
</div>
</div>
<script>)";
    
    // Generate effects list grouped
    html += "const E=[";
    for (int i = 0; i < EFFECT_COUNT; i++) {
        html += "{n:\"" + String(EFFECT_NAMES[i]) + "\",g:" + String(EFFECT_GROUPS[i]) + "}";
        if (i < EFFECT_COUNT - 1) html += ",";
    }
    html += "];const G=[";
    for (int i = 0; i < GROUP_COUNT; i++) {
        html += "\"" + String(GROUP_NAMES[i]) + "\"";
        if (i < GROUP_COUNT - 1) html += ",";
    }
    html += R"(];
let cur=0,db;
const fx=document.getElementById('fx');
G.forEach((g,gi)=>{let d=document.createElement('div');d.className='group';d.innerHTML=`<div class="group-title">${g}</div><div class="effects" id="g${gi}"></div>`;fx.appendChild(d)});
E.forEach((e,i)=>{let b=document.createElement('button');b.className='btn';b.textContent=e.n;b.onclick=()=>setFx(i);document.getElementById('g'+e.g).appendChild(b)});
fetch('/api/status').then(r=>r.json()).then(d=>{cur=d.effect;upd();
document.getElementById('br').value=d.brightness;document.getElementById('bv').textContent=d.brightness;
document.getElementById('sp').value=d.speed;document.getElementById('sv').textContent=d.speed;
document.getElementById('col').value='#'+d.color.r.toString(16).padStart(2,'0')+d.color.g.toString(16).padStart(2,'0')+d.color.b.toString(16).padStart(2,'0');
document.getElementById('cycle').classList.toggle('on',d.autoCycle);
document.getElementById('cycleLabel').textContent=d.autoCycle?'On':'Off';
document.getElementById('cycleTime').value=d.cycleTime;
initPalette(d.palette)});
function upd(){document.querySelectorAll('.btn').forEach((b,i)=>b.classList.toggle('on',i===cur))}
function setFx(i){cur=i;upd();fetch('/api/effect',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'value='+i})}
document.getElementById('br').oninput=function(){document.getElementById('bv').textContent=this.value;deb(()=>fetch('/api/brightness',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'value='+this.value}))};
document.getElementById('sp').oninput=function(){document.getElementById('sv').textContent=this.value;deb(()=>fetch('/api/speed',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'value='+this.value}))};
document.getElementById('col').oninput=function(){deb(()=>sendCol(this.value))};
document.querySelectorAll('.preset').forEach(p=>p.onclick=function(){document.getElementById('col').value=this.dataset.c;sendCol(this.dataset.c)});
function sendCol(h){fetch('/api/color',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:`r=${parseInt(h.substr(1,2),16)}&g=${parseInt(h.substr(3,2),16)}&b=${parseInt(h.substr(5,2),16)}`})}
function deb(fn){clearTimeout(db);db=setTimeout(fn,100)}
document.getElementById('cycle').onclick=function(){this.classList.toggle('on');let on=this.classList.contains('on');document.getElementById('cycleLabel').textContent=on?'On':'Off';fetch('/api/autocycle',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'enabled='+(on?1:0)})};
document.getElementById('cycleTime').onchange=function(){fetch('/api/autocycle',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'time='+this.value})};
let pal=[];
function initPalette(p){pal=p;renderPalette()}
function renderPalette(){const c=document.getElementById('palette');c.innerHTML='';
pal.forEach((col,i)=>{const inp=document.createElement('input');inp.type='color';inp.value='#'+col.r.toString(16).padStart(2,'0')+col.g.toString(16).padStart(2,'0')+col.b.toString(16).padStart(2,'0');
inp.oninput=function(){const h=this.value;deb(()=>setPalCol(i,h))};c.appendChild(inp)})}
function setPalCol(i,h){const r=parseInt(h.substr(1,2),16),g=parseInt(h.substr(3,2),16),b=parseInt(h.substr(5,2),16);pal[i]={r,g,b};fetch('/api/palette',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:`index=${i}&r=${r}&g=${g}&b=${b}`})}
document.getElementById('addCol').onclick=function(){if(pal.length<7){pal.push({r:255,g:255,b:255});fetch('/api/palette',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'size='+pal.length}).then(()=>renderPalette())}};
document.getElementById('remCol').onclick=function(){if(pal.length>1){pal.pop();fetch('/api/palette',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'size='+pal.length}).then(()=>renderPalette())}};
</script></body></html>)";
    
    return html;
}
