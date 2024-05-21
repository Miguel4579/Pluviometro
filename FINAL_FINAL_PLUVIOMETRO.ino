#include <Wire.h>

const float mmPorPulso = 0.173;
float mmTotal = 0;
int sensor = 0;
int estado_inicial = 0;

const int OpenAirReading = 3677;   //calibration data 1
const int WaterReading = 3666;     //calibration data 2
int MoistureLevel = 0;
int SoilMoisturePercentage = 0;

const char apn[] = "movistar.pe";
const char server[] = "api.thingspeak.com";
const char resource[] = "/update";
const int port = 80;

const char apiKey[] = "E238S6TIO6JE9SPV";

#define MODEM_RST   5
#define MODEM_PWKEY 4
#define MODEM_POWER_ON 23
#define MODEM_TX    27
#define MODEM_RX    26
#define I2C_SDA              21
#define I2C_SCL              22

#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb

#include <TinyGsmClient.h>

// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to SIM800 module)
#define SerialAT Serial1

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

TinyGsmClient client(modem);

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  pinMode(25, INPUT);
  pinMode(2,INPUT);

  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  Serial1.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  Serial.println("Initializing modem...");
  modem.restart();

  Serial.print("Connecting to APN: ");
  if (!modem.gprsConnect(apn)) {
    Serial.println(" fail");
    return;
  }
  Serial.println(" OK");
}

void loop() {
  MoistureLevel = analogRead(2);
  SoilMoisturePercentage = map(MoistureLevel, OpenAirReading, WaterReading, 0, 100);
  sensor = digitalRead(25);

  if (sensor != estado_inicial) {
    mmTotal = mmTotal + mmPorPulso;
  }
  estado_inicial = sensor;

  Serial.print(" Cantidad de lluvia: ");
  Serial.print(mmTotal);
  Serial.print(" mm");
  Serial.println();  
  Serial.print(SoilMoisturePercentage);
  Serial.println("%");
  Serial.println(MoistureLevel);

  if (client.connect(server, port)) {
    Serial.println("Connected to server");

    String postStr = "api_key=";
    postStr += apiKey;
    postStr += "&field1=";
    postStr += String(mmTotal);
    postStr += "&field2=";
    postStr += String(SoilMoisturePercentage);

    client.print("POST ");
    client.print(resource);
    client.println(" HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("User-Agent: Arduino");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postStr.length());
    client.println();
    client.print(postStr);

    Serial.println("Request sent");
  } else {
    Serial.println("Connection to server failed");
  }

  delay(20000);
}


