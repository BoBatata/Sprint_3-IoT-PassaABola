#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>

//Variaveis do MPU6050
Adafruit_MPU6050 mpu;

float acceleration;
float rotation; 
float temp;

//Variaveis do PulseSensor
int PulseSensorPurplePin = 34;
int LED13 = 13;

int Signal;
int Threshold = 550;  

// Credenciais para Conectar com outros dispositivos
const char* ssid = "Wokwi-GUEST"; // Rede Wi-Fi
const char* password = ""; // Senha da rede Wi-Fi
const char* apiKey = "Z0X5IIN7AKLKEN90"; // Write API Key
const char* server = "http://api.thingspeak.com"; // Servidor ThingSpeak


void setup(void) {
  pinMode(LED13,OUTPUT); 

  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Wifi conectado!");

  while (!Serial)
    delay(10);

  Serial.println("Adafruit MPU6050 test!");

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  Serial.println("");
  delay(100);
}

void loop() {
  PulseSensor();
  
  MPU();

  WifiConnect();
}

void PulseSensor()
{
  Signal = analogRead(PulseSensorPurplePin);

  Serial.println("Heartbeat: ");
  Serial.println(Signal);                  
 
  if(Signal > Threshold){
    digitalWrite(LED13,HIGH);
  } else {
    digitalWrite(LED13,LOW);
  }
  
  delay(10);
}

void MPU()
{
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" degC");

  Serial.println("");

  delay(500);
}

void WifiConnect()
{

  if (WiFi.status() == WL_CONNECTED) {
    // Leitura dos sensores
    float pulse = Signal;
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    if (isnan(pulse)) {
      Serial.println("Falha ao ler!");
      return;
    }

    // Envio de dados para o ThingSpeak
    HTTPClient http;
    String url = String(server) + "/update?api_key=" + apiKey + "&field1=" + String(Signal) +
         "&field2=" + String(temp.temperature) + "&field3=" + String(a.acceleration.x);
    http.begin(url);

    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString(); // Resposta da requisição HTTP
      Serial.print("  Dados enviados ao ThingSpeak.");
      Serial.print("Código HTTP: ");
      Serial.print(httpCode);
      Serial.print("  Resposta: ");
      Serial.print(payload);
      Serial.print("  t: ");
    } else {
      Serial.print("Erro ao enviar dados. Código HTTP: ");
      Serial.println(httpCode);
    }
    
    http.end();
  } else {
    Serial.println("WiFi não conectado. Tentando reconectar...");
  }
  
  // Espera 2,5 segundos para enviar a requisição novamente
  delay(2500);
}