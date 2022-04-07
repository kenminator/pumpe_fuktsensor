#include <DHT.h>
#include <Wire.h>

//OLED:
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//TEMPSENSOR:
#define DHTPIN 13
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
unsigned long TempRHmillis = 0;
float t = 0;
float rh = 0;

#define potPin A0 //input for z-punkt fuktnivå
byte potValue = 0; // setter potmeterverdi til 0

#define delayPin A1 //input for delay
int delayValue = 0;

#define pumpetimerPin A2 //input for pumpetimerpotmeter
int pumpetimerValue = 0; //setter pumpetimer til 0
int sistePumpe = 0; //Tid mellom pumpekjøring

#define sensorPin A3    // setter sensorpin
int sensorValue = 0;  // setter sensorpinverdi til 0

#define relPinPump 6 //pin for pumpe
#define relPinSolenoid 7 //pin for soleniod
#define startButton 12 //pin for startknapp
#define startButtonLED 11 //pin for LED i startknapp

//Millisdelay
int period = 1000;
unsigned long time_now = 0;

void setup() {

  pinMode(relPinPump, OUTPUT);
  pinMode(relPinSolenoid, OUTPUT);
  pinMode(startButton, INPUT);
  pinMode(startButtonLED, OUTPUT);
  pinMode(delayPin, INPUT);

  Serial.begin(9600);
  Wire.begin();

  //OLED
  delay(1000);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);

  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.print("Autovanner!");
  display.display();
  

  time_now = millis();
}

void printSerial() {
  Serial.println(String("SensorPin A0: ") + sensorValue);
  Serial.println(String("PotPin A1: ") + potValue);
  Serial.println(String("PumpetimerValue A2: ") + pumpetimerValue);
  Serial.println(String("Delay A4: ") + delayValue);
}

void displayText(int leseDelayTilDisplay, char pumpeStatus, int sistePumpe) {

  // read the value from the sensor:
  sensorValue = map(analogRead(sensorPin), 1023, 0, 0, 100);
  if (sensorValue < 0) {
    sensorValue = 0;
  }

  potValue = map(analogRead(potPin), 0, 1023, 0, 100);
  pumpetimerValue = map(analogRead(pumpetimerPin), 0, 1023, 1, 30);

  //Kommenter ut hvis du vil ha debug i seriellport
  //printSerial();

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);

  //FUKTMÅLER
  display.setCursor(0, 0);
  display.print(String("Fukt:") + sensorValue + "/" + potValue);

  //DELAY
  display.setCursor(0, 16);
  display.print(String("Delay: ") + showTimer(readDelay()));

  //PUMPETID
  display.setCursor(0, 26);
  display.print(String("Pumpetid:") + pumpetimerValue + " sek");

  //PUMPE SIST KJØRT
  display.setCursor(0, 40);
  display.print(String("Forrige:") + ((millis() - sistePumpe) / 1000 / 60) + " min");

  //COUNTDOWN
  display.setCursor(0, 48);
  display.print(String("Tid: ") + leseDelayTilDisplay + " sek");

  //PUMPESTATUS
  display.setCursor(74, 0);
  if (pumpeStatus == 1) {
    display.print("Pumpe:On");
  } else {
    display.print("Pumpe:Off");
  }

  //TEMP
  //Check temp every 5 seconds
  if (((millis() / 1000) + 5) > TempRHmillis) {
    t = dht.readTemperature();
    rh = dht.readHumidity();
    TempRHmillis = millis() / 1000;
  }

  display.setCursor(0, 56);
  display.print(String("T:") + t);
  display.setCursor(56, 56);
  display.print(String("RH:") + rh);


  display.display();
}

int readDelay() {
  int delayValue = map(analogRead(delayPin), 0, 1023, 5, 7200);
  return delayValue;
}

String showTimer(int delayValue) {
  //  if (delayValue >= 3600){
  //    delayValue = delayValue / 3600;
  //    return delayValue + String(" T");
  //  }
  if (delayValue >= 60) {
    delayValue = delayValue / 60;
    return delayValue + String(" min");
  }
  else {
    return delayValue + String(" sek");
  }

}

void loop() {
  delayValue = readDelay();


  while (delayValue > 0) {
    if (millis() >= time_now + period) {
      time_now += period;
      delayValue -= 1;

      //Skriv til OLED
      displayText(delayValue, 0, sistePumpe);
    }

    //Hvis knapp trykkes, kjør pumpe
    if (startButton == HIGH) {
      while (startButton == HIGH) {
        digitalWrite(startButtonLED, LOW);
        digitalWrite(relPinSolenoid, HIGH);
        digitalWrite(relPinPump, HIGH);
        sistePumpe = millis();
        displayText(delayValue, 1, sistePumpe);
      }
      digitalWrite(startButtonLED, HIGH);
      digitalWrite(relPinSolenoid, LOW);
      digitalWrite(relPinPump, LOW);
      delayValue = readDelay();
    }
  }

  //  IF-check for pump/relay. If potValue is over sensorValue: åpne solenoid og start pumperelay
  if (sensorValue < potValue) {
    //PUMPE PÅ:
    digitalWrite(relPinSolenoid, HIGH);
    delay(20);
    digitalWrite(relPinPump, HIGH);
    sistePumpe = millis();
    displayText(delayValue, 1, sistePumpe);

    delay(pumpetimerValue * 1000);

    digitalWrite(relPinPump, LOW);
    delay(20);
    digitalWrite(relPinSolenoid, LOW);
    time_now = millis() - period;
  }
}
