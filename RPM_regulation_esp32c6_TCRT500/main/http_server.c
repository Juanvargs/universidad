#include "http_server.h"

#include "config.h"
#include "control.h"
#include "cJSON.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "HTTP";
static httpd_handle_t server;

static const char INDEX_HTML[] =
"<!doctype html>\n"
"<html lang=\"es\">\n"
"<head>\n"
"  <meta charset=\"utf-8\">\n"
"  <meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n"
"  <title>RPM Control</title>\n"
"  <link rel=\"icon\" href=\"/icon.svg\" type=\"image/svg+xml\">\n"
"  <link rel=\"stylesheet\" href=\"/styles.css\">\n"
"</head>\n"
"<body>\n"
"  <main class=\"app\">\n"
"    <header class=\"topbar\">\n"
"      <div class=\"brand\"><img src=\"/icon.svg\" alt=\"\"><div><span>ESP32-C6 PID</span><h1>RPM Control</h1></div></div>\n"
"      <div class=\"status-strip\"><b id=\"connectionState\">Conectando</b><span id=\"modeValue\">--</span></div>\n"
"    </header>\n"
"    <section class=\"layout\">\n"
"      <article class=\"panel live-panel\">\n"
"        <div class=\"panel-head\"><span>RPM actual</span><strong id=\"rpmValue\">--</strong></div>\n"
"        <div class=\"gauge\"><div class=\"gauge-ring\"><div class=\"needle\" id=\"needle\"></div></div><div class=\"gauge-center\"><b id=\"rpmMini\">0</b><span>rpm</span></div></div>\n"
"        <div class=\"progress\"><span id=\"rpmBar\"></span></div>\n"
"      </article>\n"
"      <article class=\"panel control-panel\">\n"
"        <div class=\"panel-head compact\"><span>Referencia</span><strong id=\"setpointValue\">--</strong></div>\n"
"        <form id=\"setpointForm\" class=\"setpoint-form\">\n"
"          <input id=\"setpointRange\" type=\"range\" min=\"0\" max=\"6000\" step=\"1\" value=\"500\">\n"
"          <div class=\"input-line\"><button type=\"button\" class=\"step\" data-step=\"-50\">-50</button><input id=\"setpointInput\" name=\"rpm\" type=\"number\" min=\"0\" max=\"6000\" step=\"1\" inputmode=\"numeric\" required><button type=\"button\" class=\"step\" data-step=\"50\">+50</button></div>\n"
"          <div class=\"preset-row\"><button type=\"button\" data-rpm=\"0\">0</button><button type=\"button\" data-rpm=\"500\">500</button><button type=\"button\" data-rpm=\"1000\">1000</button><button type=\"button\" data-rpm=\"3000\">3000</button><button type=\"button\" data-rpm=\"6000\">6000</button></div>\n"
"          <div class=\"action-row\"><button id=\"startButton\" class=\"secondary\" type=\"button\">Iniciar</button><button id=\"pauseButton\" class=\"danger\" type=\"button\">Pausar</button></div>\n"
"          <button class=\"primary\" type=\"submit\">Aplicar RPM</button>\n"
"          <p id=\"formMessage\" class=\"form-message\">Rango permitido: 0 a 6000 RPM</p>\n"
"        </form>\n"
"      </article>\n"
"      <article class=\"panel telemetry-panel\">\n"
"        <div class=\"metric\"><span>PIDOUT</span><b id=\"outputValue\">--</b></div>\n"
"        <div class=\"metric\"><span>CMD</span><b id=\"commandValue\">--</b></div>\n"
"        <div class=\"metric\"><span>DUTY</span><b id=\"dutyValue\">--</b></div>\n"
"        <div class=\"metric\"><span>Error</span><b id=\"errorValue\">--</b></div>\n"
"      </article>\n"
"      <article class=\"panel history-panel\">\n"
"        <div class=\"panel-head compact\"><span>Historico</span><strong id=\"samplesValue\">0 muestras</strong></div>\n"
"        <canvas id=\"historyChart\" width=\"1000\" height=\"330\"></canvas>\n"
"      </article>\n"
"    </section>\n"
"  </main>\n"
"  <script src=\"/app.js\"></script>\n"
"</body>\n"
"</html>\n";

