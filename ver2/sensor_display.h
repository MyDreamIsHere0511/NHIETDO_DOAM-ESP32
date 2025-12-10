#ifndef SENSOR_DISPLAY_H
#define SENSOR_DISPLAY_H

// ==========================================
// HÀM XỬ LÝ CẢM BIẾN
// ==========================================
void readSensor()
{
    sensors_event_t humidity, temp_event;
    aht.getEvent(&humidity, &temp_event);
    if (isnan(temp_event.temperature) || isnan(humidity.relative_humidity))
    {
        // Lỗi thì bỏ qua
    }
    else
    {
        temp = temp_event.temperature;
        hum = humidity.relative_humidity;
    }
}

// ==========================================
// HÀM HIỂN THỊ OLED
// ==========================================
void updateOLED()
{
    struct tm timeinfo;
    bool timeSynced = getLocalTime(&timeinfo);

    display.clearDisplay();

    // --- PHẦN 1: THANH TRẠNG THÁI (TOP BAR - VÙNG VÀNG) ---
    if (timeSynced)
    {
        display.setTextSize(2);
        // Căn giữa giờ (Màn hình rộng 128, chữ rộng khoảng 60 -> X ~ 34)
        display.setCursor(34, 1);
        if (timeinfo.tm_hour < 10)
            display.print("0");
        display.print(timeinfo.tm_hour);
        display.print(":");
        if (timeinfo.tm_min < 10)
            display.print("0");
        display.print(timeinfo.tm_min);
    }
    else
    {
        display.setTextSize(1);
        display.setCursor(25, 4);
        display.print("Dang cap nhat...");
    }

    // Kẻ đường phân cách
    display.drawLine(0, 17, 128, 17, SSD1306_WHITE); // Kẻ ngang (Dịch xuống 1px cho thoáng)
    display.drawLine(64, 17, 64, 64, SSD1306_WHITE); // Kẻ dọc chia đôi

    // --- PHẦN 2: NỘI DUNG CHÍNH (VÙNG XANH) ---

    // === Ô BÊN TRÁI: NHIỆT ĐỘ ===
    // 1. Hiển thị số (Size 2 - To)
    display.setTextSize(2);
    display.setCursor(10, 25); // Dịch X vào 10, Y xuống 25 cho cân
    display.print((int)temp);
    display.setTextSize(1);
    display.print("o");
    display.setTextSize(2);
    display.print("C");

    // 2. Hiển thị chữ (Size 1 - In hoa)
    display.setTextSize(1);
    display.setCursor(8, 48); // Căn giữa ô trái (Ô rộng 64px, chữ rộng ~48px)
    display.print("NHIET DO");

    // === Ô BÊN PHẢI: ĐỘ ẨM ===
    // 1. Hiển thị số (Size 2 - To)
    display.setTextSize(2);
    display.setCursor(74, 25); // 64 (giữa) + 10 (lề) = 74. Y=25 ngang hàng nhiệt độ
    display.print((int)hum);
    display.print("%");

    // 2. Hiển thị chữ (Size 1 - In hoa)
    display.setTextSize(1);
    display.setCursor(80, 48); // Căn giữa ô phải
    display.print("DO AM");

    display.display();
}

#endif
