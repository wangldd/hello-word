const API_BASE = window.API_BASE || 'http://localhost:8080/api';
const output = document.querySelector('#output');
let protocols = [];

async function api(path, options = {}) {
  const response = await fetch(`${API_BASE}${path}`, {
    headers: { 'Content-Type': 'application/json' },
    ...options,
  });
  const data = await response.json();
  if (!response.ok) throw new Error(data.error || `HTTP ${response.status}`);
  return data;
}

function parseJsonField(form, name) {
  const raw = new FormData(form).get(name);
  return raw ? JSON.parse(raw) : {};
}

function renderProtocols() {
  document.querySelectorAll('.protocol-select').forEach((select) => {
    const current = select.value;
    select.innerHTML = protocols
      .map((item) => `<option value="${item.value}">${item.label}</option>`)
      .join('');
    if (current) select.value = current;
  });
  document.querySelector('#mappingForm [name="source_protocol"]').value = 'modbus';
  document.querySelector('#mappingForm [name="target_protocol"]').value = 'iec61850';
  document.querySelector('#convertForm [name="source_protocol"]').value = 'modbus';
  document.querySelector('#convertForm [name="target_protocol"]').value = 'iec61850';
}

async function refresh() {
  protocols = await api('/protocols');
  renderProtocols();
  const [channels, mappings] = await Promise.all([api('/channels'), api('/mappings')]);
  output.textContent = JSON.stringify({ protocols, channels, mappings }, null, 2);
}

function formObject(form) {
  return Object.fromEntries(new FormData(form).entries());
}

document.querySelector('#refreshBtn').addEventListener('click', refresh);

document.querySelector('#channelForm').addEventListener('submit', async (event) => {
  event.preventDefault();
  const form = event.currentTarget;
  const data = formObject(form);
  data.enabled = data.enabled === 'true';
  data.endpoint = parseJsonField(form, 'endpoint');
  data.options = parseJsonField(form, 'options');
  output.textContent = JSON.stringify(await api('/channels', { method: 'POST', body: JSON.stringify(data) }), null, 2);
  await refresh();
});

document.querySelector('#mappingForm').addEventListener('submit', async (event) => {
  event.preventDefault();
  const data = formObject(event.currentTarget);
  data.scale = Number(data.scale || 1);
  data.offset = Number(data.offset || 0);
  output.textContent = JSON.stringify(await api('/mappings', { method: 'POST', body: JSON.stringify(data) }), null, 2);
  await refresh();
});

document.querySelector('#convertForm').addEventListener('submit', async (event) => {
  event.preventDefault();
  const form = event.currentTarget;
  const data = formObject(form);
  data.payload = parseJsonField(form, 'payload');
  output.textContent = JSON.stringify(await api('/convert', { method: 'POST', body: JSON.stringify(data) }), null, 2);
});

refresh().catch((error) => {
  output.textContent = `API 连接失败：${error.message}\n请先运行：./build/backend/protocol_gateway_server --host 127.0.0.1 --port 8080`;
});