static const char STYLES_CSS[] =
":root{color-scheme:dark;--bg:#080b10;--panel:#101722;--panel2:#141d2a;--text:#eef6ff;--muted:#91a1b5;--line:#273244;--cyan:#30d5ff;--green:#67f0a0;--yellow:#f7c948;--red:#ff6b6b}*{box-sizing:border-box}body{margin:0;min-height:100vh;background:#080b10;font-family:Inter,Segoe UI,Arial,sans-serif;color:var(--text)}.app{width:min(1240px,calc(100% - 28px));margin:0 auto;padding:18px 0}.topbar{height:88px;display:flex;align-items:center;justify-content:space-between;gap:18px}.brand{display:flex;align-items:center;gap:14px}.brand img{width:54px;height:54px}.brand span,.panel-head span,.metric span{display:block;color:var(--muted);font-size:12px;font-weight:800;text-transform:uppercase}.brand h1{margin:2px 0 0;font-size:34px;line-height:1;letter-spacing:0}.status-strip{display:flex;gap:10px;align-items:center}.status-strip b,.status-strip span{min-width:96px;text-align:center;border:1px solid var(--line);background:#0d1320;border-radius:8px;padding:10px 12px;color:var(--yellow);font-size:13px;font-weight:900}.status-strip b.ok{color:var(--green)}.status-strip b.fail{color:var(--red)}.layout{display:grid;grid-template-columns:1.05fr .95fr;gap:14px}.panel{background:linear-gradient(180deg,rgba(255,255,255,.045),rgba(255,255,255,.018));border:1px solid var(--line);border-radius:8px;padding:16px;box-shadow:0 18px 54px rgba(0,0,0,.24)}.panel-head{display:flex;align-items:flex-start;justify-content:space-between;gap:14px}.panel-head strong{font-size:clamp(46px,9vw,92px);line-height:.82}.panel-head.compact strong{font-size:28px}.gauge{position:relative;height:282px;display:grid;place-items:center}.gauge-ring{position:relative;width:min(72vw,280px);aspect-ratio:1;border-radius:50%;background:conic-gradient(from 225deg,var(--cyan),var(--green) 105deg,var(--yellow) 180deg,#273244 181deg 360deg);mask:radial-gradient(circle,transparent 0 57%,#000 58%);-webkit-mask:radial-gradient(circle,transparent 0 57%,#000 58%)}.needle{position:absolute;left:50%;top:50%;width:4px;height:42%;background:var(--text);border-radius:4px;transform-origin:50% 100%;transform:translate(-50%,-100%) rotate(-115deg);box-shadow:0 0 18px rgba(48,213,255,.55)}.gauge-center{position:absolute;display:grid;place-items:center}.gauge-center b{font-size:48px}.gauge-center span{color:var(--muted);font-size:12px;font-weight:900}.progress{height:10px;background:#0b111a;border:1px solid var(--line);border-radius:999px;overflow:hidden}.progress span{display:block;height:100%;width:0;background:linear-gradient(90deg,var(--cyan),var(--green),var(--yellow))}.setpoint-form{display:grid;gap:13px;margin-top:16px}input[type=range]{width:100%;accent-color:var(--green)}.input-line{display:grid;grid-template-columns:70px 1fr 70px;gap:10px}input[type=number]{width:100%;height:58px;border:1px solid var(--line);background:#0d1320;color:var(--text);border-radius:8px;padding:0 14px;font-size:28px;font-weight:950;text-align:center}button{height:48px;border:1px solid var(--line);border-radius:8px;background:#172234;color:var(--text);font-weight:950;cursor:pointer}button:active{transform:translateY(1px)}button.primary{height:54px;border:0;background:linear-gradient(135deg,var(--cyan),var(--green));color:#051018}.action-row{display:grid;grid-template-columns:1fr 1fr;gap:10px}.action-row .secondary{border-color:rgba(103,240,160,.45);color:var(--green);background:#0f1f1c}.action-row .danger{border-color:rgba(255,107,107,.55);color:var(--red);background:#221217}.preset-row{display:grid;grid-template-columns:repeat(5,1fr);gap:8px}.form-message{min-height:20px;margin:0;color:var(--muted);font-size:13px;font-weight:800}.form-message.ok{color:var(--green)}.form-message.fail{color:var(--red)}.telemetry-panel{grid-column:1/-1;display:grid;grid-template-columns:repeat(4,1fr);gap:10px}.metric{background:#0d1320;border:1px solid var(--line);border-radius:8px;padding:13px}.metric b{display:block;margin-top:8px;font-size:30px}.history-panel{grid-column:1/-1}canvas{width:100%;height:330px;display:block;background:#0b111b;border:1px solid var(--line);border-radius:8px;margin-top:12px}@media(max-width:820px){.topbar{height:auto;align-items:flex-start;flex-direction:column;padding:10px 0}.layout{grid-template-columns:1fr}.telemetry-panel{grid-template-columns:repeat(2,1fr)}.input-line{grid-template-columns:58px 1fr 58px}.preset-row{grid-template-columns:repeat(3,1fr)}.panel-head{flex-direction:column}.gauge{height:230px}.brand h1{font-size:30px}}\n";

