#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// --- Definições do DHT11 ---
#define DHTPIN 15      // Pino GPIO15 conectado ao DHT11
#define DHTTYPE DHT11  // Tipo de sensor: DHT11
DHT dht(DHTPIN, DHTTYPE);

// --- Definições do LDR ---
#define LDR_PIN 34 // Pino GPIO34 (entrada analógica) para o LDR

// --- Credenciais WiFi ---
const char* ssid = "Wokwi-GUEST";   // Nome da rede Wi-Fi
const char* password = "";          // Senha da rede Wi-Fi

// --- Configuração do ThingSpeak ---
const char* apiKey = "3INX495B5OU81KKK"; // Write API Key do canal
const char* server = "http://api.thingspeak.com";

void setup() {
  Serial.begin(9600);
  dht.begin();

  // Configuração dos pinos
  pinMode(LDR_PIN, INPUT);

  // Conexão ao Wi-Fi
  Serial.print("Conectando ao WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // --- Leitura dos sensores ---
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int ldrValue = analogRead(LDR_PIN);        // Lê valor do LDR (0 a 4095)
    float luminosity = map(ldrValue, 0, 4095, 0, 100); // Normaliza para % de luminosidade

    // Verificação de erro no DHT11
    if (isnan(h) || isnan(t)) {
      Serial.println("Falha ao ler o sensor DHT11!");
      return;
    }

    // --- Debug no Serial ---
    Serial.println("=== Dados Coletados ===");
    Serial.print("Temperatura: ");
    Serial.print(t);
    Serial.println(" °C");

    Serial.print("Umidade: ");
    Serial.print(h);
    Serial.println(" %");

    Serial.print("Luminosidade: ");
    Serial.print(luminosity);
    Serial.println(" %");
    Serial.println("========================");

    // --- Envio ao ThingSpeak ---
    HTTPClient http;
    String url = String(server) + "/update?api_key=" + apiKey +
                 "&field1=" + String(t) +
                 "&field2=" + String(h) +
                 "&field3=" + String(luminosity);

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.println("Dados enviados ao ThingSpeak!");
      Serial.print("Código HTTP: ");
      Serial.println(httpCode);
    } else {
      Serial.print("Erro ao enviar dados. Código HTTP: ");
      Serial.println(httpCode);
    }

    http.end();
  } else {
    Serial.println("WiFi desconectado. Tentando reconectar...");
    WiFi.begin(ssid, password);
  }

  // ⚠️ Intervalo mínimo do ThingSpeak é 15s
  delay(15000);
}