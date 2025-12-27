/*
 * ESP32-CAM Canlı Görüntü Akışı + Servo Kontrol
 * AsyncWebServer + WebSocket ile stabil ve profesyonel yapı
 */

#include "esp_camera.h"
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Servo.h>
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

// Servo Motor Pin Tanımlamaları
#define PAN_PIN  14   // Pan (Yatay) servo
#define TILT_PIN 15   // Tilt (Dikey) servo

// Servo nesneleri (ESP32Servo kütüphanesi Servo class'ını kullanır)
Servo panServo;
Servo tiltServo;

// Servo pozisyonları
int panAngle = 90;   // 0-180 arası
int tiltAngle = 90;  // 0-180 arası

// AsyncWebServer ve WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// WiFi bağlantı durumu
unsigned long lastWiFiCheck = 0;
const unsigned long wifiCheckInterval = 30000; // 30 saniyede bir kontrol

// HTML Sayfası (Modern Profesyonel Tasarım + Servo Kontrolü)
const char* html_page = R"rawliteral(
<!DOCTYPE html>
<html lang="tr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32-CAM | Canlı Görüntü + Servo Kontrol</title>
    <style>
        :root {
            --bg: oklch(0.99 0.002 264);
            --fg: oklch(0.15 0.02 264);
            --card: oklch(0.985 0.003 264);
            --card-fg: oklch(0.15 0.02 264);
            --primary: oklch(0.5 0.2 264);
            --primary-fg: oklch(0.99 0.002 264);
            --secondary: oklch(0.96 0.005 264);
            --secondary-fg: oklch(0.2 0.02 264);
            --muted: oklch(0.965 0.006 264);
            --muted-fg: oklch(0.45 0.015 264);
            --accent: oklch(0.94 0.015 264);
            --accent-fg: oklch(0.25 0.03 264);
            --border: oklch(0.9 0.008 264);
            --ring: oklch(0.5 0.2 264);
            --success: oklch(0.65 0.2 160);
            --radius: 0.75rem;
        }
        
        .dark {
            --bg: oklch(0.12 0.015 264);
            --fg: oklch(0.95 0.005 264);
            --card: oklch(0.15 0.018 264);
            --card-fg: oklch(0.95 0.005 264);
            --primary: oklch(0.65 0.2 264);
            --primary-fg: oklch(0.98 0.003 264);
            --secondary: oklch(0.22 0.02 264);
            --secondary-fg: oklch(0.92 0.008 264);
            --muted: oklch(0.2 0.018 264);
            --muted-fg: oklch(0.6 0.015 264);
            --accent: oklch(0.28 0.03 264);
            --accent-fg: oklch(0.95 0.005 264);
            --border: oklch(1 0 0 / 12%);
            --ring: oklch(0.65 0.2 264);
            --success: oklch(0.7 0.22 160);
        }
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Roboto', sans-serif;
            background: var(--bg);
            color: var(--fg);
            min-height: 100vh;
            padding: 1.5rem;
            transition: background-color 0.3s ease, color 0.3s ease;
        }
        
        .header {
            max-width: 1200px;
            margin: 0 auto 1.5rem;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        
        .title-section h1 {
            font-size: 1.75rem;
            font-weight: 600;
            color: var(--fg);
            margin-bottom: 0.25rem;
        }
        
        .title-section p {
            color: var(--muted-fg);
            font-size: 0.875rem;
        }
        
        .theme-toggle {
            background: var(--secondary);
            color: var(--secondary-fg);
            border: 1px solid var(--border);
            padding: 0.5rem 1rem;
            border-radius: calc(var(--radius) - 2px);
            cursor: pointer;
            font-size: 0.875rem;
            transition: all 0.2s ease;
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }
        
        .theme-toggle:hover {
            background: var(--accent);
            color: var(--accent-fg);
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            display: grid;
            grid-template-columns: 1fr 300px;
            gap: 1.5rem;
        }
        
        .main-content {
            display: flex;
            flex-direction: column;
            gap: 1.5rem;
        }
        
        .card {
            background: var(--card);
            border: 1px solid var(--border);
            border-radius: var(--radius);
            padding: 1.5rem;
            box-shadow: 0 1px 3px 0 rgba(0, 0, 0, 0.1);
        }
        
        .video-container {
            position: relative;
            width: 100%;
            background: #000;
            border-radius: calc(var(--radius) - 2px);
            overflow: hidden;
            aspect-ratio: 4/3;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        
        .video-container img {
            width: 100%;
            height: 100%;
            object-fit: contain;
            display: block;
        }
        
        .controls {
            display: flex;
            justify-content: center;
            gap: 0.75rem;
            flex-wrap: wrap;
        }
        
        button {
            background: var(--primary);
            color: var(--primary-fg);
            border: none;
            padding: 0.625rem 1.25rem;
            border-radius: calc(var(--radius) - 2px);
            font-size: 0.875rem;
            font-weight: 500;
            cursor: pointer;
            transition: all 0.2s ease;
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }
        
        button:hover {
            opacity: 0.9;
            transform: translateY(-1px);
        }
        
        button.secondary {
            background: var(--secondary);
            color: var(--secondary-fg);
        }
        
        .servo-control {
            display: flex;
            flex-direction: column;
            gap: 1.5rem;
        }
        
        .servo-item {
            display: flex;
            flex-direction: column;
            gap: 0.75rem;
        }
        
        .servo-item label {
            font-weight: 500;
            font-size: 0.875rem;
            color: var(--fg);
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        
        .servo-item .value {
            color: var(--primary);
            font-weight: 600;
        }
        
        .servo-item input[type="range"] {
            width: 100%;
            height: 6px;
            border-radius: 3px;
            background: var(--muted);
            outline: none;
            -webkit-appearance: none;
        }
        
        .servo-item input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 18px;
            height: 18px;
            border-radius: 50%;
            background: var(--primary);
            cursor: pointer;
            transition: all 0.2s ease;
        }
        
        .servo-item input[type="range"]::-webkit-slider-thumb:hover {
            transform: scale(1.1);
        }
        
        .servo-item input[type="range"]::-moz-range-thumb {
            width: 18px;
            height: 18px;
            border-radius: 50%;
            background: var(--primary);
            cursor: pointer;
            border: none;
        }
        
        .status {
            text-align: center;
            padding: 1rem;
            background: var(--muted);
            border: 1px solid var(--border);
            border-radius: calc(var(--radius) - 2px);
            color: var(--muted-fg);
            font-size: 0.875rem;
            transition: all 0.3s ease;
        }
        
        .status.online {
            background: oklch(0.94 0.1 160);
            color: oklch(0.25 0.15 160);
            border-color: var(--success);
        }
        
        .dark .status.online {
            background: oklch(0.3 0.1 160);
            color: oklch(0.9 0.05 160);
        }
        
        .status strong {
            font-weight: 600;
            margin-right: 0.5rem;
        }
        
        @media (max-width: 968px) {
            .container {
                grid-template-columns: 1fr;
            }
        }
        
        @media (max-width: 768px) {
            body {
                padding: 1rem;
            }
            
            .header {
                flex-direction: column;
                align-items: flex-start;
                gap: 1rem;
            }
            
            .title-section h1 {
                font-size: 1.5rem;
            }
            
            .card {
                padding: 1rem;
            }
        }
    </style>
</head>
<body>
    <div class="header">
        <div class="title-section">
            <h1>📹 ESP32-CAM</h1>
            <p>Canlı Görüntü + Servo Kontrol</p>
        </div>
        <button class="theme-toggle" onclick="toggleTheme()" id="themeBtn">
            <span id="themeIcon">🌙</span>
            <span id="themeText">Karanlık</span>
        </button>
    </div>
    
    <div class="container">
        <div class="main-content">
            <div class="card">
                <div class="video-container">
                    <img id="stream" src="" alt="Canlı Görüntü">
                </div>
                
                <div class="controls" style="margin-top: 1rem;">
                    <button onclick="startStream()">
                        <span>▶</span>
                        <span>Başlat</span>
                    </button>
                    <button class="secondary" onclick="stopStream()">
                        <span>⏸</span>
                        <span>Durdur</span>
                    </button>
                    <button class="secondary" onclick="window.location.reload()">
                        <span>🔄</span>
                        <span>Yenile</span>
                    </button>
                </div>
                
                <div class="status" id="status">
                    <strong>Durum:</strong>
                    <span id="statusText">Bağlanıyor...</span>
                </div>
            </div>
        </div>
        
        <div class="servo-control">
            <div class="card">
                <h2 style="margin-bottom: 1rem; font-size: 1.125rem; font-weight: 600;">Servo Kontrol</h2>
                
                <div class="servo-item">
                    <label>
                        <span>Pan (Yatay)</span>
                        <span class="value" id="panValue">90°</span>
                    </label>
                    <input type="range" min="0" max="180" value="90" id="panSlider" 
                           oninput='sendServoCommand("Pan", this.value)'>
                </div>
                
                <div class="servo-item">
                    <label>
                        <span>Tilt (Dikey)</span>
                        <span class="value" id="tiltValue">90°</span>
                    </label>
                    <input type="range" min="0" max="180" value="90" id="tiltSlider"
                           oninput='sendServoCommand("Tilt", this.value)'>
                </div>
                
                <button class="secondary" onclick="resetServos()" style="margin-top: 1rem; width: 100%;">
                    <span>↺</span>
                    <span>Merkeze Al</span>
                </button>
            </div>
        </div>
    </div>
    
    <script>
        // Theme Management
        const themeBtn = document.getElementById('themeBtn');
        const themeIcon = document.getElementById('themeIcon');
        const themeText = document.getElementById('themeText');
        
        function initTheme() {
            const saved = localStorage.getItem('theme') || 'light';
            document.documentElement.classList.toggle('dark', saved === 'dark');
            updateThemeUI(saved === 'dark');
        }
        
        function updateThemeUI(isDark) {
            themeIcon.textContent = isDark ? '☀️' : '🌙';
            themeText.textContent = isDark ? 'Aydınlık' : 'Karanlık';
        }
        
        function toggleTheme() {
            const isDark = document.documentElement.classList.toggle('dark');
            localStorage.setItem('theme', isDark ? 'dark' : 'light');
            updateThemeUI(isDark);
        }
        
        // WebSocket Management
        let ws = null;
        let streamRunning = false;
        const img = document.getElementById('stream');
        const statusText = document.getElementById('statusText');
        const statusDiv = document.getElementById('status');
        
        function connectWebSocket() {
            const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
            const wsUrl = `${protocol}//${window.location.host}/ws`;
            
            ws = new WebSocket(wsUrl);
            
            ws.onopen = function() {
                console.log('WebSocket bağlandı');
                statusText.textContent = 'WebSocket bağlı';
                statusDiv.className = 'status online';
                if (streamRunning) {
                    startStream();
                }
            };
            
            ws.onmessage = function(event) {
                if (event.data instanceof Blob) {
                    // Görüntü verisi
                    const url = URL.createObjectURL(event.data);
                    img.src = url;
                    // Eski URL'i temizle
                    img.onload = function() {
                        URL.revokeObjectURL(url);
                    };
                } else {
                    // Servo pozisyon güncellemesi
                    const data = event.data.split(',');
                    if (data[0] === 'Pan') {
                        document.getElementById('panValue').textContent = data[1] + '°';
                        document.getElementById('panSlider').value = data[1];
                    } else if (data[0] === 'Tilt') {
                        document.getElementById('tiltValue').textContent = data[1] + '°';
                        document.getElementById('tiltSlider').value = data[1];
                    }
                }
            };
            
            ws.onerror = function(error) {
                console.error('WebSocket hatası:', error);
                statusText.textContent = 'WebSocket hatası';
                statusDiv.className = 'status';
            };
            
            ws.onclose = function() {
                console.log('WebSocket bağlantısı kapandı');
                statusText.textContent = 'Yeniden bağlanılıyor...';
                statusDiv.className = 'status';
                // 3 saniye sonra yeniden bağlan
                setTimeout(connectWebSocket, 3000);
            };
        }
        
        function sendServoCommand(servo, value) {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(`${servo},${value}`);
                // UI güncellemesi
                if (servo === 'Pan') {
                    document.getElementById('panValue').textContent = value + '°';
                } else if (servo === 'Tilt') {
                    document.getElementById('tiltValue').textContent = value + '°';
                }
            }
        }
        
        function resetServos() {
            sendServoCommand('Pan', 90);
            sendServoCommand('Tilt', 90);
            document.getElementById('panSlider').value = 90;
            document.getElementById('tiltSlider').value = 90;
        }
        
        function startStream() {
            streamRunning = true;
            statusText.textContent = 'Akış başlatıldı';
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send('START_STREAM');
            }
        }
        
        function stopStream() {
            streamRunning = false;
            img.src = '';
            statusText.textContent = 'Akış durduruldu';
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send('STOP_STREAM');
            }
        }
        
        // Initialize
        initTheme();
        connectWebSocket();
        window.onload = function() {
            startStream();
        };
    </script>