static const char APP_JS[] =
"const state={history:[],maxSamples:180,min:0,max:6000,lastSetpoint:500,lastRunSetpoint:500,sending:false,pending:null,paused:false,dirty:false};\n"
"const $=id=>document.getElementById(id);\n"
"const fmt=n=>Number.isFinite(n)?Math.round(n).toLocaleString('es-CO'):'--';\n"
"function clamp(v){return Math.max(state.min,Math.min(state.max,Math.round(v)));}\n"
"function setConn(ok,text){const el=$('connectionState');el.textContent=text;el.className=ok?'ok':'fail';}\n"
"function setMsg(text,cls){const m=$('formMessage');m.textContent=text;m.className='form-message '+(cls||'');}\n"
"function syncInputs(v,force,dirty){state.lastSetpoint=clamp(v);if(dirty)state.dirty=true;if(state.lastSetpoint>0)state.lastRunSetpoint=state.lastSetpoint;$('setpointRange').min=state.min;$('setpointRange').max=state.max;$('setpointInput').min=state.min;$('setpointInput').max=state.max;$('setpointRange').value=state.lastSetpoint;if(force||document.activeElement!==$('setpointInput'))$('setpointInput').value=state.lastSetpoint;}\n"
"function draw(){const c=$('historyChart'),ctx=c.getContext('2d'),w=c.width,h=c.height;ctx.clearRect(0,0,w,h);ctx.fillStyle='#0b111b';ctx.fillRect(0,0,w,h);ctx.strokeStyle='#263244';ctx.lineWidth=1;for(let i=1;i<5;i++){let y=h*i/5;ctx.beginPath();ctx.moveTo(0,y);ctx.lineTo(w,y);ctx.stroke();}if(state.history.length<2)return;const max=Math.max(state.max,...state.history.map(p=>p.rpm),...state.history.map(p=>p.setpoint));const x=i=>i*(w/(state.maxSamples-1));const y=v=>h-(v/max)*(h-20)-10;ctx.lineWidth=3;ctx.strokeStyle='#30d5ff';ctx.beginPath();state.history.forEach((p,i)=>{const xx=x(i),yy=y(p.rpm);i?ctx.lineTo(xx,yy):ctx.moveTo(xx,yy)});ctx.stroke();ctx.setLineDash([8,8]);ctx.strokeStyle='#67f0a0';ctx.beginPath();state.history.forEach((p,i)=>{const xx=x(i),yy=y(p.setpoint);i?ctx.lineTo(xx,yy):ctx.moveTo(xx,yy)});ctx.stroke();ctx.setLineDash([]);}\n"
"function render(data){state.min=data.min_setpoint;state.max=data.max_setpoint;$('rpmValue').textContent=fmt(data.rpm);$('rpmMini').textContent=fmt(data.rpm);$('setpointValue').textContent=fmt(data.setpoint);$('dutyValue').textContent=fmt(data.duty);$('outputValue').textContent=fmt(data.output);$('commandValue').textContent=fmt(data.command);$('errorValue').textContent=fmt(data.setpoint-data.rpm);$('modeValue').textContent=data.mode||'--';if(data.setpoint>0)state.paused=false;if(data.setpoint>0&&!state.sending&&!state.dirty)syncInputs(data.setpoint,false,false);setMsg(state.dirty?'Valor listo, presiona Aplicar RPM':`Rango permitido: ${fmt(data.min_setpoint)} a ${fmt(data.max_setpoint)} RPM`,state.dirty?'ok':'');const ratio=Math.max(0,Math.min(1,data.rpm/Math.max(data.max_setpoint,1)));$('needle').style.transform=`translate(-50%,-100%) rotate(${-115+ratio*230}deg)`;$('rpmBar').style.width=`${ratio*100}%`;state.history.push({rpm:data.rpm,setpoint:data.setpoint});if(state.history.length>state.maxSamples)state.history.shift();$('samplesValue').textContent=`${state.history.length} muestras`;draw();}\n"
"async function sendSetpoint(rpm){rpm=clamp(rpm);if(rpm>0){state.paused=false;state.lastRunSetpoint=rpm;}if(state.sending){state.pending=rpm;return;}state.sending=true;try{const r=await fetch('/api/setpoint',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({rpm})});const data=await r.json();if(!r.ok)throw new Error(data.error||'No aplicado');state.dirty=false;setMsg(rpm>0?'Referencia aplicada':'Sistema en pausa','ok');render(data);}catch(e){setMsg(e.message,'fail');}finally{state.sending=false;if(state.pending!==null){const p=state.pending;state.pending=null;sendSetpoint(p);}}}\n"
"async function poll(){try{const r=await fetch('/api/status',{cache:'no-store'});if(!r.ok)throw new Error('HTTP '+r.status);render(await r.json());setConn(true,'En linea');}catch(e){setConn(false,'Sin conexion');}}\n"
"$('setpointRange').addEventListener('input',e=>syncInputs(Number(e.target.value),true,true));\n"
"$('setpointInput').addEventListener('input',e=>{const v=Number(e.target.value);if(Number.isFinite(v))syncInputs(v,false,true);});\n"
"document.querySelectorAll('[data-step]').forEach(b=>b.addEventListener('click',()=>syncInputs(state.lastSetpoint+Number(b.dataset.step),true,true)));\n"
"document.querySelectorAll('[data-rpm]').forEach(b=>b.addEventListener('click',()=>syncInputs(Number(b.dataset.rpm),true,true)));\n"
"$('startButton').addEventListener('click',()=>sendSetpoint(state.lastSetpoint>0?state.lastSetpoint:state.lastRunSetpoint));\n"
"$('pauseButton').addEventListener('click',()=>{state.paused=true;sendSetpoint(0);});\n"
"$('setpointForm').addEventListener('submit',ev=>{ev.preventDefault();sendSetpoint(Number($('setpointInput').value));});\n"
"poll();setInterval(poll,250);\n";

