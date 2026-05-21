const form = document.getElementById("configForm");
const startButton = document.getElementById("startButton");
const stopButton = document.getElementById("stopButton");
const statePill = document.getElementById("statePill");
const statusText = document.getElementById("statusText");
const bufferMeter = document.getElementById("bufferMeter");
const audioMeter = document.getElementById("audioMeter");
const bufferValue = document.getElementById("bufferValue");
const audioValue = document.getElementById("audioValue");
const sampleRate = document.getElementById("sampleRate");
const bandwidth = document.getElementById("bandwidth");
const blocksRead = document.getElementById("blocksRead");
const underruns = document.getElementById("underruns");
const dropped = document.getElementById("dropped");
const psdRange = document.getElementById("psdRange");
const audioPeak = document.getElementById("audioPeak");
const psdCanvas = document.getElementById("psdCanvas");
const audioCanvas = document.getElementById("audioCanvas");
const nfftInput = document.getElementById("nfft");
const npersegInput = document.getElementById("nperseg");

let busy = false;
let lastSnapshot = null;

function $(id) {
  return document.getElementById(id);
}

function syncInputs(source, target) {
  source.addEventListener("input", () => {
    target.value = source.value;
  });
}

syncInputs(nfftInput, npersegInput);
syncInputs(npersegInput, nfftInput);

form.addEventListener("submit", async (event) => {
  event.preventDefault();
  await startOrApply();
});

stopButton.addEventListener("click", async () => {
  await postJSON("/api/stop", {});
});

async function startOrApply() {
  const payload = {
    fc_mhz: $("fc_mhz").value,
    span_mhz: $("span_mhz").value,
    nfft: $("nfft").value,
    nperseg: $("nperseg").value,
    noverlap: $("noverlap").value,
    fine_tune_khz: $("fine_tune_khz").value,
    freq_correction_ppm: $("freq_correction_ppm").value,
    gain_db: $("gain_db").value,
    deemphasis_us: $("deemphasis_us").value,
  };
  await postJSON("/api/start", payload);
}

async function postJSON(url, payload) {
  if (busy) return;
  busy = true;
  startButton.disabled = true;
  stopButton.disabled = true;
  try {
    const response = await fetch(url, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload),
    });
    const data = await response.json();
    if (!response.ok || data.error) {
      throw new Error(data.error || "No se pudo completar la accion.");
    }
    updateStatus(data);
  } catch (error) {
    statusText.textContent = `Error: ${error.message}`;
    statePill.textContent = "Error";
    statePill.className = "state-pill warn";
  } finally {
    busy = false;
    startButton.disabled = false;
    stopButton.disabled = false;
  }
}

async function loadInitialStatus() {
  try {
    const status = await fetchJSON("/api/status");
    updateStatus(status);
    setForm(status.config);
  } catch (error) {
    statusText.textContent = `No se pudo conectar: ${error.message}`;
    statePill.textContent = "Sin conexion";
    statePill.className = "state-pill warn";
  }
}

async function pollSnapshot() {
  try {
    const snapshot = await fetchJSON("/api/snapshot");
    lastSnapshot = snapshot;
    updateStatus(snapshot.status);
    drawPSD(snapshot.psd);
    drawAudio(snapshot.audio, snapshot.status);
  } catch (error) {
    statusText.textContent = `Servidor no disponible: ${error.message}`;
    statePill.textContent = "Sin conexion";
    statePill.className = "state-pill warn";
  } finally {
    window.setTimeout(pollSnapshot, 500);
  }
}

async function fetchJSON(url) {
  const response = await fetch(url, { cache: "no-store" });
  const data = await response.json();
  if (!response.ok || data.error) {
    throw new Error(data.error || "Respuesta invalida.");
  }
  return data;
}

function setForm(config) {
  $("fc_mhz").value = Number(config.fc_mhz).toFixed(3);
  $("span_mhz").value = Number(config.span_mhz).toFixed(3);
  $("nfft").value = config.nfft;
  $("nperseg").value = config.nperseg;
  $("noverlap").value = config.noverlap;
  $("fine_tune_khz").value = Number(config.fine_tune_khz).toFixed(1);
  $("freq_correction_ppm").value = config.freq_correction_ppm;
  $("gain_db").value = Number(config.gain_db).toFixed(1);
  $("deemphasis_us").value = String(Math.round(Number(config.deemphasis_us)));
}