</body>
</html>
)rawliteral";

// WiFi bağlantısını kontrol et ve yeniden bağlan
void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi bağlantısı kesildi! Yeniden bağlanılıyor...");
    WiFi.disconnect();
    delay(1000);
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi'ye yeniden bağlandı!");
      Serial.print("IP Adresi: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nWiFi'ye bağlanılamadı!");
    }
  }
}

// WebSocket event handler
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                     AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("WebSocket istemci #%u bağlandı: %s\n", client->id(), client->remoteIP().toString().c_str());
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("WebSocket istemci #%u bağlantısı kesildi\n", client->id());
  } else if (type == WS_EVT_DATA) {
    // Servo komutu işle
    String message = String((char*)data);
    message.trim();
    
    Serial.printf("WebSocket mesajı: %s\n", message.c_str());
    
    // Mesaj formatı: "Pan,120" veya "Tilt,90"
    int commaIndex = message.indexOf(',');
    if (commaIndex > 0) {
      String servoName = message.substring(0, commaIndex);
      int angle = message.substring(commaIndex + 1).toInt();
      
      if (servoName == "Pan" && angle >= 0 && angle <= 180) {
        panAngle = angle;
        panServo.write(panAngle);
        Serial.printf("Pan servo: %d derece\n", panAngle);
        // Pozisyonu tüm istemcilere bildir
        ws.textAll("Pan," + String(panAngle));
      } else if (servoName == "Tilt" && angle >= 0 && angle <= 180) {
        tiltAngle = angle;
        tiltServo.write(tiltAngle);
        Serial.printf("Tilt servo: %d derece\n", tiltAngle);
        // Pozisyonu tüm istemcilere bildir
        ws.textAll("Tilt," + String(tiltAngle));
      }
    } else if (message == "START_STREAM") {
      Serial.println("Stream başlatıldı");
    } else if (message == "STOP_STREAM") {
      Serial.println("Stream durduruldu");
    }
  }
}

