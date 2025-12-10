#ifndef MODBUS_CRC_H
#define MODBUS_CRC_H

// ==========================================
// HÀM TÍNH CRC16 MODBUS
// Thuật toán: CRC-16/MODBUS (Polynomial 0x8005)
// ==========================================

uint16_t calculateCRC16(uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFF; // Giá trị khởi tạo

    for (uint16_t i = 0; i < length; i++)
    {
        crc ^= (uint16_t)data[i]; // XOR byte vào CRC

        for (uint8_t j = 0; j < 8; j++)
        { // Xử lý 8 bit
            if (crc & 0x0001)
            {                  // Nếu bit thấp nhất = 1
                crc >>= 1;     // Dịch phải 1 bit
                crc ^= 0xA001; // XOR với polynomial đảo
            }
            else
            {
                crc >>= 1; // Chỉ dịch phải
            }
        }
    }

    return crc; // Trả về CRC 16-bit
}

// ==========================================
// GIẢI THÍCH THUẬT TOÁN CRC16:
// ==========================================
//
// Bước 1: Khởi tạo CRC = 0xFFFF (16 bit toàn 1)
//
// Bước 2: Với mỗi byte dữ liệu:
//    - XOR byte đó vào CRC
//    - Lặp 8 lần (cho 8 bit):
//      + Nếu bit cuối = 1: dịch phải, XOR với 0xA001
//      + Nếu bit cuối = 0: chỉ dịch phải
//
// Bước 3: Trả về giá trị CRC 16-bit
//
// VÍ DỤ:
// Data = [0x01, 0x03, 0x00, 0x00, 0x00, 0x02]
// CRC  = 0xC40B
// Gửi đi: [0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0x0B, 0xC4]
//         (CRC gửi theo thứ tự Low byte trước, High byte sau)
//
// ==========================================

// Kiểm tra CRC có đúng không
bool verifyCRC16(uint8_t *data, uint16_t length, uint16_t receivedCRC)
{
    uint16_t calculatedCRC = calculateCRC16(data, length);
    return (calculatedCRC == receivedCRC);
}

#endif
