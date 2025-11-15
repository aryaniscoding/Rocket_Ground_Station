// frontend/app.js
const evtUrl = 'http://127.0.0.1:9090/events';

const ctxAlt  = document.getElementById('altChart').getContext('2d');
const ctxPres = document.getElementById('presChart').getContext('2d');
const ctxTemp = document.getElementById('tempChart').getContext('2d');
const ctxVolt = document.getElementById('voltChart').getContext('2d');
const ctxSats = document.getElementById('satsChart').getContext('2d');
const ctxAcc  = document.getElementById('accChart').getContext('2d');

function lineChart(ctx, label, color) {
  return new Chart(ctx, {
    type: 'line',
    data: { labels: [], datasets: [{ label, data: [], borderColor: color, tension: 0.2, pointRadius: 0, borderWidth: 2 }] },
    options: {
      animation: false,
      maintainAspectRatio: false,
      scales: {
        x: { ticks: { color: '#9fb3c8' }, grid: { color: '#1b2835' } },
        y: { ticks: { color: '#9fb3c8' }, grid: { color: '#1b2835' } }
      },
      plugins: { legend: { labels: { color: '#cfe2f3' } } }
    }
  });
}

const altChart  = lineChart(ctxAlt,  'Altitude (m)',          '#4cc9f0');
const presChart = lineChart(ctxPres, 'Pressure (Pa)',         '#f4a261');
const tempChart = lineChart(ctxTemp, 'Temperature (°C)',      '#e76f51');
const voltChart = lineChart(ctxVolt, 'Voltage (V)',           '#90be6d');
const satsChart = lineChart(ctxSats, 'GNSS Satellites (cnt)', '#ffd166');
const accChart  = lineChart(ctxAcc,  'Accel/Orient (units)',  '#a78bfa');


function push(chart, t, v) {
  const maxPoints = 600; // 10 minutes @ 1 Hz
  chart.data.labels.push(t);
  chart.data.datasets[0].data.push(v);
  if (chart.data.labels.length > maxPoints) {
    chart.data.labels.shift();
    chart.data.datasets[0].data.shift();
  }
  chart.update('none');
}

function pushMulti(chart, t, arr) {
  // For simplicity, reuse one dataset; sum or first axis can be plotted; inline multi not shown
  if (arr.length > 0) push(chart, t, arr[0]);
}

const sse = new EventSource(evtUrl);
sse.onmessage = (ev) => {
  try {
    const p = JSON.parse(ev.data);
    document.getElementById('team').textContent = `Team: ${p.team_id}`;
    document.getElementById('state').textContent = `State: ${p.flight_state}`;
    document.getElementById('time').textContent = `Mission t: ${p.time_s.toFixed(1)} s`;
    document.getElementById('sats').textContent = `GNSS sats: ${p.gnss_sats}`;

    const t = p.time_s.toFixed(1);
    push(altChart,  t, p.altitude_m);
    push(presChart, t, p.pressure_pa);
    push(tempChart, t, p.temp_c);
    push(voltChart, t, p.voltage_v);
    push(satsChart, t, p.gnss_sats);
    pushMulti(accChart, t, p.accel_orient || []); 

    document.getElementById('lastPacket').textContent = JSON.stringify(p, null, 2);

    console.log("INCOMING PACKET JSON:", p);

  } catch (e) {
    // ignore parse errors
  }
};

sse.onerror = () => {
  // Optionally render a disconnected banner
};
