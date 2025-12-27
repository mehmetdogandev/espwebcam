/*
 * ESP32-CAM Canlı Görüntü Akışı Projesi
 * WiFi üzerinden canlı kamera görüntüsü yayını
 */

#include "esp_camera.h"
#include <WiFi.h>
#include "env.h"

// Kamera Pin Tanımlamaları (AI-Thinker ESP32-CAM)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Web Sunucusu
WiFiServer server(80);

// HTML Sayfası (Modern ve Güzel Arayüz)
const char* html_page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32-CAM Canlı Görüntü</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            align-items: center;
            padding: 20px;
        }
        
        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            padding: 30px;
            max-width: 1200px;
            width: 100%;
        }
        
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 10px;
            font-size: 2.5em;
        }
        
        .subtitle {
            text-align: center;
            color: #666;
            margin-bottom: 30px;
            font-size: 1.1em;
        }
        
        .video-container {
            position: relative;
            width: 100%;
            background: #000;
            border-radius: 15px;
            overflow: hidden;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
            margin-bottom: 20px;
        }
        
        img {
            width: 100%;
            height: auto;
            display: block;
        }
        
        .controls {
            display: flex;
            justify-content: center;
            gap: 15px;
            flex-wrap: wrap;
            margin-top: 20px;
        }
        
        button {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            padding: 12px 30px;
            border-radius: 25px;
            font-size: 16px;
            cursor: pointer;
            transition: all 0.3s ease;
            box-shadow: 0 4px 15px rgba(102, 126, 234, 0.4);
        }
        
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(102, 126, 234, 0.6);
        }
        
        button:active {
            transform: translateY(0);
        }
        
        .status {
            text-align: center;
            margin-top: 20px;
            padding: 15px;
            background: #f0f0f0;
            border-radius: 10px;
            color: #333;
        }
        
        .status.online {
            background: #d4edda;
            color: #155724;
        }
        
        @media (max-width: 768px) {
            h1 {
                font-size: 1.8em;
            }
            
            .container {
                padding: 20px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>📹 ESP32-CAM Canlı Görüntü</h1>
        <p class="subtitle">Gerçek zamanlı kamera akışı</p>
        
        <div class="video-container">
            <img id="stream" src="/stream" alt="Canlı Görüntü">
        </div>
        
        <div class="controls">
            <button onclick="window.location.reload()">🔄 Yenile</button>
            <button onclick="toggleStream()">⏸️ Duraklat / ▶️ Devam</button>
        </div>
        
        <div class="status" id="status">
            <strong>Durum:</strong> <span id="statusText">Bağlanıyor...</span>
        </div>
    </div>
    
    <script>
        let streamRunning = true;
        const img = document.getElementById('stream');
        const statusText = document.getElementById('statusText');
        const statusDiv = document.getElementById('status');
        
        img.onload = function() {
            statusText.textContent = 'Bağlı - Canlı yayın aktif';
            statusDiv.className = 'status online';
        };
        
        img.onerror = function() {
            statusText.textContent = 'Bağlantı hatası - Yeniden deneniyor...';
            statusDiv.className = 'status';
            setTimeout(() => {
                if(streamRunning) {
                    img.src = '/stream?' + new Date().getTime();
                }
            }, 1000);
        };
        
        function toggleStream() {
            streamRunning = !streamRunning;
            if(streamRunning) {
                img.src = '/stream?' + new Date().getTime();
                statusText.textContent = 'Yayın devam ediyor';
            } else {
                img.src = '';
                statusText.textContent = 'Yayın duraklatıldı';
            }
        }
        
        // Otomatik yenileme (her 30 saniyede bir)
        setInterval(() => {
            if(streamRunning) {
                img.src = '/stream?' + new Date().getTime();
            }
        }, 30000);
    </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nESP32-CAM Canlı Görüntü Projesi Başlatılıyor...");
  
  // Kamera Ayarları
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Kalite ayarları (daha yüksek kalite için daha fazla RAM gerekir)
  // FRAMESIZE_QQVGA (160x120) - En hızlı, en az RAM
  // FRAMESIZE_QVGA (320x240) - Hızlı, az RAM
  // FRAMESIZE_VGA (640x480) - Orta hız, orta RAM
  // FRAMESIZE_SVGA (800x600) - Yavaş, çok RAM
  // FRAMESIZE_XGA (1024x768) - Çok yavaş, çok RAM
  config.frame_size = FRAMESIZE_VGA;  // 640x480 - İyi kalite/hız dengesi
  config.jpeg_quality = 12;  // 0-63, düşük sayı = yüksek kalite
  config.fb_count = 2;  // Frame buffer sayısı
  
  // Kamera başlatma
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Kamera başlatılamadı! Hata kodu: 0x%x\n", err);
    return;
  }
  
  Serial.println("✓ Kamera başarıyla başlatıldı");
  
  // WiFi Bağlantısı
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("WiFi'ye bağlanılıyor");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi'ye başarıyla bağlandı!");
    Serial.print("IP Adresi: ");
    Serial.println(WiFi.localIP());
    Serial.println("Tarayıcınızda bu IP adresini açarak canlı görüntüyü görebilirsiniz.");
  } else {
    Serial.println("\n✗ WiFi'ye bağlanılamadı!");
    Serial.println("Lütfen WiFi bilgilerinizi kontrol edin.");
    return;
  }
  
  // Web Sunucusu Başlatma
  server.begin();
  Serial.println("✓ Web sunucusu başlatıldı");
  Serial.println("Hazır! Tarayıcınızda http://" + WiFi.localIP().toString() + " adresini açın.");
}

void loop() {
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("Yeni istemci bağlandı");
    String currentLine = "";
    unsigned long timeout = millis() + 5000;  // 5 saniye timeout
    
    while (client.connected() && millis() < timeout) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // HTTP isteği tamamlandı, yanıt gönder
            String request = client.readStringUntil('\r');
            client.flush();
            
            if (request.indexOf("/stream") >= 0) {
              // Canlı görüntü akışı
              camera_fb_t * fb = esp_camera_fb_get();
              if (!fb) {
                Serial.println("Kamera görüntüsü alınamadı");
                client.stop();
                continue;
              }
              
              client.println("HTTP/1.1 200 OK");
              client.println("Content-Type: image/jpeg");
              client.println("Content-Length: " + String(fb->len));
              client.println("Connection: close");
              client.println("Cache-Control: no-cache, no-store, must-revalidate");
              client.println("Pragma: no-cache");
              client.println("Expires: 0");
              client.println();
              client.write(fb->buf, fb->len);
              
              esp_camera_fb_return(fb);
            } else {
              // Ana sayfa (HTML)
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();
              client.println(html_page);
            }
            
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    
    client.stop();
    Serial.println("İstemci bağlantısı kesildi");
  }
}