function updateStatus(status) {
  if (!status || !status.config) return;
  statusText.textContent = status.status;
  if (status.waiting_device) {
    statePill.textContent = "Buscando SDR";
    statePill.className = "state-pill warn";
  } else if (status.running) {
    statePill.textContent = status.audio_started ? "En vivo" : "Buffer";
    statePill.className = "state-pill live";
  } else {
    statePill.textContent = "Detenido";
    statePill.className = "state-pill idle";
  }

  const bufferPercent = clamp((status.buffer_ms / Math.max(status.buffer_max_ms, 1)) * 100, 0, 100);
  const audioPercent = clamp(status.audio_peak * 100, 0, 100);
  bufferMeter.style.width = `${bufferPercent}%`;
  audioMeter.style.width = `${audioPercent}%`;
  bufferValue.textContent = `${Math.round(status.buffer_ms)} ms`;
  audioValue.textContent = `${Math.round(audioPercent)}%`;
  sampleRate.textContent = `${(status.config.sample_rate_hz / 1e6).toFixed(2)} MS/s`;
  bandwidth.textContent = `${Number(status.config.span_mhz).toFixed(2)} MHz`;
  blocksRead.textContent = status.blocks_read;
  underruns.textContent = status.underruns;
  dropped.textContent = `${Math.round(status.dropped_ms)} ms`;
  audioPeak.textContent = `Pico ${Number(status.audio_peak).toFixed(2)}`;
}

function drawPSD(psd) {
  const x = psd?.freq_mhz || [];
  const y = psd?.db || [];
  const ctx = prepareCanvas(psdCanvas);
  const box = chartBox(psdCanvas);
  clearCanvas(ctx, psdCanvas);
  drawGrid(ctx, box, "PSD [dB/Hz]", "Frecuencia [MHz]");

  if (!x.length || !y.length) {
    drawEmpty(ctx, box, "Esperando muestras IQ para PSD");
    psdRange.textContent = "-- MHz";
    return;
  }

  const xMin = Math.min(...x);
  const xMax = Math.max(...x);
  const yMin = -150;
  const yMax = -20;
  psdRange.textContent = `${xMin.toFixed(3)} - ${xMax.toFixed(3)} MHz`;

  drawLine(ctx, box, x, y, xMin, xMax, yMin, yMax, {
    stroke: "#00a6a6",
    fill: "rgba(0, 166, 166, 0.13)",
    width: 2,
  });
  drawAxisLabels(ctx, box, xMin, xMax, yMin, yMax, "MHz", "dB");
}

function drawAudio(audio, status) {
  const x = audio?.time_ms || [];
  const y = audio?.amplitude || [];
  const ctx = prepareCanvas(audioCanvas);
  const box = chartBox(audioCanvas);
  clearCanvas(ctx, audioCanvas);
  drawGrid(ctx, box, "Amplitud", "Tiempo [ms]");

  if (!x.length || !y.length || !status?.running) {
    drawEmpty(ctx, box, status?.running ? "Esperando audio demodulado" : "Audio detenido");
    return;
  }

  drawLine(ctx, box, x, y, 0, Math.max(...x, 1), -1.05, 1.05, {
    stroke: "#7557d8",
    fill: "rgba(117, 87, 216, 0.12)",
    width: 2,
  });
  drawZeroLine(ctx, box);
  drawAxisLabels(ctx, box, 0, Math.max(...x, 1), -1, 1, "ms", "");
}

function prepareCanvas(canvas) {
  const rect = canvas.getBoundingClientRect();
  const ratio = window.devicePixelRatio || 1;
  const width = Math.max(320, Math.floor(rect.width * ratio));
  const height = Math.max(220, Math.floor(rect.height * ratio));
  if (canvas.width !== width || canvas.height !== height) {
    canvas.width = width;
    canvas.height = height;
  }
  const ctx = canvas.getContext("2d");
  ctx.setTransform(ratio, 0, 0, ratio, 0, 0);
  return ctx;
}

function chartBox(canvas) {
  const rect = canvas.getBoundingClientRect();
  return {
    x: 58,
    y: 18,
    w: Math.max(120, rect.width - 82),
    h: Math.max(120, rect.height - 58),
  };
}

