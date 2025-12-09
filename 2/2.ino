// ==========================================
// NHÚNG CÁC FILE MODULE
// ==========================================
#include "config.h"         // Thư viện, cấu hình chân, biến toàn cục
#include "sensor_display.h" // Cảm biến & Màn hình OLED
#include "wifi_mode.h"      // EEPROM & Chế độ WiFi

// ==========================================
// SETUP CHÍNH
// ==========================================
void setup()
{
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  // Khởi động phần cứng
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("Lỗi OLED"));
  }
  else
  {
    display.setTextColor(WHITE);
  }
  if (!aht.begin())
  {
    Serial.println("Lỗi AHT30");
  }

  // Đọc Wifi đã lưu
  String saved_ssid = readString(SSID_ADDR);
  String saved_pass = readString(PASS_ADDR);

  if (saved_ssid.length() > 1)
  {
    // TRƯỜNG HỢP A: ĐÃ CÓ WIFI -> KẾT NỐI
    WiFi.mode(WIFI_STA);
    WiFi.begin(saved_ssid.c_str(), saved_pass.c_str());

    // Hiển thị trạng thái kết nối (Tạm thời)
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("Dang ket noi Wifi...");
    display.println(saved_ssid);
    display.display();

    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20)
    {
      delay(500);
      Serial.print(".");
      retries++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      // Kết nối thành công -> Vào chế độ Giám sát
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

      // Hiển thị IP 3 giây rồi vào màn hình chính
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Wifi OK!");
      display.println("IP:");
      display.println(WiFi.localIP());
      display.display();
      delay(3000);

      setupNormalMode();
    }
    else
    {
      // Kết nối thất bại -> Quay về chế độ Cấu hình
      setupConfigMode();
    }
  }
  else
  {
    // TRƯỜNG HỢP B: CHƯA CÓ WIFI -> CHẾ ĐỘ CẤU HÌNH
    setupConfigMode();
  }
}

// ==========================================
// 9. LOOP CHÍNH
// ==========================================
void loop()
{
  server.handleClient();

  // 1. Logic nút Reset (Giữ 5 giây để xóa Wifi)
  int btnState = digitalRead(RESET_PIN);
  if (btnState == LOW)
  {
    if (!isPressing)
    {
      isPressing = true;
      pressTime = millis();
    }
    if (millis() - pressTime > 5000)
    {
      display.clearDisplay();
      display.setCursor(0, 20);
      display.setTextSize(2);
      display.print("RESETING...");
      display.display();
      digitalWrite(LED_PIN, HIGH);

      clearEEPROM(); // Xóa sạch Wifi
      delay(1000);
      ESP.restart();
    }
  }
  else
  {
    isPressing = false;
    digitalWrite(LED_PIN, LOW);
  }

  // 2. Logic màn hình & Cảm biến (Chỉ chạy khi ở Chế độ Giám sát)
  if (!isConfigMode)
  {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
      previousMillis = currentMillis;
      readSensor();
      updateOLED(); // Gọi hàm OLED cũ của bạn
    }
  }
}