static const char ICON_SVG[] =
"<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 128 128\"><defs><linearGradient id=\"g\" x1=\"18\" y1=\"108\" x2=\"110\" y2=\"20\" gradientUnits=\"userSpaceOnUse\"><stop stop-color=\"#32d7ff\"/><stop offset=\"1\" stop-color=\"#66f2a5\"/></linearGradient></defs><rect width=\"128\" height=\"128\" rx=\"24\" fill=\"#080a0f\"/><path d=\"M25 82a43 43 0 1 1 78 0\" fill=\"none\" stroke=\"url(#g)\" stroke-width=\"12\" stroke-linecap=\"round\"/><path d=\"M64 69 92 39\" stroke=\"#eff6ff\" stroke-width=\"8\" stroke-linecap=\"round\"/><circle cx=\"64\" cy=\"72\" r=\"10\" fill=\"#eff6ff\"/><path d=\"M32 94h64\" stroke=\"#263244\" stroke-width=\"8\" stroke-linecap=\"round\"/></svg>\n";

static esp_err_t send_text(httpd_req_t *req, const char *type, const char *body) {
    httpd_resp_set_type(req, type);
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    return httpd_resp_send(req, body, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t index_get_handler(httpd_req_t *req) {
    return send_text(req, "text/html; charset=utf-8", INDEX_HTML);
}

static esp_err_t css_get_handler(httpd_req_t *req) {
    return send_text(req, "text/css; charset=utf-8", STYLES_CSS);
}

static esp_err_t js_get_handler(httpd_req_t *req) {
    return send_text(req, "application/javascript; charset=utf-8", APP_JS);
}

static esp_err_t icon_get_handler(httpd_req_t *req) {
    return send_text(req, "image/svg+xml", ICON_SVG);
}

static esp_err_t health_get_handler(httpd_req_t *req) {
    return send_text(req, "application/json", "{\"status\":\"ok\"}");
}

static esp_err_t send_status(httpd_req_t *req) {
    control_status_t status;
    char body[256];

    control_get_status(&status);
    snprintf(body, sizeof(body),
             "{\"rpm\":%.2f,\"setpoint\":%.2f,\"output\":%.2f,\"command\":%.2f,"
             "\"duty\":%.2f,\"min_setpoint\":%.2f,\"max_setpoint\":%.2f,\"mode\":\"%s\"}",
             status.rpm, status.setpoint, status.output, status.command, status.duty,
             status.min_setpoint, status.max_setpoint,
             status.mode != NULL ? status.mode : "UNKNOWN");

    return send_text(req, "application/json", body);
}

static esp_err_t status_get_handler(httpd_req_t *req) {
    return send_status(req);
}

static esp_err_t setpoint_get_handler(httpd_req_t *req) {
    control_status_t status;
    char body[96];

    control_get_status(&status);
    snprintf(body, sizeof(body),
             "{\"setpoint\":%.2f,\"min_setpoint\":%.2f,\"max_setpoint\":%.2f}",
             status.setpoint, status.min_setpoint, status.max_setpoint);

    return send_text(req, "application/json", body);
}

static esp_err_t bad_request(httpd_req_t *req, const char *message) {
    char body[96];
    httpd_resp_set_status(req, "400 Bad Request");
    snprintf(body, sizeof(body), "{\"error\":\"%s\"}", message);
    return send_text(req, "application/json", body);
}

static esp_err_t setpoint_post_handler(httpd_req_t *req) {
    if (req->content_len == 0 || req->content_len >= 128) {
        return bad_request(req, "cuerpo invalido");
    }

    char body[128];
    size_t received = 0;
    while (received < req->content_len) {
        int chunk = httpd_req_recv(req, body + received, req->content_len - received);
        if (chunk <= 0) {
            if (chunk == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            return bad_request(req, "no se pudo leer el cuerpo");
        }
        received += (size_t)chunk;
    }
    body[received] = '\0';

    cJSON *root = cJSON_Parse(body);
    if (root == NULL) {
        return bad_request(req, "json invalido");
    }

    cJSON *rpm_item = cJSON_GetObjectItemCaseSensitive(root, "rpm");
    if (!cJSON_IsNumber(rpm_item)) {
        cJSON_Delete(root);
        return bad_request(req, "rpm requerido");
    }

    float rpm = (float)rpm_item->valuedouble;
    cJSON_Delete(root);

    if (rpm < MIN_RPM_SETPOINT || rpm > MAX_RPM_SETPOINT) {
        return bad_request(req, "rpm fuera de rango");
    }

    control_set_setpoint(rpm);
    ESP_LOGI(TAG, "Referencia HTTP actualizada: %.0f RPM", rpm);
    return send_status(req);
}

esp_err_t rpm_http_server_start(void) {
    if (server != NULL) {
        return ESP_OK;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    config.max_uri_handlers = 12;
    config.uri_match_fn = httpd_uri_match_wildcard;

    esp_err_t err = httpd_start(&server, &config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "No se pudo iniciar HTTP: %s", esp_err_to_name(err));
        return err;
    }

    const httpd_uri_t routes[] = {
        {.uri = "/", .method = HTTP_GET, .handler = index_get_handler},
        {.uri = "/styles.css", .method = HTTP_GET, .handler = css_get_handler},
        {.uri = "/app.js", .method = HTTP_GET, .handler = js_get_handler},
        {.uri = "/icon.svg", .method = HTTP_GET, .handler = icon_get_handler},
        {.uri = "/favicon.ico", .method = HTTP_GET, .handler = icon_get_handler},
        {.uri = "/health", .method = HTTP_GET, .handler = health_get_handler},
        {.uri = "/api/status", .method = HTTP_GET, .handler = status_get_handler},
        {.uri = "/api/setpoint", .method = HTTP_GET, .handler = setpoint_get_handler},
        {.uri = "/api/setpoint", .method = HTTP_POST, .handler = setpoint_post_handler},
    };

    for (size_t i = 0; i < sizeof(routes) / sizeof(routes[0]); i++) {
        err = httpd_register_uri_handler(server, &routes[i]);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "No se pudo registrar ruta %s: %s", routes[i].uri, esp_err_to_name(err));
            httpd_stop(server);
            server = NULL;
            return err;
        }
    }

    ESP_LOGI(TAG, "Servidor HTTP iniciado");
    return ESP_OK;
}