// Görüntü akışı görev fonksiyonu
void streamTask(void *pvParameters) {
  while (true) {
    // Tüm WebSocket istemcilerine görüntü gönder
    if (ws.count() > 0) {
      camera_fb_t * fb = esp_camera_fb_get();
      if (fb) {
        // Frame boyutunu kontrol et - çok büyükse atla
        if (fb->len < 10000) {  // 10KB'dan küçük frame'ler gönder
          // Her istemciye görüntü gönder
          ws.binaryAll(fb->buf, fb->len);
        }
        esp_camera_fb_return(fb);
      }
    }
    // 100ms = ~10 FPS - Daha stabil ve hızlı
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nESP32-CAM Canlı Görüntü + Servo Kontrol Projesi");
  Serial.println("================================================");
  
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
  
  // Hız optimizasyonu ayarları - Maksimum hız için
  config.frame_size = FRAMESIZE_QQVGA;  // 160x120 - En hızlı, en küçük dosya
  config.jpeg_quality = 30;  // Yüksek sayı = daha küçük dosya = DAHA HIZLI
  config.fb_count = 1;  // Tek buffer - daha hızlı işleme
  
  // Kamera başlatma
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Kamera başlatılamadı! Hata kodu: 0x%x\n", err);
    return;
  }
  Serial.println("✓ Kamera başarıyla başlatıldı");
  
  // Servo Motorlar
  Serial.println("Servo motorlar başlatılıyor...");
  
  panServo.attach(PAN_PIN);
  tiltServo.attach(TILT_PIN);
  panServo.write(panAngle);
  tiltServo.write(tiltAngle);
  Serial.println("✓ Pan ve Tilt servolar başlatıldı");
  Serial.printf("  Pan: %d°, Tilt: %d°\n", panAngle, tiltAngle);
  
  // WiFi Bağlantısı - Güçlendirilmiş ayarlar
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  
  // WiFi güç ayarları (daha stabil bağlantı için)
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  
  Serial.print("WiFi'ye bağlanılıyor: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
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
    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("\n✗ WiFi'ye bağlanılamadı!");
    Serial.println("Lütfen WiFi bilgilerinizi kontrol edin.");
    return;
  }
  
  // WebSocket ayarları
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  
  // Ana sayfa
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", html_page);
  });
  
  // Web sunucusu başlat
  server.begin();
  Serial.println("✓ Web sunucusu başlatıldı");
  Serial.println("✓ WebSocket hazır");
  Serial.println("\nHazır! Tarayıcınızda http://" + WiFi.localIP().toString() + " adresini açın.");
  
  // Görüntü akışı görevi başlat
  xTaskCreatePinnedToCore(
    streamTask,
    "streamTask",
    4096,
    NULL,
    1,
    NULL,
    1  // Core 1'de çalıştır (Core 0 WiFi için)
  );
  Serial.println("✓ Görüntü akışı görevi başlatıldı");
}

void loop() {
  // WiFi bağlantısını düzenli kontrol et
  unsigned long now = millis();
  if (now - lastWiFiCheck >= wifiCheckInterval) {
    lastWiFiCheck = now;
    checkWiFiConnection();
  }
  
  // WebSocket temizliği (bağlantı kopan istemcileri temizle)
  ws.cleanupClients();
  
  delay(100);
}
