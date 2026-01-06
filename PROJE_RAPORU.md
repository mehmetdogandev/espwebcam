# 📋 ESP32-CAM Canlı Görüntü Akışı + Servo Kontrol Projesi
## Teknik Geliştirme Raporu

---

## 📌 Proje Özeti

Bu proje, ESP32-CAM modülü kullanarak WiFi üzerinden canlı kamera görüntüsü yayını yapan ve iki eksenli servo motor kontrolü sağlayan bir IoT sistemidir. Proje, AsyncWebServer ve WebSocket teknolojileri kullanılarak geliştirilmiş, modern web arayüzü ile kullanıcıya sunulmuştur.

---

## 🎯 Proje Hedefleri

1. ESP32-CAM modülünden canlı görüntü akışı sağlamak
2. Web tabanlı kullanıcı arayüzü oluşturmak
3. Servo motorlarla kamera kontrolü (Pan/Tilt) yapmak
4. Stabil WebSocket bağlantısı kurmak
5. Güç tüketimini optimize etmek (Brownout önleme)

---

## 🔧 Donanım Yapısı ve Bağlantılar

### 1. ESP32-CAM Modülü

**Kullanılan Modül:** AI-Thinker ESP32-CAM
- **İşlemci:** ESP32 (Dual-core, 240MHz - projede 160MHz'ye düşürüldü)
- **Kamera:** OV2640 (2MP)
- **WiFi:** 2.4 GHz, 802.11 b/g/n
- **RAM:** 520KB SRAM + PSRAM desteği

**Neden Bu Modül Seçildi?**
- Entegre kamera sensörü
- WiFi bağlantısı
- Yeterli işlem gücü
- Uygun fiyat/performans oranı

### 2. Servo Motor Bağlantıları

**Kullanılan Servo Motorlar:** SG90 veya benzeri (5V, 180°)

**Bağlantı Şeması:**

```
Pan Servo (Yatay Hareket):
├── VCC (Kırmızı)    → 5V Güç Kaynağı (AYRI!)
├── GND (Siyah)      → ESP32-CAM GND
└── Signal (Sarı)    → GPIO 14

Tilt Servo (Dikey Hareket):
├── VCC (Kırmızı)    → 5V Güç Kaynağı (AYRI!)
├── GND (Siyah)      → ESP32-CAM GND
└── Signal (Sarı)    → GPIO 15
```

**Neden GPIO 14 ve 15 Seçildi?**
- Bu pinler ESP32-CAM'de kullanılabilir durumda
- PWM sinyali üretmek için uygun
- Timer çakışması olmayan pinler

**Neden Servo Motorlar Ayrı Güç Kaynağından Besleniyor?**
- Servo motorlar yüksek akım çeker (500mA-1A)
- ESP32-CAM'in 5V pininden çekilirse brownout hatası oluşur
- Sistem stabilitesi için kritik öneme sahip

### 3. Güç Yönetimi

**Ana Güç Kaynağı:** 5V 2A minimum adaptör

**Güç Dağılımı:**
- ESP32-CAM: 5V pininden beslenir
- Servo Motorlar: Ayrı 5V kaynaktan beslenir (ortak GND)

**Neden 2A Minimum?**
- ESP32-CAM: ~200-300mA (WiFi aktifken)
- Kamera: ~100-150mA
- Servo motorlar: ~500-1000mA (hareket sırasında)
- Toplam: ~1-1.5A (güvenlik payı ile 2A)

---

## 💻 Yazılım Mimarisi

### 1. Kütüphane Seçimleri ve Nedenleri

#### a) ESPAsyncWebServer
```cpp
#include <ESPAsyncWebServer.h>
```

**Neden Bu Kütüphane?**
- Asenkron çalışma: Ana loop'u bloklamaz
- WebSocket desteği: Gerçek zamanlı veri aktarımı
- Düşük bellek kullanımı
- Stabil bağlantı yönetimi

**Alternatif:** Standart `WebServer` kütüphanesi
- **Reddedilme Nedeni:** Senkron çalışır, loop'u bloklar, WebSocket desteği zayıf

#### b) AsyncTCP
```cpp
#include <AsyncTCP.h>
```

**Neden Gerekli?**
- ESPAsyncWebServer'ın temel bağımlılığı
- Asenkron TCP bağlantıları için gerekli

#### c) ESP32Servo
```cpp
#include <ESP32Servo.h>
```

**Neden Bu Kütüphane?**
- ESP32 için optimize edilmiş
- Kolay kullanım API'si
- Timer yönetimi otomatik

**Alternatif:** Manuel PWM kontrolü
- **Reddedilme Nedeni:** Daha karmaşık kod, timer çakışması riski

#### d) esp_camera.h
```cpp
#include "esp_camera.h"
```

**Neden Bu Kütüphane?**
- ESP32 için resmi kamera kütüphanesi
- OV2640 sensörü için optimize
- PSRAM desteği

### 2. Kod Yapısı ve Kararlar

#### a) İki Ayrı WebSocket Kullanımı

```cpp
AsyncWebSocket wsCamera("/Camera");      // Sadece görüntü için
AsyncWebSocket wsServoInput("/ServoInput");  // Sadece servo komutları için
```

**Neden İki Ayrı WebSocket?**
1. **Performans:** Görüntü verisi binary, servo komutları text
2. **Stabilite:** Bir bağlantı koparsa diğeri etkilenmez
3. **Yönetim:** Her bağlantı kendi event handler'ına sahip
4. **Buffer Yönetimi:** Ayrı buffer'lar çakışmayı önler

**Alternatif:** Tek WebSocket
- **Reddedilme Nedeni:** Binary ve text veri karışır, buffer yönetimi zorlaşır

#### b) Servo Pozisyon Takibi

```cpp
int panAngle = 90;   // 0-180 arası
int tiltAngle = 90;  // 0-180 arası
```

**Neden Global Değişkenler?**
- WebSocket event handler'larından erişim kolaylığı
- Servo pozisyonlarının sürekli takibi
- Merkez pozisyon (90°) başlangıç değeri

**Neden 90° Başlangıç?**
- Servo motorların mekanik merkez pozisyonu
- Her iki yöne eşit hareket imkanı
- Güvenli başlangıç pozisyonu

#### c) Frame Rate Kontrolü

```cpp
delay(100); // ~10 FPS
```

**Neden 100ms Delay?**
1. **Güç Tüketimi:** Düşük frame rate = düşük güç tüketimi
2. **Brownout Önleme:** Yüksek frame rate brownout'a neden olur
3. **Network:** WiFi bant genişliği sınırlı
4. **Stabilite:** Düşük frame rate daha stabil bağlantı

**Hesaplama:**
- 100ms delay = 10 FPS
- 30ms delay = ~30 FPS (daha fazla güç tüketir)
- 200ms delay = 5 FPS (çok yavaş)

#### d) Buffer Yönetimi

```cpp
if (client->queueIsFull()) {
    return; // Buffer dolu, bu frame'i atla
}
```

**Neden Buffer Kontrolü?**
- WebSocket buffer'ı dolduğunda sistem takılır
- Frame atlama, sistem stabilitesini korur
- Brownout riskini azaltır

**Alternatif:** Buffer dolana kadar bekleme
- **Reddedilme Nedeni:** Sistem donması, brownout riski

### 3. WebSocket Event Handler Yapısı

#### a) Camera WebSocket Handler

```cpp
void onCameraWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                     AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      cameraClientId = client->id();
      break;
    case WS_EVT_DISCONNECT:
      if (cameraClientId == client->id()) {
        cameraClientId = 0;
      }
      break;
    // ...
  }
}
```

**Neden Client ID Takibi?**
- Sadece aktif client'a görüntü gönderilir
- Gereksiz işlem yükü önlenir
- Bağlantı kopunca ID sıfırlanır

#### b) Servo Input WebSocket Handler

```cpp
void onServoInputWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                     AwsEventType type, void *arg, uint8_t *data, size_t len) {
  // Komut işleme
  std::string key, value;
  std::getline(ss, key, ',');
  std::getline(ss, value, ',');
  
  if (key == "Pan" && valueInt >= 0 && valueInt <= 180) {
    panAngle = valueInt;
    panServo.write(panAngle);
  }
}
```

**Neden String Parsing?**
- Basit komut formatı: "Pan,90" veya "Tilt,45"
- Kolay debug
- Genişletilebilir yapı

**Neden Sınır Kontrolü (0-180)?**
- Servo motorların mekanik sınırları
- Sistem güvenliği
- Hatalı komutlardan korunma

### 4. Güç Optimizasyonu Kararları

#### a) CPU Frekansı Düşürme

```cpp
setCpuFrequencyMhz(160); // 240MHz'den 160MHz'ye
```

**Neden 160MHz?**
- Güç tüketimi: ~%30 azalma
- Performans: Hala yeterli (kamera + WiFi için)
- Brownout riski: Önemli ölçüde azalır

**Test Sonuçları:**
- 240MHz: Brownout riski yüksek
- 160MHz: Stabil çalışma
- 80MHz: Çok yavaş, görüntü kalitesi düşer

#### b) Kamera Clock Frekansı

```cpp
config.xclk_freq_hz = 10000000; // 20MHz'den 10MHz'ye
```

**Neden 10MHz?**
- Güç tüketimi: ~%20 azalma
- Görüntü kalitesi: Minimal etki
- Sistem stabilitesi: Artar

#### c) WiFi Güç Seviyesi

```cpp
WiFi.setTxPower(WIFI_POWER_11dBm); // 19.5dBm'den 11dBm'ye
```

**Neden Düşük Güç?**
- Güç tüketimi: ~%40 azalma
- Menzil: Ev içi kullanım için yeterli
- Brownout riski: Önemli ölçüde azalır

**Trade-off:** Daha kısa menzil, ama ev içi kullanım için yeterli

### 5. Web Arayüzü Tasarım Kararları

#### a) Modern CSS Tasarımı

```css
:root {
    --bg: oklch(0.99 0.002 264);
    --primary: oklch(0.5 0.2 264);
    /* ... */
}
```

**Neden OKLCH Renk Sistemi?**
- Modern tarayıcı desteği
- Daha doğru renk gösterimi
- Dark mode desteği kolay

#### b) Dark Mode Desteği

```javascript
function toggleTheme() {
    const isDark = document.documentElement.classList.toggle('dark');
    localStorage.setItem('theme', isDark ? 'dark' : 'light');
}
```

**Neden Dark Mode?**
- Kullanıcı deneyimi
- Göz yorgunluğunu azaltır
- Modern web standartı

#### c) Slider ile Servo Kontrolü

```html
<input type="range" min="0" max="180" step="10" value="90" id="panSlider" 
       oninput='sendServoCommand("Pan", this.value)'>
```

**Neden Step="10"?**
- Kullanıcı isteği: 10'ar derece adımlarla hareket
- Hassas kontrol: 1 derece çok hassas, 10 derece ideal
- Servo motor performansı: 10 derece adımlar daha hızlı

**JavaScript Tarafında Yuvarlama:**
```javascript
const roundedValue = Math.round(value / 10) * 10;
```

**Neden Çift Kontrol?**
- HTML step kontrolü + JavaScript yuvarlama
- Güvenlik: Hatalı değerlerden korunma
- Tutarlılık: Her zaman 10'un katı

### 6. WebSocket Bağlantı Stabilitesi

#### a) Ping/Pong Mekanizması

```cpp
if (now - lastPingTime >= pingInterval) {
    client->ping();
}
```

**Neden Ping/Pong?**
- Bağlantı durumu kontrolü
- Kopan bağlantıları tespit etme
- WebSocket standardı

**Neden 30 Saniye?**
- Çok sık ping: Gereksiz trafik
- Çok seyrek ping: Geç tespit
- 30 saniye: Dengeli

#### b) Exponential Backoff Yeniden Bağlanma

```javascript
const delay = Math.min(1000 * Math.pow(2, cameraReconnectAttempts), 10000);
```

**Neden Exponential Backoff?**
- Sunucu yükünü azaltır
- Ağ sorunlarında daha etkili
- Standart pratik

**Hesaplama:**
- 1. deneme: 1 saniye sonra
- 2. deneme: 2 saniye sonra
- 3. deneme: 4 saniye sonra
- 4. deneme: 8 saniye sonra
- 5+ deneme: 10 saniye (maksimum)

#### c) Client Durumu Kontrolü

```cpp
if (!client || !client->canSend()) {
    cameraClientId = 0;
    return;
}
```

**Neden canSend() Kontrolü?**
- Buffer durumu kontrolü
- Bağlantı durumu kontrolü
- Hatalı gönderimleri önleme

**Alternatif:** Sadece null kontrolü
- **Reddedilme Nedeni:** Buffer doluysa hata oluşur

### 7. Kamera Görüntü İşleme

#### a) Frame Yakalama

```cpp
camera_fb_t * fb = esp_camera_fb_get();
if (!fb) {
    Serial.println("Frame buffer alınamadı");
    return;
}
```

**Neden Null Kontrolü?**
- Kamera hatalarında sistem çökmesini önler
- Debug kolaylığı
- Graceful degradation

#### b) Binary Data Gönderimi

```cpp
wsCamera.binary(cameraClientId, fb->buf, fb->len);
```

**Neden Binary?**
- JPEG verisi binary format
- Text'e çevirmek gereksiz overhead
- Daha hızlı transfer

**Alternatif:** Base64 encoding
- **Reddedilme Nedeni:** %33 daha fazla veri, daha yavaş

#### c) Frame Buffer Yönetimi

```cpp
esp_camera_fb_return(fb);
```

**Neden Hemen Return?**
- Bellek yönetimi: PSRAM sınırlı
- Yeni frame için yer açma
- Sistem stabilitesi

### 8. Servo Motor Kontrolü

#### a) Servo Attach İşlemi

```cpp
panServo.attach(PAN_PIN);
tiltServo.attach(TILT_PIN);
```

**Neden Setup'ta Attach?**
- Bir kez yapılan işlem
- Timer ayarları otomatik
- Performans optimizasyonu

#### b) Servo Write İşlemi

```cpp
panServo.write(panAngle);
```

**Neden Direkt Write?**
- Hızlı yanıt
- Basit API
- Yeterli hassasiyet (10 derece adımlar)

**Alternatif:** Smooth movement
- **Reddedilme Nedeni:** Daha karmaşık, gerek yok

### 9. Hata Yönetimi ve Debug

#### a) Serial Printf Kullanımı

```cpp
Serial.printf("Pan servo: %d derece\n", panAngle);
```

**Neden Printf?**
- Formatlanmış çıktı
- Debug kolaylığı
- Performans: Minimal overhead

#### b) Hata Mesajları

```cpp
Serial.printf("Kamera başlatılamadı! Hata kodu: 0x%x\n", err);
```

**Neden Hex Format?**
- ESP32 hata kodları hex formatında
- Kolay referans
- Debug kolaylığı

### 10. Performans Optimizasyonları

#### a) Single Frame Buffer

```cpp
config.fb_count = 1;
```

**Neden Tek Buffer?**
- Bellek tasarrufu
- Düşük güç tüketimi
- Yeterli performans (10 FPS için)

**Alternatif:** Çift buffer
- **Reddedilme Nedeni:** Daha fazla RAM, gerek yok

#### b) Cleanup Optimizasyonu

```cpp
if (now - lastCleanupTime >= cleanupInterval) {
    wsCamera.cleanupClients();
}
```

**Neden Periyodik Cleanup?**
- Her loop'ta cleanup: Gereksiz overhead
- Hiç cleanup: Bellek sızıntısı
- 5 saniyede bir: Dengeli

---

## 🔍 Karşılaşılan Sorunlar ve Çözümler

### 1. Brownout Hatası

**Sorun:** Sistem sürekli reset oluyor, "Brownout detector was triggered" hatası

**Neden:**
- Yetersiz güç kaynağı (USB kablosu)
- Servo motorlar ESP32-CAM'den çekiliyor
- Yüksek CPU/kamera frekansları

**Çözüm:**
1. CPU frekansını 160MHz'ye düşürdük
2. Kamera clock'u 10MHz'ye düşürdük
3. WiFi gücünü 11dBm'ye düşürdük
4. Frame rate'i 10 FPS'e düşürdük
5. Servo motorları ayrı güç kaynağından besledik

**Sonuç:** Sistem stabil çalışıyor

### 2. WebSocket Bağlantı Kopmaları

**Sorun:** WebSocket bağlantısı sürekli kopuyor

**Neden:**
- Ping/pong mekanizması yoktu
- Client durumu kontrolü eksikti
- Buffer yönetimi yetersizdi

**Çözüm:**
1. 30 saniyede bir ping gönderme eklendi
2. `canSend()` kontrolü eklendi
3. Buffer doluluğu kontrolü eklendi
4. Exponential backoff yeniden bağlanma eklendi

**Sonuç:** Bağlantı stabil hale geldi

### 3. Servo Motor Hareket Sorunları

**Sorun:** Servo motorlar bazen hareket etmiyor

**Neden:**
- Güç kaynağı yetersiz
- Pin bağlantıları hatalı
- Timer çakışması

**Çözüm:**
1. Servo motorlar ayrı güç kaynağından besleniyor
2. Pin numaraları doğrulandı (GPIO 14, 15)
3. ESP32Servo kütüphanesi timer yönetimini otomatik yapıyor

**Sonuç:** Servo motorlar düzgün çalışıyor

### 4. Görüntü Kalitesi vs. Performans Dengesi

**Sorun:** Yüksek kalite brownout'a neden oluyor

**Neden:**
- Yüksek çözünürlük daha fazla işlem gücü gerektirir
- Yüksek JPEG kalitesi daha fazla veri = daha fazla güç

**Çözüm:**
1. Varsayılan QVGA (320x240) seçildi
2. JPEG kalitesi 15 (dengeli)
3. Kullanıcı web arayüzünden ayarlayabilir

**Sonuç:** Dengeli performans ve kalite

---

## 📊 Performans Metrikleri

### Güç Tüketimi

| Mod | Güç Tüketimi | Notlar |
|-----|--------------|--------|
| Bekleme (WiFi kapalı) | ~80mA | Kamera kapalı |
| WiFi bağlı, kamera kapalı | ~120mA | Bağlantı kurulmuş |
| Kamera aktif (QVGA, 10 FPS) | ~250mA | Normal kullanım |
| Servo hareket (anlık) | +500mA | Hareket sırasında |
| **Toplam (normal kullanım)** | **~250-300mA** | Stabil çalışma |

### Frame Rate

| Ayar | FPS | Delay | Güç Tüketimi |
|------|-----|-------|--------------|
| Yüksek | ~30 | 30ms | Yüksek (brownout riski) |
| **Normal (önerilen)** | **~10** | **100ms** | **Orta (stabil)** |
| Düşük | ~5 | 200ms | Düşük (yavaş) |

### WebSocket Bağlantı Süresi

- **İlk bağlantı:** ~1-2 saniye
- **Yeniden bağlantı:** Exponential backoff ile
- **Ping interval:** 30 saniye
- **Timeout:** Otomatik tespit

---

## 🎓 Öğrenilen Dersler

1. **Güç Yönetimi Kritik:** ESP32-CAM güç açısından hassas, mutlaka yeterli güç kaynağı kullanılmalı

2. **WebSocket Stabilitesi:** Ping/pong ve buffer yönetimi olmadan bağlantı kopuyor

3. **Servo Motor Güç:** Servo motorlar mutlaka ayrı güç kaynağından beslenmeli

4. **Frame Rate vs. Güç:** Düşük frame rate brownout'u önler, yeterli performans sağlar

5. **İki WebSocket:** Binary ve text veri ayrı WebSocket'lerde daha stabil

6. **CPU Frekansı:** 160MHz yeterli performans sağlıyor, güç tüketimini azaltıyor

---

## 🔮 Gelecek İyileştirmeler

1. **Smooth Servo Movement:** Servo motorların yumuşak hareketi
2. **Preset Pozisyonlar:** Önceden tanımlı kamera açıları
3. **Motion Detection:** Hareket algılama özelliği
4. **SD Card Kayıt:** Görüntüleri SD karta kaydetme
5. **Multi-Client Support:** Birden fazla client desteği
6. **Authentication:** Web arayüzü için şifre koruması

---

## 📚 Referanslar ve Kaynaklar

- ESP32-CAM Resmi Dokümantasyon
- ESPAsyncWebServer GitHub Repository
- ESP32Servo Kütüphane Dokümantasyonu
- WebSocket Protocol Specification (RFC 6455)
- OV2640 Kamera Sensörü Datasheet

---

## ✅ Sonuç

Bu proje, ESP32-CAM kullanarak başarılı bir şekilde canlı görüntü akışı ve servo motor kontrolü sağlamaktadır. Güç yönetimi, WebSocket stabilitesi ve performans optimizasyonları ile sistem stabil çalışmaktadır. Proje, IoT uygulamaları için iyi bir referans noktası oluşturmaktadır.

**Proje Durumu:** ✅ Tamamlandı ve Test Edildi
**Son Güncelleme:** 2024

---

*Bu rapor, projenin teknik geliştirme sürecini ve alınan kararları dokümante etmek için hazırlanmıştır.*

