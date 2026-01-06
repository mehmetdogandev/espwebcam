# 📹 ESP32-CAM Canlı Görüntü Akışı + Servo Kontrol

ESP32-CAM kartı kullanarak WiFi üzerinden canlı kamera görüntüsü yayını yapan ve servo motorlarla kamera kontrolü sağlayan profesyonel Arduino projesi.

---

## 📦 Gerekli Malzemeler

### Ana Bileşenler
- **ESP32-CAM modülü** (AI-Thinker ESP32-CAM)
- **2x Servo Motor** (SG90 veya benzeri, 5V)
- **Güç Kaynağı**: **5V 2A minimum** (USB kablosu yeterli değil, adaptör gerekli!)
- **USB-UART Dönüştürücü** (FTDI veya CP2102 - kod yüklemek için)
- **Jumper kablolar** (erkek-erkek)
- **Breadboard** (isteğe bağlı, bağlantıları düzenlemek için)

### Önemli Not
⚠️ **ESP32-CAM çok fazla güç çeker!** USB kablosu yeterli değildir. Mutlaka **5V 2A veya daha yüksek** güç adaptörü kullanın. Aksi halde "Brownout" hatası alırsınız ve sistem sürekli reset olur.

---

## 🔌 Bağlantı Şeması

### 1. ESP32-CAM Bağlantıları

ESP32-CAM modülünün üzerinde pinler şu şekilde etiketlenmiştir. USB-UART dönüştürücü ile bağlantı yaparken:

| ESP32-CAM Pin | USB-UART Pin | Açıklama |
|---------------|--------------|----------|
| **5V** | **5V** | Güç (kod yükleme sırasında) |
| **GND** | **GND** | Toprak |
| **U0R** | **RX** | Seri iletişim (kodu yüklemek için) |
| **U0T** | **TX** | Seri iletişim (kodu yüklemek için) |
| **IO0** | **GND** | Boot modu için (kod yükleme sırasında GND'ye bağlayın) |

**⚠️ ÖNEMLİ:** Kod yükleme sırasında:
1. **IO0 pinini GND'ye bağlayın** (Boot modu)
2. **RESET butonuna basın ve bırakın**
3. Kodu yükleyin
4. Yükleme tamamlandıktan sonra **IO0 bağlantısını çıkarın**
5. **RESET butonuna tekrar basın**

### 2. Servo Motor Bağlantıları

Servo motorların 3 kablosu vardır:
- **Kırmızı (VCC)**: 5V güç
- **Siyah/Kahverengi (GND)**: Toprak
- **Sarı/Turuncu (Signal)**: Kontrol sinyali

| Servo Motor | ESP32-CAM Pin | Açıklama |
|-------------|---------------|----------|
| **Pan Servo (Yatay)** | **GPIO 14** | Sağa-sola hareket |
| **Tilt Servo (Dikey)** | **GPIO 15** | Yukarı-aşağı hareket |

**Servo Motor Güç Bağlantısı:**
- Servo motorların **VCC (kırmızı)** kablolarını **5V güç kaynağına** bağlayın
- Servo motorların **GND (siyah)** kablolarını **ESP32-CAM'in GND'sine** bağlayın
- **Signal (sarı)** kablolarını sırasıyla GPIO 14 ve GPIO 15'e bağlayın

**⚠️ ÖNEMLİ:** Servo motorlar ESP32-CAM'in 5V pininden çekilmemelidir! Servo motorlar çok fazla akım çeker ve brownout'a neden olur. Mutlaka **ayrı bir 5V güç kaynağı** kullanın veya servo motorlar için ayrı bir güç kaynağı bağlayın.

### 3. Güç Bağlantısı (Çalışma Modu)

Kod yüklendikten ve test edildikten sonra, ESP32-CAM'i çalıştırmak için:

1. **5V 2A güç adaptörünü** ESP32-CAM'in **5V pinine** bağlayın
2. **GND'yi** güç adaptörünün **GND'sine** bağlayın
3. USB-UART dönüştürücüyü çıkarabilirsiniz (artık gerekli değil)

**Alternatif:** Eğer USB-UART dönüştürücünüz 5V sağlayabiliyorsa, onu da kullanabilirsiniz ama genellikle yeterli değildir.

---

## 💻 Yazılım Kurulumu

### 1. Arduino IDE Kurulumu

1. [Arduino IDE](https://www.arduino.cc/en/software) indirin ve kurun
2. Arduino IDE'yi açın

### 2. ESP32 Board Support Package Kurulumu

1. Arduino IDE'de **Dosya > Tercihler** menüsüne gidin
2. **Ek Kart Yöneticisi URL'leri** bölümüne şu adresi ekleyin:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. **Tamam** butonuna tıklayın
4. **Araçlar > Kart > Kart Yöneticisi** menüsüne gidin
5. Arama kutusuna **"esp32"** yazın
6. **"esp32 by Espressif Systems"** paketini bulun ve **Yükle** butonuna tıklayın
7. Kurulum tamamlanana kadar bekleyin (birkaç dakika sürebilir)

### 3. Gerekli Kütüphaneleri Kurun

Arduino IDE'de **Araçlar > Kütüphaneleri Yönet** menüsünden şu kütüphaneleri arayıp kurun:

1. **ESPAsyncWebServer** (mehmetdogan tarafından)
2. **AsyncTCP** (mehmetdogan tarafından)
3. **ESP32Servo** (Kevin Harrington tarafından)

**Not:** `esp_camera.h` ve `WiFi.h` kütüphaneleri ESP32 board support package ile birlikte gelir, ayrıca kurmanıza gerek yoktur.

### 4. Proje Dosyalarını Hazırlayın

1. Proje klasörünü bilgisayarınıza indirin veya klonlayın
2. Arduino IDE'de **Dosya > Aç** menüsünden `espwebcam.ino` dosyasını açın
3. `env_example.h` dosyasını `env.h` olarak kopyalayın:
   - Windows: Dosyayı kopyalayıp adını değiştirin
   - Mac/Linux: Terminal'de şu komutu çalıştırın:
     ```bash
     cp env_example.h env.h
     ```

### 5. WiFi Ayarlarını Yapılandırın

1. Arduino IDE'de `env.h` dosyasını açın
2. Aşağıdaki satırları kendi WiFi bilgilerinizle değiştirin:
   ```cpp
   const char* ssid = "WIFI_ADI";        // Kendi WiFi ağ adınızı girin
   const char* password = "WIFI_SIFRESI"; // Kendi WiFi şifrenizi girin
   ```
   
   Örnek:
   ```cpp
   const char* ssid = "Ev_WiFi";
   const char* password = "sifre1234";
   ```
3. Dosyayı kaydedin (**Ctrl+S** veya **Cmd+S**)

### 6. Arduino IDE Ayarları

1. **Araçlar > Kart** menüsünden **"AI-Thinker ESP32-CAM"** seçin
2. **Araçlar > Port** menüsünden USB-UART dönüştürücünüzün portunu seçin
3. **Araçlar > Upload Speed** menüsünden **"115200"** seçin (daha yavaş ama daha stabil)

---

## 📤 Kodu ESP32-CAM'e Yükleme

### Adım Adım Yükleme

1. **IO0 pinini GND'ye bağlayın** (Boot modu için)
2. **RESET butonuna basın ve bırakın**
3. Arduino IDE'de **Yükle** butonuna tıklayın (✓ ikonu) veya **Ctrl+U** (Windows) / **Cmd+U** (Mac) tuşlarına basın
4. Yükleme işlemi başlayacak. Seri monitörde ilerlemeyi görebilirsiniz
5. Yükleme tamamlandığında "Yükleme tamamlandı" mesajını göreceksiniz
6. **IO0 bağlantısını çıkarın**
7. **RESET butonuna tekrar basın**

**⚠️ Sorun Yaşıyorsanız:**
- IO0'un GND'ye bağlı olduğundan emin olun
- Upload speed'i 115200'den daha düşük bir değere (9600) düşürmeyi deneyin
- USB-UART dönüştürücünüzün sürücülerinin yüklü olduğundan emin olun
- Farklı bir USB kablosu deneyin

---

## 🚀 İlk Çalıştırma

### 1. Seri Monitörü Açın

1. Arduino IDE'de **Araçlar > Seri Monitör** menüsüne gidin (veya **Ctrl+Shift+M** / **Cmd+Shift+M**)
2. Sağ alt köşedeki baud hızını **115200** olarak ayarlayın
3. ESP32-CAM'in **RESET** butonuna basın

### 2. WiFi Bağlantısını Kontrol Edin

Seri monitörde şu mesajları görmelisiniz:

```
ESP32-CAM Canlı Görüntü + Servo Kontrol Projesi
================================================
✓ Kamera başarıyla başlatıldı
Servo motorlar başlatılıyor...
✓ Pan ve Tilt servolar başlatıldı
  Pan: 90°, Tilt: 90°
WiFi'ye bağlanılıyor: Ev_WiFi
...
✓ WiFi'ye başarıyla bağlandı!
IP Adresi: 192.168.1.100
RSSI: -44 dBm
✓ Web sunucusu başlatıldı
✓ WebSocket hazır

Hazır! Tarayıcınızda http://192.168.1.100 adresini açın.
```

**IP adresini not edin!** Bu adresi tarayıcınızda açacaksınız.

### 3. Web Arayüzünü Açın

1. Bilgisayarınızın veya telefonunuzun tarayıcısını açın
2. Seri monitörde gördüğünüz **IP adresini** adres çubuğuna yazın
3. Enter'a basın
4. Canlı kamera görüntüsü ve servo kontrol paneli görünecektir

---

## 🎮 Kullanım

### Servo Motor Kontrolü

Web arayüzünde sağ tarafta **Servo Kontrol** paneli bulunur:

- **Pan (Yatay) Slider**: Servo motoru sağa-sola hareket ettirir (0-180 derece, 10'ar derece adımlarla)
- **Tilt (Dikey) Slider**: Servo motoru yukarı-aşağı hareket ettirir (0-180 derece, 10'ar derece adımlarla)
- **Merkeze Al Butonu**: Her iki servo motoru da 90 dereceye (merkez pozisyon) getirir

**Not:** Slider'lar 10'ar derece adımlarla çalışır. Örneğin 90'dan 100'e, 100'den 110'a şeklinde hareket eder.

### Görüntü Kalitesi Ayarları

Web arayüzünde **Görüntü Kalitesi** panelinden:

- **Çözünürlük**: QQVGA (160x120), QVGA (320x240), VGA (640x480), SVGA (800x600)
- **JPEG Kalitesi**: 10-40 arası (düşük sayı = yüksek kalite, yüksek sayı = düşük kalite)

**Öneri:** Başlangıç için **QVGA (320x240)** ve **JPEG Kalitesi: 15** ayarları idealdir. Daha yüksek kalite daha fazla güç tüketir ve brownout'a neden olabilir.

---

## ⚙️ Güç Yönetimi ve Brownout Önleme

Proje, güç tüketimini minimize etmek için optimize edilmiştir:

- **CPU Frekansı**: 160 MHz (varsayılan 240 MHz yerine)
- **Kamera Clock**: 10 MHz (varsayılan 20 MHz yerine)
- **WiFi Güç**: Düşük güç modu
- **Frame Rate**: ~10 FPS (güç tüketimini azaltır)

**Yine de brownout hatası alıyorsanız:**

1. **Güç kaynağınızı kontrol edin**: Mutlaka 5V 2A veya daha yüksek güç adaptörü kullanın
2. **Servo motorları ayrı güç kaynağından besleyin**: ESP32-CAM'in 5V pininden çekmeyin
3. **Frame rate'i daha da düşürün**: `espwebcam.ino` dosyasında `delay(100)` değerini `delay(150)` veya `delay(200)` yapın
4. **Kamera kalitesini düşürün**: Web arayüzünden QQVGA çözünürlüğü seçin

---

## 🐛 Sorun Giderme

### WiFi'ye Bağlanamıyor

- `env.h` dosyasındaki SSID ve şifre bilgilerini kontrol edin
- WiFi ağınızın 2.4 GHz bandında olduğundan emin olun (ESP32-CAM 5 GHz desteklemez)
- WiFi şifresinde özel karakterler varsa, bunları kontrol edin
- Seri monitörde hata mesajlarını okuyun

### Kamera Görüntüsü Gelmiyor

- Seri monitörde "Kamera başlatılamadı" hatası varsa, kamera modülünün düzgün bağlı olduğundan emin olun
- WebSocket bağlantısının kurulduğundan emin olun (seri monitörde "Camera WebSocket istemci bağlandı" mesajını görmelisiniz)
- Tarayıcınızın JavaScript'inin aktif olduğundan emin olun
- Farklı bir tarayıcı deneyin (Chrome, Firefox, Safari)

### Servo Motorlar Hareket Etmiyor

- Servo motorların sinyal kablolarının (sarı) GPIO 14 ve GPIO 15'e doğru bağlı olduğundan emin olun
- Servo motorların güç kablolarının (kırmızı) 5V'a bağlı olduğundan emin olun
- Servo motorların toprak kablolarının (siyah) GND'ye bağlı olduğundan emin olun
- Seri monitörde servo komutlarının geldiğini kontrol edin ("ServoInput: Key,Value = [Pan,90]" gibi mesajlar)

### Brownout Hatası (Sürekli Reset)

- Güç kaynağınızın en az 5V 2A olduğundan emin olun
- Servo motorları ESP32-CAM'in 5V pininden çekmeyin, ayrı güç kaynağı kullanın
- USB kablosu kullanıyorsanız, mutlaka güçlü bir adaptör kullanın
- Frame rate'i düşürün (`delay(100)` değerini artırın)
- Kamera kalitesini düşürün (QQVGA çözünürlük)

### WebSocket Bağlantısı Kopuyor

- WiFi sinyal gücünü kontrol edin (RSSI değeri -70'ten daha iyi olmalı)
- ESP32-CAM ile router arasındaki mesafeyi azaltın
- WiFi kanalını değiştirmeyi deneyin (router ayarlarından)
- WebSocket ping/pong mekanizması zaten aktif, ancak bağlantı hala kopuyorsa güç kaynağını kontrol edin

---

## 📋 Teknik Özellikler

- **Kamera**: OV2640 (2MP)
- **Çözünürlük Seçenekleri**: QQVGA, QVGA, VGA, SVGA
- **Frame Rate**: ~10 FPS (ayarlanabilir)
- **Servo Kontrol**: 2 eksen (Pan/Tilt), 0-180 derece, 10'ar derece adımlarla
- **WebSocket**: AsyncWebServer ile stabil bağlantı
- **WiFi**: 2.4 GHz, WPA/WPA2
- **Güç Tüketimi**: Optimize edilmiş (brownout önleme)

---

## 📝 Kod Yapısı

- `espwebcam.ino`: Ana program dosyası
- `env.h`: WiFi bilgileri (git'e eklenmez)
- `env_example.h`: WiFi bilgileri örnek dosyası

**Önemli:** `env.h` dosyası hassas bilgiler içerdiği için `.gitignore` dosyasına eklenmiştir. GitHub'a yüklerken sadece `env_example.h` dosyası yüklenir.

---

## 🔄 Güncellemeler

Projeyi güncellerken:

1. Yeni kodu indirin
2. `env.h` dosyanızı koruyun (silmeden önce yedek alın)
3. Yeni kodu yükleyin
4. `env.h` dosyasını geri kopyalayın

---

## 📄 Lisans

Bu proje açık kaynak kodludur.

---

## 👨‍💻 Geliştirici

**Mehmet Doğan**

[GitHub Profili](https://github.com/mehmetdogandev)

---

## 🙏 Teşekkürler

Bu projeyi kullandığınız için teşekkürler! Sorularınız veya önerileriniz için GitHub'da issue açabilirsiniz.

**İyi kullanımlar! 🚀**