function clearCanvas(ctx, canvas) {
  const rect = canvas.getBoundingClientRect();
  ctx.clearRect(0, 0, rect.width, rect.height);
  ctx.fillStyle = "#fbfdfe";
  ctx.fillRect(0, 0, rect.width, rect.height);
}

function drawGrid(ctx, box, yLabel, xLabel) {
  ctx.save();
  ctx.strokeStyle = "#dbe4ea";
  ctx.lineWidth = 1;
  ctx.fillStyle = "#64717c";
  ctx.font = "12px Segoe UI, Arial";
  for (let i = 0; i <= 5; i += 1) {
    const y = box.y + (box.h / 5) * i;
    ctx.beginPath();
    ctx.moveTo(box.x, y);
    ctx.lineTo(box.x + box.w, y);
    ctx.stroke();
  }
  for (let i = 0; i <= 6; i += 1) {
    const x = box.x + (box.w / 6) * i;
    ctx.beginPath();
    ctx.moveTo(x, box.y);
    ctx.lineTo(x, box.y + box.h);
    ctx.stroke();
  }
  ctx.fillText(xLabel, box.x + box.w - 88, box.y + box.h + 36);
  ctx.save();
  ctx.translate(18, box.y + 98);
  ctx.rotate(-Math.PI / 2);
  ctx.fillText(yLabel, 0, 0);
  ctx.restore();
  ctx.restore();
}

function drawLine(ctx, box, x, y, xMin, xMax, yMin, yMax, style) {
  const points = x.map((value, index) => ({
    x: box.x + ((value - xMin) / Math.max(xMax - xMin, 0.000001)) * box.w,
    y: box.y + (1 - (y[index] - yMin) / Math.max(yMax - yMin, 0.000001)) * box.h,
  }));

  ctx.save();
  ctx.beginPath();
  points.forEach((point, index) => {
    if (index === 0) ctx.moveTo(point.x, point.y);
    else ctx.lineTo(point.x, point.y);
  });
  ctx.strokeStyle = style.stroke;
  ctx.lineWidth = style.width;
  ctx.lineJoin = "round";
  ctx.stroke();

  ctx.lineTo(box.x + box.w, box.y + box.h);
  ctx.lineTo(box.x, box.y + box.h);
  ctx.closePath();
  ctx.fillStyle = style.fill;
  ctx.fill();
  ctx.restore();
}

function drawZeroLine(ctx, box) {
  const y = box.y + box.h / 2;
  ctx.save();
  ctx.strokeStyle = "#e05a6f";
  ctx.setLineDash([5, 5]);
  ctx.beginPath();
  ctx.moveTo(box.x, y);
  ctx.lineTo(box.x + box.w, y);
  ctx.stroke();
  ctx.restore();
}

function drawAxisLabels(ctx, box, xMin, xMax, yMin, yMax, xUnit, yUnit) {
  ctx.save();
  ctx.fillStyle = "#64717c";
  ctx.font = "11px Segoe UI, Arial";
  ctx.fillText(`${xMin.toFixed(2)} ${xUnit}`, box.x, box.y + box.h + 18);
  ctx.fillText(`${xMax.toFixed(2)} ${xUnit}`, box.x + box.w - 72, box.y + box.h + 18);
  ctx.fillText(`${yMax.toFixed(0)} ${yUnit}`, box.x - 48, box.y + 4);
  ctx.fillText(`${yMin.toFixed(0)} ${yUnit}`, box.x - 48, box.y + box.h);
  ctx.restore();
}

function drawEmpty(ctx, box, message) {
  ctx.save();
  ctx.fillStyle = "#64717c";
  ctx.font = "600 14px Segoe UI, Arial";
  ctx.textAlign = "center";
  ctx.fillText(message, box.x + box.w / 2, box.y + box.h / 2);
  ctx.restore();
}

function clamp(value, min, max) {
  return Math.min(max, Math.max(min, value));
}

window.addEventListener("resize", () => {
  if (lastSnapshot) {
    drawPSD(lastSnapshot.psd);
    drawAudio(lastSnapshot.audio, lastSnapshot.status);
  }
});

loadInitialStatus();
pollSnapshot();
