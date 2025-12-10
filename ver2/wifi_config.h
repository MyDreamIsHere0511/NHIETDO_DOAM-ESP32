#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

const char wifi_config_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
  <title>CẤU HÌNH WIFI</title>
  <style>
    :root {
        --bg: #0f172a; --card: #1e293b; --text: #f1f5f9; --accent: #3b82f6; --danger: #ef4444;
    }
    body { font-family: 'Segoe UI', sans-serif; background-color: var(--bg); color: var(--text); margin: 0; display: flex; justify-content: center; align-items: center; min-height: 100vh; padding: 20px; }
    
    .login-card { background-color: var(--card); padding: 30px; border-radius: 12px; width: 100%; max-width: 400px; box-shadow: 0 10px 25px rgba(0,0,0,0.3); border: 1px solid #334155; }
    h2 { text-align: center; margin-bottom: 20px; font-weight: 300; letter-spacing: 1px; text-transform: uppercase; border-bottom: 1px solid #334155; padding-bottom: 15px; }
    
    label { display: block; margin-bottom: 8px; font-size: 0.9rem; color: #94a3b8; }
    select, input { width: 100%; padding: 12px; margin-bottom: 20px; background: #0f172a; border: 1px solid #334155; border-radius: 6px; color: white; font-size: 1rem; box-sizing: border-box; }
    select:focus, input:focus { outline: none; border-color: var(--accent); }
    
    .btn-group { display: flex; gap: 10px; }
    button { flex: 1; padding: 12px; border: none; border-radius: 6px; font-size: 1rem; cursor: pointer; font-weight: 600; transition: 0.2s; }
    .btn-save { background-color: var(--accent); color: white; }
    .btn-save:hover { background-color: #2563eb; }
    .btn-reset { background-color: transparent; border: 1px solid var(--danger); color: var(--danger); }
    .btn-reset:hover { background-color: var(--danger); color: white; }
    
    .status { text-align: center; margin-top: 15px; font-size: 0.9rem; color: #22c55e; min-height: 20px; }
    .spinner { display: inline-block; width: 20px; height: 20px; border: 3px solid rgba(255,255,255,.3); border-radius: 50%; border-top-color: #fff; animation: spin 1s ease-in-out infinite; margin-bottom: 10px; }
    @keyframes spin { to { transform: rotate(360deg); } }
  </style>
</head>
<body>
  <div class="login-card">
    <h2>Cấu Hình Hệ Thống</h2>
    
    <div id="loading" style="text-align:center;">
      <div class="spinner"></div><br>Đang quét mạng Wifi...
    </div>

    <div id="form" style="display:none;">
      <label>Chọn Wifi Nhà Bạn:</label>
      <select id="ssid"></select>
      
      <label>Mật Khẩu:</label>
      <input type="password" id="pass" placeholder="Nhập mật khẩu wifi...">
      
      <div class="btn-group">
        <button class="btn-save" onclick="save()">LƯU KẾT NỐI</button>
        <button class="btn-reset" onclick="restart()">KHỞI ĐỘNG LẠI</button>
      </div>
    </div>
    
    <div id="status" class="status"></div>
  </div>

  <script>
    // Tự động quét khi mở trang
    window.onload = function() {
      fetch('/scan').then(res => res.json()).then(data => {
        let select = document.getElementById('ssid');
        data.forEach(net => {
          let option = document.createElement("option");
          option.text = net; option.value = net;
          select.add(option);
        });
        document.getElementById('loading').style.display = 'none';
        document.getElementById('form').style.display = 'block';
      });
    };

    function save() {
      let ssid = document.getElementById('ssid').value;
      let pass = document.getElementById('pass').value;
      if(!ssid) return alert("Vui lòng chọn Wifi!");
      
      document.getElementById('status').innerHTML = "Đang lưu và kết nối...";
      fetch('/save', {
        method: 'POST',
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},
        body: 'ssid=' + encodeURIComponent(ssid) + '&pass=' + encodeURIComponent(pass)
      }).then(res => res.text()).then(txt => {
        alert(txt);
      });
    }

    function restart() {
      if(confirm("Bạn có chắc muốn khởi động lại?")) {
        fetch('/restart');
      }
    }
  </script>
</body>
</html>
)rawliteral";
#endif