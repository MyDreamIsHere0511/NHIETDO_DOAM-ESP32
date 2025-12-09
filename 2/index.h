#ifndef INDEX_H
#define INDEX_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
  <title>ESP32 SCIENTIFIC MONITOR</title>
  <style>
    :root {
      --bg: #0b0e14;
      --card-bg: rgba(22, 27, 34, 0.7);
      --text-main: #e6edf3;
      --text-sub: #7d8590;
      --accent-temp: #f78166; /* Cam San Hô */
      --accent-hum: #58a6ff;  /* Xanh Khoa Học */
      --border: 1px solid rgba(48, 54, 61, 0.7);
      --shadow: 0 10px 30px -10px rgba(0,0,0,0.5);
    }

    body {
      font-family: 'SF Pro Display', -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
      background-color: var(--bg);
      color: var(--text-main);
      margin: 0;
      padding: 20px;
      background-image: radial-gradient(circle at 50% 0%, #161b22 0%, #0b0e14 100%);
      min-height: 100vh;
    }

    /* HEADER */
    header {
      display: flex; justify-content: space-between; align-items: center;
      max-width: 900px; margin: 0 auto 30px auto;
      border-bottom: 1px solid rgba(255,255,255,0.1);
      padding-bottom: 15px;
    }
    .brand { font-size: 1.2rem; font-weight: 700; letter-spacing: 1px; color: var(--text-main); }
    .status-badge {
      display: flex; align-items: center; gap: 8px;
      font-size: 0.8rem; color: #3fb950; background: rgba(63, 185, 80, 0.1);
      padding: 5px 10px; border-radius: 20px; border: 1px solid rgba(63, 185, 80, 0.2);
    }
    .dot { width: 8px; height: 8px; background: #3fb950; border-radius: 50%; animation: pulse 2s infinite; }
    
    /* LAYOUT */
    .dashboard {
      display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
      gap: 20px; max-width: 900px; margin: 0 auto;
    }

    /* CARD STYLE (GLASSMORPHISM) */
    .card {
      background: var(--card-bg);
      backdrop-filter: blur(10px);
      -webkit-backdrop-filter: blur(10px);
      border: var(--border);
      border-radius: 16px;
      padding: 25px;
      box-shadow: var(--shadow);
      position: relative;
      overflow: hidden;
    }

    /* METRIC CARDS */
    .card-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 15px; }
    .card-title { font-size: 0.9rem; font-weight: 600; color: var(--text-sub); text-transform: uppercase; letter-spacing: 1px; }
    .icon { font-size: 1.5rem; opacity: 0.8; }
    
    .value-box { display: flex; align-items: baseline; gap: 5px; }
    .value { font-size: 3.5rem; font-weight: 300; line-height: 1; text-shadow: 0 0 20px rgba(0,0,0,0.5); }
    .unit { font-size: 1.5rem; color: var(--text-sub); font-weight: 400; }

    .stats-row {
      display: flex; justify-content: space-between; margin-top: 20px;
      padding-top: 15px; border-top: 1px solid rgba(255,255,255,0.05);
    }
    .stat-item { display: flex; flex-direction: column; font-size: 0.8rem; color: var(--text-sub); }
    .stat-val { font-size: 1rem; font-weight: 600; color: var(--text-main); margin-top: 2px; }

    /* COLOR THEMES */
    .temp-theme .value { color: var(--accent-temp); text-shadow: 0 0 15px rgba(247, 129, 102, 0.3); }
    .temp-theme .icon { color: var(--accent-temp); }
    .hum-theme .value { color: var(--accent-hum); text-shadow: 0 0 15px rgba(88, 166, 255, 0.3); }
    .hum-theme .icon { color: var(--accent-hum); }

    /* CHART CARD */
    .chart-card { grid-column: 1 / -1; height: 350px; display: flex; flex-direction: column; }
    .chart-container { flex-grow: 1; position: relative; width: 100%; margin-top: 10px; }
    
    /* SVG CHART STYLES */
    svg { width: 100%; height: 100%; overflow: visible; }
    .grid-line { stroke: rgba(255,255,255,0.05); stroke-width: 1; }
    .chart-line { fill: none; stroke-width: 2.5; stroke-linecap: round; stroke-linejoin: round; vector-effect: non-scaling-stroke; }
    .area-fill { stroke: none; opacity: 0.15; }
    
    .line-temp { stroke: var(--accent-temp); filter: drop-shadow(0 0 4px rgba(247, 129, 102, 0.5)); }
    .fill-temp { fill: var(--accent-temp); }
    
    .line-hum { stroke: var(--accent-hum); filter: drop-shadow(0 0 4px rgba(88, 166, 255, 0.5)); }
    .fill-hum { fill: var(--accent-hum); }

    /* FOOTER */
    footer { text-align: center; color: var(--text-sub); margin-top: 40px; font-size: 0.8rem; }

    @keyframes pulse { 0% { opacity: 1; } 50% { opacity: 0.4; } 100% { opacity: 1; } }
    @media (max-width: 600px) { .value { font-size: 3rem; } .chart-card { height: 250px; } }
  </style>
</head>
<body>

  <header>
    <div class="brand">IOT MONITOR STATION</div>
    <div class="status-badge"><div class="dot"></div> LIVE DATA</div>
  </header>

  <div class="dashboard">
    <div class="card temp-theme">
      <div class="card-header">
        <div class="card-title">Temperature</div>
        <div class="icon">🌡️</div>
      </div>
      <div class="value-box">
        <span id="temp" class="value">--</span><span class="unit">°C</span>
      </div>
      <div class="stats-row">
        <div class="stat-item">MIN <span id="minT" class="stat-val">--</span></div>
        <div class="stat-item" style="text-align: right;">MAX <span id="maxT" class="stat-val">--</span></div>
      </div>
    </div>

    <div class="card hum-theme">
      <div class="card-header">
        <div class="card-title">Humidity</div>
        <div class="icon">💧</div>
      </div>
      <div class="value-box">
        <span id="hum" class="value">--</span><span class="unit">%</span>
      </div>
      <div class="stats-row">
        <div class="stat-item">MIN <span id="minH" class="stat-val">--</span></div>
        <div class="stat-item" style="text-align: right;">MAX <span id="maxH" class="stat-val">--</span></div>
      </div>
    </div>

    <div class="card chart-card">
      <div class="card-header">
        <div class="card-title">Environmental Trends</div>
        <div style="font-size: 0.8rem; color: var(--text-sub);">Last 20 Samples</div>
      </div>
      <div class="chart-container">
        <svg id="chart" viewBox="0 0 100 100" preserveAspectRatio="none">
          <line x1="0" y1="25" x2="100" y2="25" class="grid-line" />
          <line x1="0" y1="50" x2="100" y2="50" class="grid-line" />
          <line x1="0" y1="75" x2="100" y2="75" class="grid-line" />
          
          <path id="pathAreaT" class="area-fill fill-temp" d="" />
          <path id="pathLineT" class="chart-line line-temp" d="" />
          
          <path id="pathAreaH" class="area-fill fill-hum" d="" />
          <path id="pathLineH" class="chart-line line-hum" d="" />
        </svg>
      </div>
    </div>
  </div>

  <footer>
    Designed for ESP32 & AHT30 Sensor
  </footer>

  <script>
    // --- CONFIG ---
    const MAX_POINTS = 20;
    let dataT = new Array(MAX_POINTS).fill(0);
    let dataH = new Array(MAX_POINTS).fill(0);
    let minT=100, maxT=0, minH=100, maxH=0;
    let firstRun = true;

    function updateStats(t, h) {
      if(firstRun) { minT=t; maxT=t; minH=h; maxH=h; firstRun=false; }
      if(t < minT) minT = t; if(t > maxT) maxT = t;
      if(h < minH) minH = h; if(h > maxH) maxH = h;
      
      document.getElementById('minT').innerText = minT.toFixed(1) + '°';
      document.getElementById('maxT').innerText = maxT.toFixed(1) + '°';
      document.getElementById('minH').innerText = minH.toFixed(0) + '%';
      document.getElementById('maxH').innerText = maxH.toFixed(0) + '%';
    }

    function drawChart() {
      // Scale: Temp (0-60), Hum (0-100) mapped to SVG (100-0)
      const getY_T = (val) => 100 - ((val / 60) * 100); 
      const getY_H = (val) => 100 - ((val / 100) * 100);
      const step = 100 / (MAX_POINTS - 1);

      let dLineT = "", dAreaT = `M0,100 `;
      let dLineH = "", dAreaH = `M0,100 `;

      for(let i=0; i<MAX_POINTS; i++) {
        let x = i * step;
        let yT = getY_T(dataT[i]);
        let yH = getY_H(dataH[i]);

        dLineT += (i===0 ? "M" : "L") + `${x},${yT} `;
        dAreaT += `L${x},${yT} `;
        
        dLineH += (i===0 ? "M" : "L") + `${x},${yH} `;
        dAreaH += `L${x},${yH} `;
      }
      dAreaT += `L100,100 Z`;
      dAreaH += `L100,100 Z`;

      document.getElementById('pathLineT').setAttribute('d', dLineT);
      document.getElementById('pathAreaT').setAttribute('d', dAreaT);
      document.getElementById('pathLineH').setAttribute('d', dLineH);
      document.getElementById('pathAreaH').setAttribute('d', dAreaH);
    }

    setInterval(function() {
      fetch("/readings")
        .then(response => response.json())
        .then(obj => {
          // Update Numbers
          document.getElementById("temp").innerText = obj.temperature.toFixed(1);
          document.getElementById("hum").innerText = obj.humidity.toFixed(0);
          
          // Update Stats
          updateStats(obj.temperature, obj.humidity);

          // Update Chart Data (Shift & Push)
          dataT.shift(); dataT.push(obj.temperature);
          dataH.shift(); dataH.push(obj.humidity);
          drawChart();
        })
        .catch(err => console.log('Connection Lost'));
    }, 2000);
    
    // Init Chart
    drawChart();
  </script>
</body>
</html>
)rawliteral";

#endif