# 📹 ESP32-CAM Canlı Görüntü Akışı

ESP32-CAM kartı kullanarak WiFi üzerinden canlı kamera görüntüsü yayını yapan Arduino projesi.

## 🚀 Kurulum

### 1. Repository'yi Klonlayın

```bash
git clone https://github.com/mehmetdogandev/espwebcam.git
cd espwebcam
```

### 2. Ortam Değişkenlerini Yapılandırın

Projeyi kullanmadan önce WiFi bilgilerinizi yapılandırmanız gerekmektedir:

1. `env_example.h` dosyasını `env.h` olarak kopyalayın:
   ```bash
   cp env_example.h env.h
   ```

2. `env.h` dosyasını açın ve aşağıdaki alanları kendi bilgilerinizle doldurun:
   ```cpp
   const char* ssid = "WIFI_ADI";        // Kendi WiFi ağ adınızı girin
   const char* password = "WIFI_SIFRESI"; // Kendi WiFi şifrenizi girin
   ```

### 3. Arduino IDE'de Yükleyin

1. Arduino IDE'yi açın
2. Kart olarak **"AI-Thinker ESP32"** seçin
3. Gerekli kütüphanelerin yüklü olduğundan emin olun:
   - `esp_camera.h` (ESP32 kütüphanesi ile birlikte gelir)
   - `WiFi.h` (ESP32 kütüphanesi ile birlikte gelir)
4. `espwebcam.ino` dosyasını açın
5. Kodu ESP32-CAM kartınıza yükleyin

### 4. Seri Monitörü Kontrol Edin

1. Seri monitörü açın (115200 baud hızı)
2. WiFi'ye bağlandıktan sonra IP adresini not edin
3. Tarayıcınızda bu IP adresini açarak canlı görüntüyü görebilirsiniz

## ✨ Özellikler

- ✅ WiFi üzerinden canlı görüntü akışı
- ✅ Modern ve responsive web arayüzü
- ✅ Gerçek zamanlı kamera görüntüsü
- ✅ Yenileme ve duraklat/devam kontrolleri
- ✅ Otomatik bağlantı durumu takibi
- ✅ Mobil cihazlardan erişim desteği

## 📋 Gereksinimler

- ESP32-CAM modülü (AI-Thinker)
- Arduino IDE
- ESP32 Board Support Package
- WiFi ağına erişim

## 🔧 Yapılandırma

Kamera kalitesi ve performans ayarları `espwebcam.ino` dosyasında yapılabilir:

- **Frame Size**: `FRAMESIZE_VGA` (640x480) - Varsayılan
- **JPEG Quality**: `12` (0-63 arası, düşük sayı = yüksek kalite)

Daha yüksek kalite için `FRAMESIZE_SVGA` veya `FRAMESIZE_XGA` kullanabilirsiniz (daha fazla RAM gerektirir).

## 📝 Notlar

- `env.h` dosyası hassas bilgiler içerdiği için `.gitignore` dosyasına eklenmiştir
- GitHub'a yüklerken sadece `env_example.h` dosyası yüklenir
- İlk bağlantı 30 saniyeye kadar sürebilir

## 🐛 Sorun Giderme

- **WiFi'ye bağlanamıyor**: `env.cpp` dosyasındaki SSID ve şifre bilgilerini kontrol edin
- **Kamera görüntüsü gelmiyor**: Seri monitörde hata mesajlarını kontrol edin
- **Yavaş görüntü akışı**: Frame size'ı düşürün veya JPEG quality'yi artırın

## 📄 Lisans

Bu proje açık kaynak kodludur.

---

## 👨‍💻 Geliştirici

<a href="https://github.com/mehmetdogandev">
  <img src="https://avatars.githubusercontent.com/u/115467130?v=4" width="100" height="100" style="border-radius: 50%;">
</a>

**Mehmet Doğan**

[GitHub Profili](https://github.com/mehmetdogandev)

