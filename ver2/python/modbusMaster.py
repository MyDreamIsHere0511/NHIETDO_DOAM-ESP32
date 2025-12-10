# ==========================================
# MODBUS TCP CLIENT VỚI GIAO DIỆN ĐỒ HỌA
# Giống QModMaster - Có nút Connect, đọc liên tục
# ==========================================
# Cài đặt: pip install tkinter
# Chạy: python modbus_gui.py

import socket
import struct
import threading
import time
import tkinter as tk
from tkinter import ttk, messagebox

# ==========================================
# HÀM TÍNH CRC16 MODBUS
# ==========================================
def calculate_crc16(data):
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x0001:
                crc >>= 1
                crc ^= 0xA001
            else:
                crc >>= 1
    return crc

# ==========================================
# CLASS MODBUS CLIENT GUI
# ==========================================
class ModbusClientGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("🔌 Modbus TCP Client - Kiểm tra CRC")
        self.root.geometry("750x700")
        self.root.minsize(650, 550)  # Kích thước tối thiểu
        self.root.configure(bg="#1e1e1e")
        
        self.sock = None
        self.connected = False
        self.polling = False
        self.poll_thread = None
        self.auto_scroll = True  # Auto-scroll log mặc định BẬT
        self.display_format = "DEC"  # DEC, HEX, BIN
        self.registers_data = []  # Lưu dữ liệu registers
        
        self.create_widgets()
    
    def create_widgets(self):
        # ====== FRAME KẾT NỐI ======
        conn_frame = tk.LabelFrame(self.root, text="⚙️ Cấu hình kết nối", 
                                   bg="#2d2d2d", fg="white", font=("Arial", 10, "bold"))
        conn_frame.pack(fill="x", padx=10, pady=5)
        
        # IP
        tk.Label(conn_frame, text="IP ESP32:", bg="#2d2d2d", fg="white").grid(row=0, column=0, padx=5, pady=5)
        self.ip_entry = tk.Entry(conn_frame, width=20)
        self.ip_entry.insert(0, "192.168.30.235")
        self.ip_entry.grid(row=0, column=1, padx=5, pady=5)
        
        # Port
        tk.Label(conn_frame, text="Port:", bg="#2d2d2d", fg="white").grid(row=0, column=2, padx=5, pady=5)
        self.port_entry = tk.Entry(conn_frame, width=8)
        self.port_entry.insert(0, "502")
        self.port_entry.grid(row=0, column=3, padx=5, pady=5)
        
        # Nút Connect
        self.connect_btn = tk.Button(conn_frame, text="🔌 Connect", command=self.toggle_connection,
                                     bg="#4CAF50", fg="white", width=12)
        self.connect_btn.grid(row=0, column=4, padx=10, pady=5)
        
        # Trạng thái
        self.status_label = tk.Label(conn_frame, text="● Chưa kết nối", bg="#2d2d2d", fg="red", font=("Arial", 10))
        self.status_label.grid(row=0, column=5, padx=10, pady=5)
        
        # ====== FRAME CẤU HÌNH ĐỌC ======
        read_frame = tk.LabelFrame(self.root, text="📖 Cấu hình đọc", 
                                   bg="#2d2d2d", fg="white", font=("Arial", 10, "bold"))
        read_frame.pack(fill="x", padx=10, pady=5)
        
        # Slave ID
        tk.Label(read_frame, text="Slave ID:", bg="#2d2d2d", fg="white").grid(row=0, column=0, padx=5, pady=5)
        self.slave_entry = tk.Entry(read_frame, width=5)
        self.slave_entry.insert(0, "1")
        self.slave_entry.grid(row=0, column=1, padx=5, pady=5)
        
        # Start Address
        tk.Label(read_frame, text="Start Addr:", bg="#2d2d2d", fg="white").grid(row=0, column=2, padx=5, pady=5)
        self.addr_entry = tk.Entry(read_frame, width=5)
        self.addr_entry.insert(0, "0")
        self.addr_entry.grid(row=0, column=3, padx=5, pady=5)
        
        # Quantity
        tk.Label(read_frame, text="Quantity:", bg="#2d2d2d", fg="white").grid(row=0, column=4, padx=5, pady=5)
        self.qty_entry = tk.Entry(read_frame, width=5)
        self.qty_entry.insert(0, "4")
        self.qty_entry.grid(row=0, column=5, padx=5, pady=5)
        
        # Scan Rate
        tk.Label(read_frame, text="Scan (ms):", bg="#2d2d2d", fg="white").grid(row=0, column=6, padx=5, pady=5)
        self.scan_entry = tk.Entry(read_frame, width=6)
        self.scan_entry.insert(0, "1000")
        self.scan_entry.grid(row=0, column=7, padx=5, pady=5)
        
        # Nút Read Once
        self.read_btn = tk.Button(read_frame, text="📖 Read Once", command=self.read_once,
                                  bg="#2196F3", fg="white", width=10)
        self.read_btn.grid(row=1, column=0, columnspan=2, padx=5, pady=5)
        
        # Nút Poll
        self.poll_btn = tk.Button(read_frame, text="▶ Start Poll", command=self.toggle_poll,
                                  bg="#FF9800", fg="white", width=10)
        self.poll_btn.grid(row=1, column=2, columnspan=2, padx=5, pady=5)
        
        # ====== FRAME GHI REGISTER ======
        write_frame = tk.LabelFrame(self.root, text="✏️ Ghi Register (FC 0x06)", 
                                    bg="#2d2d2d", fg="white", font=("Arial", 10, "bold"))
        write_frame.pack(fill="x", padx=10, pady=5)
        
        # Địa chỉ Register
        tk.Label(write_frame, text="Địa chỉ:", bg="#2d2d2d", fg="white").grid(row=0, column=0, padx=5, pady=5)
        self.write_addr_entry = tk.Entry(write_frame, width=5)
        self.write_addr_entry.insert(0, "3")
        self.write_addr_entry.grid(row=0, column=1, padx=5, pady=5)
        
        # Giá trị
        tk.Label(write_frame, text="Giá trị:", bg="#2d2d2d", fg="white").grid(row=0, column=2, padx=5, pady=5)
        self.write_value_entry = tk.Entry(write_frame, width=8)
        self.write_value_entry.insert(0, "1")
        self.write_value_entry.grid(row=0, column=3, padx=5, pady=5)
        
        # Nút Ghi
        self.write_btn = tk.Button(write_frame, text="📝 Ghi", command=self.write_single,
                                   bg="#9C27B0", fg="white", width=8, font=("Arial", 9, "bold"))
        self.write_btn.grid(row=0, column=4, padx=10, pady=5)
        
        # Trạng thái ghi
        self.write_status_label = tk.Label(write_frame, text="---", bg="#2d2d2d", fg="cyan", font=("Arial", 10))
        self.write_status_label.grid(row=0, column=5, padx=10, pady=5)
        
        # ====== PANEDWINDOW CHỨA DATA VÀ LOG (RESIZE ĐƯỢC) ======
        paned = tk.PanedWindow(self.root, orient=tk.VERTICAL, bg="#1e1e1e", 
                               sashwidth=8, sashrelief=tk.RAISED, sashpad=2)
        paned.pack(fill="both", expand=True, padx=10, pady=5)
        
        # ====== FRAME DỮ LIỆU ======
        data_frame = tk.LabelFrame(paned, text="📊 Dữ liệu Registers", 
                                   bg="#2d2d2d", fg="white", font=("Arial", 10, "bold"))
        paned.add(data_frame, minsize=120, height=180)  # Chiều cao tối thiểu 120px
        
        # Header với nút chọn định dạng
        data_header = tk.Frame(data_frame, bg="#2d2d2d")
        data_header.pack(fill="x", padx=5, pady=2)
        
        tk.Label(data_header, text="Định dạng hiển thị:", bg="#2d2d2d", fg="white").pack(side="left", padx=5)
        
        self.format_var = tk.StringVar(value="DEC")
        format_options = [("DEC", "DEC"), ("HEX", "HEX"), ("BIN", "BIN")]
        for text, value in format_options:
            rb = tk.Radiobutton(data_header, text=text, variable=self.format_var, value=value,
                               bg="#2d2d2d", fg="cyan", selectcolor="#404040",
                               activebackground="#2d2d2d", activeforeground="white",
                               command=self.update_display_format, font=("Arial", 9, "bold"))
            rb.pack(side="left", padx=5)
        
        # Bảng dữ liệu với scrollbar
        columns = ("Địa chỉ", "Giá trị", "Ý nghĩa")
        
        # Style cho Treeview - chữ to hơn
        style = ttk.Style()
        style.configure("Custom.Treeview", font=("Arial", 12), rowheight=30)
        style.configure("Custom.Treeview.Heading", font=("Arial", 11, "bold"))
        
        # Frame chứa tree và scrollbar
        tree_container = tk.Frame(data_frame, bg="#2d2d2d")
        tree_container.pack(fill="both", expand=True, padx=5, pady=5)
        
        # Scrollbar dọc
        tree_scrollbar = tk.Scrollbar(tree_container, orient="vertical")
        tree_scrollbar.pack(side="right", fill="y")
        
        self.tree = ttk.Treeview(tree_container, columns=columns, show="headings", height=5,
                                  yscrollcommand=tree_scrollbar.set, style="Custom.Treeview")
        tree_scrollbar.config(command=self.tree.yview)
        
        # Cột có thể resize - kéo được
        self.tree.heading("Địa chỉ", text="Địa chỉ")
        self.tree.column("Địa chỉ", width=80, minwidth=50, anchor="center", stretch=True)
        
        self.tree.heading("Giá trị", text="Giá trị")
        self.tree.column("Giá trị", width=150, minwidth=80, anchor="center", stretch=True)
        
        self.tree.heading("Ý nghĩa", text="Ý nghĩa")
        self.tree.column("Ý nghĩa", width=200, minwidth=100, anchor="w", stretch=True)
        
        self.tree.pack(fill="both", expand=True, side="left")
        
        # ====== FRAME CRC ======
        crc_frame = tk.LabelFrame(paned, text="🔐 Kiểm tra CRC16", 
                                  bg="#2d2d2d", fg="white", font=("Arial", 10, "bold"))
        paned.add(crc_frame, minsize=40, height=50)
        
        self.crc_recv_label = tk.Label(crc_frame, text="CRC Nhận: ---", bg="#2d2d2d", fg="cyan", font=("Consolas", 11))
        self.crc_recv_label.grid(row=0, column=0, padx=20, pady=5)
        
        self.crc_calc_label = tk.Label(crc_frame, text="CRC Tính: ---", bg="#2d2d2d", fg="cyan", font=("Consolas", 11))
        self.crc_calc_label.grid(row=0, column=1, padx=20, pady=5)
        
        self.crc_status_label = tk.Label(crc_frame, text="Trạng thái: ---", bg="#2d2d2d", fg="white", font=("Arial", 11, "bold"))
        self.crc_status_label.grid(row=0, column=2, padx=20, pady=5)
        
        # ====== FRAME LOG ======
        log_frame = tk.LabelFrame(paned, text="📜 Log", 
                                  bg="#2d2d2d", fg="white", font=("Arial", 10, "bold"))
        paned.add(log_frame, minsize=100, height=150)  # Chiều cao tối thiểu 100px
        
        # Frame header với nút auto-scroll
        log_header = tk.Frame(log_frame, bg="#2d2d2d")
        log_header.pack(fill="x", padx=5, pady=2)
        
        # Nút toggle auto-scroll (mũi tên xuống)
        self.auto_scroll_btn = tk.Button(log_header, text="⬇ Auto", command=self.toggle_auto_scroll,
                                         bg="#4CAF50", fg="white", width=8, font=("Arial", 8, "bold"))
        self.auto_scroll_btn.pack(side="left", padx=5)
        
        # Nút xóa log
        self.clear_log_btn = tk.Button(log_header, text="🗑 Xóa", command=self.clear_log,
                                       bg="#607D8B", fg="white", width=6, font=("Arial", 8))
        self.clear_log_btn.pack(side="left", padx=5)
        
        # Frame chứa log và scrollbar
        log_container = tk.Frame(log_frame, bg="#1a1a1a")
        log_container.pack(fill="both", expand=True, padx=5, pady=5)
        
        # Scrollbar dọc
        log_scrollbar = tk.Scrollbar(log_container, orient="vertical")
        log_scrollbar.pack(side="right", fill="y")
        
        self.log_text = tk.Text(log_container, height=4, bg="#1a1a1a", fg="#00ff00", 
                                font=("Consolas", 9), yscrollcommand=log_scrollbar.set)
        log_scrollbar.config(command=self.log_text.yview)
        self.log_text.pack(fill="both", expand=True, side="left")
    
    def log(self, message):
        self.log_text.insert(tk.END, f"{time.strftime('%H:%M:%S')} - {message}\n")
        if self.auto_scroll:
            self.log_text.see(tk.END)
    
    def toggle_auto_scroll(self):
        """Bật/tắt auto-scroll log"""
        self.auto_scroll = not self.auto_scroll
        if self.auto_scroll:
            self.auto_scroll_btn.config(text="⬇ Auto", bg="#4CAF50")
            self.log_text.see(tk.END)
        else:
            self.auto_scroll_btn.config(text="⏸ Dừng", bg="#607D8B")
    
    def clear_log(self):
        """Xóa toàn bộ log"""
        self.log_text.delete(1.0, tk.END)
    
    def toggle_connection(self):
        if not self.connected:
            self.connect()
        else:
            self.disconnect()
    
    def connect(self):
        try:
            ip = self.ip_entry.get()
            port = int(self.port_entry.get())
            
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(3)
            self.sock.connect((ip, port))
            
            self.connected = True
            self.connect_btn.config(text="🔌 Disconnect", bg="#f44336")
            self.status_label.config(text="● Đã kết nối", fg="#4CAF50")
            self.log(f"✅ Kết nối thành công tới {ip}:{port}")
            
        except Exception as e:
            messagebox.showerror("Lỗi kết nối", str(e))
            self.log(f"❌ Lỗi: {e}")
    
    def disconnect(self):
        self.polling = False
        if self.sock:
            self.sock.close()
        self.connected = False
        self.connect_btn.config(text="🔌 Connect", bg="#4CAF50")
        self.status_label.config(text="● Chưa kết nối", fg="red")
        self.poll_btn.config(text="▶ Start Poll", bg="#FF9800")
        self.log("🔌 Đã ngắt kết nối")
    
    def read_once(self):
        if not self.connected:
            messagebox.showwarning("Cảnh báo", "Chưa kết nối!")
            return
        self.read_registers()
    
    def write_single(self):
        """Ghi giá trị vào register được chọn"""
        if not self.connected:
            messagebox.showwarning("Cảnh báo", "Chưa kết nối!")
            return
        try:
            addr = int(self.write_addr_entry.get())
            value = int(self.write_value_entry.get())
            self.write_register(addr, value)
            self.write_status_label.config(text=f"✅ Reg[{addr}]={value}", fg="#4CAF50")
        except ValueError:
            messagebox.showerror("Lỗi", "Địa chỉ và giá trị phải là số!")
    
    def format_value(self, value):
        """Format giá trị theo định dạng được chọn"""
        fmt = self.format_var.get()
        if fmt == "DEC":
            return str(value)
        elif fmt == "HEX":
            return f"0x{value:04X}"
        elif fmt == "BIN":
            return f"{value:016b}"
        return str(value)
    
    def update_display_format(self):
        """Cập nhật bảng khi thay đổi định dạng"""
        if not self.registers_data:
            return
        
        self.tree.delete(*self.tree.get_children())
        labels = ["Nhiệt độ", "Độ ẩm", "Relay Status", "Relay Control"]
        
        for i, value in enumerate(self.registers_data):
            formatted_val = self.format_value(value)
            
            if i == 0:
                meaning = f"Nhiệt độ: {value/10:.1f}°C"
            elif i == 1:
                meaning = f"Độ ẩm: {value/10:.1f}%"
            elif i == 2:
                meaning = f"Relay Status: {'ON' if value else 'OFF'}"
            elif i == 3:
                meaning = f"Relay Control: {'ON' if value else 'OFF'}"
            else:
                meaning = f"Reg {i}: {value}"
            
            self.tree.insert("", "end", values=(i, formatted_val, meaning))
    
    def write_register(self, address, value):
        """Ghi giá trị vào 1 register - Function Code 0x06"""
        try:
            slave_id = int(self.slave_entry.get())
            
            # Tạo request Write Single Register (Function Code 0x06)
            # Format: Transaction ID (2) + Protocol ID (2) + Length (2) + Unit ID (1) + 
            #         Function Code (1) + Address (2) + Value (2)
            request = struct.pack('>HHHBBHH', 
                                  0x0001,    # Transaction ID
                                  0x0000,    # Protocol ID
                                  0x0006,    # Length (6 bytes sau)
                                  slave_id,  # Unit ID
                                  0x06,      # Function Code: Write Single Register
                                  address,   # Register Address
                                  value)     # Value to write
            
            self.log(f"📤 Write Reg[{address}] = {value}")
            self.log(f"   Request: {request.hex().upper()}")
            
            self.sock.send(request)
            response = self.sock.recv(256)
            
            self.log(f"📥 Response: {response.hex().upper()}")
            
            # Kiểm tra response (echo lại request = thành công)
            if len(response) >= 12:
                resp_func = response[7]
                if resp_func == 0x06:
                    self.log("✅ Ghi thành công!")
                elif resp_func == 0x86:  # Error
                    self.log("❌ Lỗi ghi register!")
            
        except Exception as e:
            self.log(f"❌ Lỗi write: {e}")
    
    def toggle_poll(self):
        if not self.connected:
            messagebox.showwarning("Cảnh báo", "Chưa kết nối!")
            return
        
        if not self.polling:
            self.polling = True
            self.poll_btn.config(text="⏹ Stop Poll", bg="#f44336")
            self.poll_thread = threading.Thread(target=self.poll_loop, daemon=True)
            self.poll_thread.start()
            self.log("▶ Bắt đầu polling...")
        else:
            self.polling = False
            self.poll_btn.config(text="▶ Start Poll", bg="#FF9800")
            self.log("⏹ Dừng polling")
    
    def poll_loop(self):
        while self.polling and self.connected:
            try:
                self.read_registers()
                scan_rate = int(self.scan_entry.get()) / 1000
                time.sleep(scan_rate)
            except:
                break
    
    def read_registers(self):
        try:
            slave_id = int(self.slave_entry.get())
            start_addr = int(self.addr_entry.get())
            quantity = int(self.qty_entry.get())
            
            # Tạo request
            request = struct.pack('>HHHBBHH', 0x0001, 0x0000, 0x0006, 
                                  slave_id, 0x03, start_addr, quantity)
            
            self.sock.send(request)
            response = self.sock.recv(256)
            
            self.parse_response(response, quantity)
            
        except Exception as e:
            self.log(f"❌ Lỗi đọc: {e}")
            self.disconnect()
    
    def parse_response(self, response, quantity):
        if len(response) < 9:
            return
        
        # Log raw response để debug
        self.log(f"📦 Raw: {response.hex().upper()}")
        
        byte_count = response[8]
        data_start = 9
        data_end = 9 + byte_count
        
        # Lấy registers - ĐỌC ĐÚNG THỨ TỰ BIG ENDIAN (High byte trước, Low byte sau)
        registers = []
        for i in range(0, byte_count, 2):
            high_byte = response[data_start + i]
            low_byte = response[data_start + i + 1]
            value = (high_byte << 8) | low_byte
            # Log từng giá trị để debug
            self.log(f"   Reg[{i//2}]: High=0x{high_byte:02X}, Low=0x{low_byte:02X} → {value}")
            registers.append(value)
        
        # Lưu dữ liệu để cập nhật khi đổi format
        self.registers_data = registers
        
        # Cập nhật bảng với format hiện tại
        self.update_display_format()
        
        # Kiểm tra CRC
        if len(response) >= data_end + 2:
            crc_low = response[data_end]
            crc_high = response[data_end + 1]
            crc_received = (crc_high << 8) | crc_low
            
            pdu_data = response[6:data_end]
            crc_calculated = calculate_crc16(pdu_data)
            
            self.crc_recv_label.config(text=f"CRC Nhận: 0x{crc_received:04X}")
            self.crc_calc_label.config(text=f"CRC Tính: 0x{crc_calculated:04X}")
            
            if crc_received == crc_calculated:
                self.crc_status_label.config(text="✅ CRC ĐÚNG!", fg="#4CAF50")
            else:
                self.crc_status_label.config(text="❌ CRC SAI!", fg="#f44336")
        else:
            self.crc_recv_label.config(text="CRC Nhận: N/A")
            self.crc_calc_label.config(text="CRC Tính: N/A")
            self.crc_status_label.config(text="⚠️ Không có CRC", fg="#FF9800")

# ==========================================
# MAIN
# ==========================================
if __name__ == "__main__":
    root = tk.Tk()
    app = ModbusClientGUI(root)
    root.mainloop()
