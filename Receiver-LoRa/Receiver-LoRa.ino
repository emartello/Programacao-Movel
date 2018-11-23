#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include "SSD1306.h"
#include <WiFi.h>

// Definião dos pinos WIFI LoRa 32
#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI00    26   // GPIO26 -- SX127x's IRQ(Interrupt Request)
#define PABOOST true

/* *****************Wifi******************/
const char* ssid     = "Elvis";
const char* password = "martellonova";
const char* host = "httpbin.org";
String readString;
/* *****************Wifi******************/

SSD1306 display(0x3c, 4, 15, 14);
String rssi = "RSSI --";
String packSize = "--";
String packet ;
float frequencia[6] = {915E6, 915.125E6, 868E6, 868.125E6, 433E6, 433.125E6};   //array de frequencias válidas que utilizaremos para trocar
int indice = 0;   //controla o indice da frequencia atual no array

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high、
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  delay(1000);
  display.clear();

  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI00);

  if (!LoRa.begin(frequencia[indice], PABOOST)) {
    display.drawString(0, 0, "Starting LoRa failed!");
    display.display();
    while (1);
  }
  LoRa.enableCrc();
  display.drawString(0, 0, "LoRa Initial success!");
  display.drawString(0, 10, "Wait for incomm data...");
  display.display();
  delay(1000);
  LoRa.receive();
}

void loop() {
  delay(500);
  int packetSize = LoRa.parsePacket();
  display.drawString(0, 45, "Freq: " + String(frequencia[indice])); //imprime na tela a frequencia atual que está configurada
  display.display();

  if (packetSize) {
    parserPacket(packetSize);
  }
  /*************************************HTTP********************************************/
  delay(2000);
  Serial.print("Conectando a ");
  Serial.println(host);

  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("Conexao Falhou");
    return;
  }

  Serial.println("Conectando...");
  if (client.connect(host, httpPort)) {
    Serial.println("Conectado!");
    client.println("GET /ip HTTP/1.1");   //client.println("GET /titon.php?token=1234 HTTP/1.1");
    client.println("Host: httpbin.org");
    client.println("Connection: close");
    client.println();
  }

  if (client) {
    while (client.connected()) {
      while (client.available()) {
        char c = client.read();
        readString += c;
      }
    }
  }
  Serial.println(readString);
}
/*************************************HTTP********************************************/

void parserPacket(int packetSize) {   //faz o parser do pacote recebido para tratar os dados
  packet = "";
  packSize = String(packetSize, DEC);
  for (int i = 0; i < packetSize; i++) {
    packet += (char) LoRa.read();
  }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
  verificaFrequencia();
  loraData();
}

void verificaFrequencia()   //verifica se no pacote recebido tem um comando para trocar de frequencia
{
  //  Serial.println(packet);
  //o primeiro byte do pacote tem o indice da frequencia que o emissor está usando,
  // colocamos esse valor em CMD para fazeras verificações
  String cmd = String(packet.charAt(0));
  //se o indice recebido é diferente do atual, então trocamos a frequencia
  if (cmd.toInt() != indice)
  {
    if (!LoRa.begin(frequencia[cmd.toInt()], PABOOST))
    {
      display.drawString(0, 0, "Starting LoRa failed!");
      display.display();
      while (1);
    }
    indice = cmd.toInt();
    LoRa.enableCrc();
  }
}

void loraData() {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0 , 15 , "Received " + packSize + " bytes");
  display.drawStringMaxWidth(0 , 26 , 128, packet);

  display.drawString(0, 0, rssi);
  display.display();
}
