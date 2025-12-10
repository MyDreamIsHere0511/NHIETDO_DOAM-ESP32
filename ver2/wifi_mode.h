#ifndef WIFI_MODE_H
#define WIFI_MODE_H

// ==========================================
// CÁC HÀM EEPROM (LƯU WIFI)
// ==========================================
void writeString(int add, String data)
{
    int _size = data.length();
    for (int i = 0; i < _size; i++)
        EEPROM.write(add + i, data[i]);
    EEPROM.write(add + _size, '\0');
    EEPROM.commit();
}

String readString(int add)
{
    char data[100];
    int len = 0;
    unsigned char k = EEPROM.read(add);
    while (k != '\0' && len < 99)
    {
        k = EEPROM.read(add + len);
        data[len] = k;
        len++;
    }
    data[len] = '\0';
    return String(data);
}

void clearEEPROM()
{
    for (int i = 0; i < EEPROM_SIZE; i++)
        EEPROM.write(i, 0);
    EEPROM.commit();
}

// ==========================================
// SETUP WEB SERVER (CHIA 2 CHẾ ĐỘ)
// ==========================================

// Chế độ 1: Cấu hình Wifi (Khi chưa có mạng)
void setupConfigMode()
{
    isConfigMode = true;
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("HE THONG GIAM SAT"); // Tên Wifi phát ra

    // Hiển thị thông báo lên OLED (Chỉ hiện lúc cấu hình)
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("CHE DO CAU HINH");
    display.println("----------------");
    display.println("Wifi: HE THONG...");
    display.println("IP: 192.168.4.1");
    display.display();

    // Các API cho trang cấu hình
    server.on("/", []()
              { server.send(200, "text/html", wifi_config_html); });

    server.on("/scan", []()
              {
    int n = WiFi.scanNetworks();
    String json = "[";
    for (int i = 0; i < n; ++i) {
      if (i) json += ",";
      json += "\"" + WiFi.SSID(i) + "\"";
    }
    json += "]";
    server.send(200, "application/json", json); });

    server.on("/save", HTTP_POST, []()
              {
    String s = server.arg("ssid");
    String p = server.arg("pass");
    if (s.length() > 0) {
      clearEEPROM();
      writeString(SSID_ADDR, s);
      writeString(PASS_ADDR, p);
      server.send(200, "text/plain", "Da luu! Dang khoi dong lai...");
      delay(1000);
      ESP.restart();
    } else {
      server.send(400, "text/plain", "Loi: Thieu ten Wifi");
    } });

    server.on("/restart", []()
              {
    server.send(200, "text/plain", "Restarting...");
    delay(1000);
    ESP.restart(); });

    server.begin();
}

// Chế độ 2: Giám sát (Khi đã có mạng - Chạy Web cũ của bạn)
void setupNormalMode()
{
    isConfigMode = false;

    server.on("/", []()
              { server.send(200, "text/html", index_html); });

    server.on("/readings", []()
              {
    String json = "{\"temperature\":" + String(temp, 1) + ",\"humidity\":" + String(hum, 0) + "}";
    server.send(200, "application/json", json); });

    server.begin();
}

#endif
