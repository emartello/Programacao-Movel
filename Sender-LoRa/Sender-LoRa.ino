#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include "SSD1306.h"
#include <EmonLib.h>

// Definição dos pinos WIFI LoRa 32
#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI00    26   // GPIO26 -- SX127x's IRQ(Interrupt Request)
#define PIN_BTN 23  //pino Botão
#define PIN_LED 22  //pino LED
#define PABOOST true

EnergyMonitor emon1;
double Irms = 0;
double Irmsacum = 0;
double minutes = 0;
double ConsumoAcumulado = 0;
double Irmsmedia, Kwh;
unsigned long time;
unsigned long mil = 1000;
unsigned long sessenta = 60;
int estadoBtn, ultimoEstadoBtn;
int pino_sct = 12;  //Pino que recebe os dados do sensor de corrente
int indice = 0;   //controla o indice da frequencia atual no array
int controle = 1;
long int j = 0;
float frequencia[6] = {915E6, 915.125E6, 868E6, 868.125E6, 433E6, 433.125E6};   //array de frequencias válidas que utilizaremos para trocar
unsigned int counter = 0;

SSD1306 display(0x3c, 4, 15, 14);

void setup() {
  pinMode(16, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(PIN_BTN, INPUT);
  pinMode(PIN_LED, OUTPUT);
  
  emon1.current(pino_sct, 6.0606);
  Serial.begin(9600);
  
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  delay(1500);
  display.clear();

  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI00);

  if (!LoRa.begin(frequencia[indice], PABOOST))
  {
    display.drawString(0, 0, "Starting LoRa failed!");
    display.display();
    while (1);
  }
  LoRa.enableCrc();
  display.drawString(0, 0, "LoRa Initial success!");
  display.display();
  delay(1000);
  
  estadoBtn = ultimoEstadoBtn = LOW;   //define estado do botao como desligado
  digitalWrite(PIN_LED, LOW);

}

void loop()
{
  time = millis();
  estadoBtn = digitalRead(PIN_BTN);   //recupera o estado do botao
  
  Irms = emon1.calcIrms(1480);
  Serial.print("Corrente: ");
  Serial.println(Irms); // Irms

  if (1) {
    Irmsacum = Irmsacum + Irms;
    j++;
  }
  if ((time / mil) >= sessenta * controle)
  {
    controle++;
    minutes += 1.0;
    Irmsmedia = Irmsacum / j;
    Irmsacum = 0;
    j = 0;
    ConsumoAcumulado = ConsumoAcumulado + Kwh;
  }

  if ( (estadoBtn != ultimoEstadoBtn) && (estadoBtn == HIGH) )
  {
    digitalWrite(PIN_LED, HIGH);  //liga o LED indicador de mudança de frequencia
    if ( (indice + 1) > 5) { //verificação para não estourar o indice do array
      indice = 0;
    }
    else indice++;
    enviarPacote();
    atualizarDisplay();
    mudarFrequencia();
  }
  //caso nao tenha apertado o botão, ou seja, seu estado é DESLIGADO, então a frequencia permanece a mesma
  else {
    enviarPacote();
    atualizarDisplay();
    Kwh = (220 * Irmsmedia) / 1000;
    Serial.println(time);
    Serial.print("KWH: ");
    Serial.println(Kwh);
    Serial.print("Corrente Media: ");
    Serial.println(Irmsmedia);
    Serial.print("CONSUMO ACUMULADO EM KWH: ");
    Serial.println(((ConsumoAcumulado/minutes) / 60)*minutes);
  }
  counter++;  //contador que é enviado (DADO)
  ultimoEstadoBtn = estadoBtn;   //copia o estado atual do botão na variável de verificação da mudança de estado

  //acende LED interno para indicar que enviou o pacote e da um delay
  digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(700);               // wait for a second
  digitalWrite(2, LOW);    // turn the LED off by making the voltage LOW
  delay(500);               // wait for a second
}

//muda a frequencia do Lora, a nova frequencia será de acordo com a variável "INDICE" que pegará no array a nova frequencia
void mudarFrequencia()
{
  if (!LoRa.begin(frequencia[indice], PABOOST))
  {
    display.drawString(0, 0, "Starting LoRa failed!");
    display.display();
    while (1);
  }
  LoRa.enableCrc();
  digitalWrite(PIN_LED, LOW); //desliga o LED indicador da mudança de frequência
}

//envia o pacote na rede Lora
void enviarPacote()
{
  // envia u pacote
  LoRa.beginPacket();
  LoRa.print(indice); //indice do array que contem as frequencias
  LoRa.print("#"); //delimitador
  LoRa.print(counter); //dado
  LoRa.print("#Consumo:");
  LoRa.print(((ConsumoAcumulado/minutes) / 60)*minutes);
  LoRa.print(" KWh");
  LoRa.endPacket();
}

//atualiza as informações no display
void atualizarDisplay()
{
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);

  display.drawString(0, 0, "Enviando Pacote: ");
  display.drawString(90, 0, String(counter));
  display.drawString(0, 20, String(frequencia[indice]));
  display.drawString(90, 20, "I: " + String(indice));
  display.drawString(0, 40, String(Irms) + " Amp");
  display.display();
}
