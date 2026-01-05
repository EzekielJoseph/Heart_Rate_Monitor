# Heart_Rate_Monitor
Heart rate monitor for VR
# ESP32 Heart Rate Monitor Web Server (MAX30102)

## 📌 Deskripsi Proyek
Proyek ini merupakan sistem **monitoring detak jantung (Heart Rate / BPM)** berbasis **ESP32** dan sensor **MAX30102**, yang menampilkan data secara **real-time di web browser tanpa perlu refresh halaman**.

ESP32 berperan sebagai:
- Access Point (Wi-Fi)
- Web Server
- Pengolah data sensor MAX30102

Komunikasi data dari ESP32 ke browser menggunakan **Server-Sent Events (SSE)** sehingga nilai BPM, IR sensor, dan counter dapat diperbarui otomatis setiap 1 detik.

---

## 🧩 Fitur Utama
- 📡 ESP32 sebagai Wi-Fi Access Point
- ❤️ Pembacaan detak jantung (BPM)
- 📊 Perhitungan rata-rata BPM
- 🌐 Tampilan web real-time tanpa reload
- 🔄 Update data otomatis tiap 1 detik
- 📦 Data dikirim dalam format JSON

---

## 🛠️ Hardware yang Digunakan
- ESP32 (DOIT ESP32 Devkit V1)
- Sensor Heart Rate **MAX30102 / MAX30105**
- Kabel jumper
- Sumber daya USB

---

## 📚 Library yang Digunakan
Pastikan library berikut sudah terinstall di Arduino IDE:

- `WiFi.h`
- `ESPAsyncWebServer`
- `ArduinoJson`
- `MAX30105`
- `heartRate`

> ⚠️ **Catatan:**  
> Library `ESPAsyncWebServer` membutuhkan:
> - `AsyncTCP`

---

## 🔌 Konfigurasi Pin (I2C)
| MAX30102 | ESP32 |
|--------|-------|
| VIN    | 3.3V  |
| GND    | GND   |
| SDA    | GPIO 21 |
| SCL    | GPIO 22 |

---

## 📡 Konfigurasi Wi-Fi
ESP32 berjalan sebagai **Access Point** dengan konfigurasi berikut:

```cpp
SSID     : ESP32-TEST
Password : 12345678
IP       : 192.168.4.1
