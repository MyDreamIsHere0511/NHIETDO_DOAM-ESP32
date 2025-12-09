#ifndef INDEX_H
#define INDEX_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
  <title>ESP32 MONITOR</title>
  <style>
    /* --- CẤU HÌNH MÀU SẮC --- */
    :root {
      --bg-dark: #050505;
      --card-bg: rgba(20, 20, 20, 0.6);
      --text-main: #ffffff;
      --text-sub: #888888;
      
      /* Màu Neon rực rỡ */
      --neon-temp: #ff5e3a; /* Cam rực */
      --neon-hum: #00d2ff;  /* Xanh rực */
    }

    body {
      font-family: 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
      background-color: var(--bg-dark);
      color: var(--text-main);
      margin: 0;
      padding: 20px;
      /* Nền tối có đốm sáng nhẹ */
      background-image: radial-gradient(circle at 50% 30%, #1a1a1a 0%, #000000 100%);
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
    }

    /* HEADER */
    header {
      width: 100%; max-width: 800px;
      display: flex; justify-content: space-between; align-items: center;
      margin-bottom: 40px; padding-bottom: 20px;
      border-bottom: 1px solid rgba(255,255,255,0.1);
    }
    .brand { font-size: 1.5rem; font-weight: 800; letter-spacing: 2px; color: var(--text-main); text-transform: uppercase; }
    .live-indicator {
      display: flex; align-items: center; gap: 8px;
      font-size: 0.9rem; color: #00ff88; font-weight: bold;
      text-shadow: 0 0 10px rgba(0, 255, 136, 0.5);
    }
    .blink { width: 10px; height: 10px; background: #00ff88; border-radius: 50%; animation: blinker 1.5s linear infinite; box-shadow: 0 0 10px #00ff88; }

    /* LAYOUT CONTAINER */
    .container {
      display: flex; flex-wrap: wrap; justify-content: center; gap: 30px;
      width: 100%; max-width: 900px;
    }

    /* CARD STYLE (KÍNH MỜ CAO CẤP) */
    .card {
      flex: 1 1 350px; /* Co giãn linh hoạt */
      background: var(--card-bg);
      border: 1px solid rgba(255,255,255,0.1);
      border-radius: 20px;
      padding: 40px 30px;
      text-align: center;
      position: relative;
      overflow: hidden;
      backdrop-filter: blur(15px);
      -webkit-backdrop-filter: blur(15px);
      box-shadow: 0 20px 50px rgba(0,0,0,0.5);
      transition: transform 0.3s ease;
    }
    .card:hover { transform: translateY(-5px); border-color: rgba(255,255,255,0.3); }

    /* ICON & TITLE */
    .icon { font-size: 3rem; margin-bottom: 15px; display: block; opacity: 0.9; }
    .label { font-size: 1.2rem; font-weight: 600; color: var(--text-sub); text-transform: uppercase; letter-spacing: 2px; margin-bottom: 10px; }

    /* GIÁ TRỊ SỐ (BIG NUMBER) */
    .value-wrapper {
      display: flex; justify-content: center; align-items: flex-start;
      margin: 10px 0;
    }
    .value {
      font-size: 7rem; /* Cực lớn */
      font-weight: 700;
      line-height: 1;
      font-variant-numeric: tabular-nums; /* Số thẳng hàng */
    }
    .unit {
      font-size: 2.5rem;
      font-weight: 300;
      margin-top: 15px;
      margin-left: 5px;
      opacity: 0.8;
    }

    /* STATS (MIN/MAX) */
    .stats {
      display: flex; justify-content: space-between;
      margin-top: 30px; padding-top: 20px;
      border-top: 1px solid rgba(255,255,255,0.1);
      font-size: 1rem; color: var(--text-sub);
    }
    .stat-val { font-weight: bold; color: #fff; }

    /* --- HIỆU ỨNG MÀU RIÊNG BIỆT --- */
    
    /* Card Nhiệt độ (Cam Lửa) */
    .card.temp .value { color: var(--neon-temp); text-shadow: 0 0 30px rgba(255, 94, 58, 0.6); }
    .card.temp .icon { text-shadow: 0 0 20px rgba(255, 94, 58, 0.8); }
    
    /* Card Độ ẩm (Xanh Băng) */
    .card.hum .value { color: var(--neon-hum); text-shadow: 0 0 30px rgba(0, 210, 255, 0.6); }
    .card.hum .icon { text-shadow: 0 0 20px rgba(0, 210, 255, 0.8); }

    /* FOOTER */
    footer { margin-top: 50px; color: var(--text-sub); font-size: 0.85rem; opacity: 0.6; }

    @keyframes blinker { 50% { opacity: 0; } }
    @media (max-width: 600px) { 
      .value { font-size: 5rem; } 
      .container { gap: 20px; }
      .card { padding: 30px 20px; }
    }
  </style>
</head>
<body>

  <header>
    <div class="brand">SYSTEM MONITOR</div>
    <div class="live-indicator"><div class="blink"></div> ONLINE</div>
  </header>

  <div class="container">
    <div class="card temp">
      <span class="icon">☀️</span>
      <div class="label">Nhiệt Độ</div>
      <div class="value-wrapper">
        <span id="temp" class="value">--</span><span class="unit">°C</span>
      </div>
      <div class="stats">
        <span>Min: <span id="minT" class="stat-val">--</span></span>
        <span>Max: <span id="maxT" class="stat-val">--</span></span>
      </div>
    </div>

    <div class="card hum">
      <span class="icon">💧</span>
      <div class="label">Độ Ẩm</div>
      <div class="value-wrapper">
        <span id="hum" class="value">--</span><span class="unit">%</span>
      </div>
      <div class="stats">
        <span>Min: <span id="minH" class="stat-val">--</span></span>
        <span>Max: <span id="maxH" class="stat-val">--</span></span>
      </div>
    </div>
  </div>

  <footer>
    ESP32 IoT Dashboard &bull; Designed by Gemini
  </footer>

  <script>
    let minT = 100, maxT = -50, minH = 100, maxH = 0;
    let firstRun = true;

    function updateStats(t, h) {
      // Bỏ qua nếu lỗi
      if(isNaN(t) || isNaN(h)) return;

      // Cập nhật Min/Max
      if(firstRun) { minT=t; maxT=t; minH=h; maxH=h; firstRun=false; }
      else {
        if(t < minT) minT = t; if(t > maxT) maxT = t;
        if(h < minH) minH = h; if(h > maxH) maxH = h;
      }
      
      document.getElementById('minT').innerText = minT.toFixed(1) + '°';
      document.getElementById('maxT').innerText = maxT.toFixed(1) + '°';
      document.getElementById('minH').innerText = minH.toFixed(0) + '%';
      document.getElementById('maxH').innerText = maxH.toFixed(0) + '%';
    }

    // Hàm lấy dữ liệu định kỳ
    setInterval(function() {
      fetch("/readings")
        .then(response => response.json())
        .then(obj => {
          let t = parseFloat(obj.temperature);
          let h = parseFloat(obj.humidity);

          // Cập nhật số to
          document.getElementById("temp").innerText = t.toFixed(1);
          document.getElementById("hum").innerText = h.toFixed(0);
          
          // Cập nhật thống kê
          updateStats(t, h);
        })
        .catch(err => console.log('Mất kết nối server'));
    }, 2000); // 2 giây
  </script>
</body>
</html>
)rawliteral";

#endif