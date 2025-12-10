#ifndef MODBUS_TCP_H
#define MODBUS_TCP_H

#include <WiFi.h>
#include "modbus_crc.h" // Thêm thư viện tính CRC16

// ==========================================
// CẤU HÌNH MODBUS TCP SERVER
// ==========================================
#define MODBUS_PORT 502   // Port chuẩn Modbus TCP
#define MODBUS_SLAVE_ID 1 // Địa chỉ Slave

// Định nghĩa địa chỉ Holding Registers
// PLC/HMI sẽ đọc các địa chỉ này (40001, 40002, 40003...)
#define REG_TEMPERATURE 0   // Register 0 = Nhiệt độ x10 (VD: 285 = 28.5°C)
#define REG_HUMIDITY 1      // Register 1 = Độ ẩm x10 (VD: 650 = 65.0%)
#define REG_RELAY_STATUS 2  // Register 2 = Trạng thái Relay (0=OFF, 1=ON)
#define REG_RELAY_CONTROL 3 // Register 3 = Điều khiển Relay từ xa (0=OFF, 1=ON)
#define NUM_HOLDING_REGS 10 // Tổng số registers

// Mảng lưu giá trị Holding Registers
uint16_t holdingRegisters[NUM_HOLDING_REGS] = {0};

// Biến điều khiển Relay từ Modbus
bool modbusRelayControl = false;

// Server Modbus TCP
WiFiServer modbusServer(MODBUS_PORT);
WiFiClient modbusClient;

// ==========================================
// CẬP NHẬT GIÁ TRỊ REGISTERS TỪ CẢM BIẾN
// ==========================================
void updateModbusRegisters()
{
    // Nhân 10 để giữ 1 chữ số thập phân (vì Modbus chỉ truyền số nguyên)
    holdingRegisters[REG_TEMPERATURE] = (uint16_t)(temp * 10);   // 28.5°C -> 285
    holdingRegisters[REG_HUMIDITY] = (uint16_t)(hum * 10);       // 65.0% -> 650
    holdingRegisters[REG_RELAY_STATUS] = digitalRead(RELAY_PIN); // 0 hoặc 1
}

// ==========================================
// XỬ LÝ YÊU CẦU MODBUS
// ==========================================
void processModbusRequest(uint8_t *request, int len)
{
    if (len < 12)
        return; // Gói tin không hợp lệ

    // Phân tích Modbus TCP Header
    uint16_t transactionId = (request[0] << 8) | request[1];
    uint16_t protocolId = (request[2] << 8) | request[3];
    uint16_t length = (request[4] << 8) | request[5];
    uint8_t unitId = request[6];
    uint8_t functionCode = request[7];
    uint16_t startAddress = (request[8] << 8) | request[9];
    uint16_t quantity = (request[10] << 8) | request[11];

    // Buffer cho response
    uint8_t response[256];
    int responseLen = 0;

    // Function Code 03: Read Holding Registers
    if (functionCode == 0x03)
    {
        // Kiểm tra địa chỉ hợp lệ
        if (startAddress + quantity > NUM_HOLDING_REGS)
        {
            // Lỗi: Địa chỉ không hợp lệ
            response[0] = request[0];
            response[1] = request[1]; // Transaction ID
            response[2] = 0;
            response[3] = 0; // Protocol ID
            response[4] = 0;
            response[5] = 3; // Length
            response[6] = unitId;
            response[7] = functionCode | 0x80; // Error code
            response[8] = 0x02;                // Illegal address
            responseLen = 9;
        }
        else
        {
            // Cập nhật giá trị mới nhất
            updateModbusRegisters();

            // Tạo response
            int dataLen = quantity * 2;
            response[0] = request[0];
            response[1] = request[1]; // Transaction ID
            response[2] = 0;
            response[3] = 0; // Protocol ID
            response[4] = 0;
            response[5] = 3 + dataLen; // Length
            response[6] = unitId;
            response[7] = functionCode;
            response[8] = dataLen; // Byte count

            // Đưa dữ liệu registers vào response
            for (int i = 0; i < quantity; i++)
            {
                response[9 + i * 2] = holdingRegisters[startAddress + i] >> 8;    // High byte
                response[10 + i * 2] = holdingRegisters[startAddress + i] & 0xFF; // Low byte
            }
            responseLen = 9 + dataLen;

            // ====== THÊM CRC16 VÀO CUỐI RESPONSE ======
            // Tính CRC từ phần PDU (từ byte 6 trở đi: UnitID + FunctionCode + Data)
            uint16_t crc = calculateCRC16(&response[6], responseLen - 6);
            response[responseLen] = crc & 0xFF;     // CRC Low byte
            response[responseLen + 1] = (crc >> 8); // CRC High byte
            responseLen += 2;                       // Tăng độ dài thêm 2 byte CRC

            // In ra Serial để debug
            Serial.print("[MODBUS] CRC16: 0x");
            Serial.println(crc, HEX);
        }

        // Gửi response
        modbusClient.write(response, responseLen);
    }
    // Function Code 06: Write Single Register
    else if (functionCode == 0x06)
    {
        uint16_t value = (request[10] << 8) | request[11];

        // Chỉ cho phép ghi vào REG_RELAY_CONTROL (Register 3)
        if (startAddress == REG_RELAY_CONTROL)
        {
            holdingRegisters[REG_RELAY_CONTROL] = value;
            modbusRelayControl = (value == 1);

            // Echo lại request (chuẩn Modbus)
            modbusClient.write(request, len);

            Serial.print("[MODBUS] Relay Control: ");
            Serial.println(modbusRelayControl ? "ON" : "OFF");
        }
        else
        {
            // Lỗi: Không cho phép ghi
            response[0] = request[0];
            response[1] = request[1];
            response[2] = 0;
            response[3] = 0;
            response[4] = 0;
            response[5] = 3;
            response[6] = unitId;
            response[7] = functionCode | 0x80;
            response[8] = 0x02; // Illegal address
            modbusClient.write(response, 9);
        }
    }
}

// ==========================================
// KHỞI TẠO MODBUS SERVER
// ==========================================
void setupModbusTCP()
{
    modbusServer.begin();
    Serial.println("[MODBUS] port: 502");
    Serial.print("[MODBUS] IP: ");
    Serial.println(WiFi.localIP());
}

// ==========================================
// XỬ LÝ CLIENT MODBUS (GỌI TRONG LOOP)
// ==========================================
void handleModbusTCP()
{
    // Kiểm tra client mới
    if (modbusServer.hasClient())
    {
        if (!modbusClient || !modbusClient.connected())
        {
            modbusClient = modbusServer.available();
            Serial.println("[MODBUS] Client connected");
        }
    }

    // Xử lý dữ liệu từ client
    if (modbusClient && modbusClient.connected())
    {
        if (modbusClient.available())
        {
            uint8_t buffer[256];
            int len = modbusClient.read(buffer, sizeof(buffer));
            if (len > 0)
            {
                processModbusRequest(buffer, len);
            }
        }
    }
}

#endif
