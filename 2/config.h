/*
 * PROJECT: IoT MONITOR + WIFI MANAGER
 * GIỮ NGUYÊN GIAO DIỆN OLED CŨ + THÊM TÍNH NĂNG CẤU HÌNH WIFI
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==========================================
// 1. THƯ VIỆN & CẤU HÌNH
// ==========================================
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_AHTX0.h>
#include <EEPROM.h> // Thêm thư viện EEPROM để lưu Wifi
#include "time.h"

// --- NHÚNG FILE GIAO DIỆN ---
#include "index.h"       // Web Giám sát (Của bạn)
#include "wifi_config.h" // Web Cấu hình Wifi (Mới thêm)

// --- Cấu hình Chân ---
#define I2C_SDA 26
#define I2C_SCL 25
#define RESET_PIN 0  // Nút BOOT (Giữ 5s để Reset Wifi)
#define LED_PIN 22   // LED báo trạng thái
#define RELAY_PIN 27 // Chân điều khiển Relay

// --- Ngưỡng nhiệt độ bật Relay ---
#define TEMP_THRESHOLD 32

// --- EEPROM Config ---
#define EEPROM_SIZE 512
#define SSID_ADDR 0
#define PASS_ADDR 64

// --- Cấu hình Thời gian ---
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;
const int daylightOffset_sec = 0;
const long interval = 2000;

// ==========================================
// 2. KHỞI TẠO ĐỐI TƯỢNG
// ==========================================
WebServer server(80);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
Adafruit_AHTX0 aht;

// Biến toàn cục
float temp = 0.0;
float hum = 0.0;
unsigned long previousMillis = 0;
bool isConfigMode = false; // Cờ báo đang ở chế độ Cấu hình hay Giám sát

// Biến cho nút Reset
unsigned long pressTime = 0;
bool isPressing = false;

#endif
