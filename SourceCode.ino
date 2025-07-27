#include <Arduino.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <AdafruitIO.h>

#define IO_USERNAME  "rashaumon0810" // Lấy mã Key từ server Adafruit.io
#define IO_KEY       "aio_LyRa07tJBPrEdYcbny9BnnAcwdfR"

#define WIFI_SSID "quang" // Tên wifi để ESP 32 kết nối vào và truy cập đến server.
#define WIFI_PASS "quangcute"  // Pass wifi


#include <AdafruitIO_WiFi.h>  // Khai báo thư viện AdafruitIO_WiFi.h để kết nối đến server.
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);  // Gọi hàm kết nối đến server.
AdafruitIO_Feed *HUMIDITY = io.feed("HUMIDITY");
AdafruitIO_Feed *TEMPERATURE = io.feed("TEMPERATURE");
AdafruitIO_Feed *GAS = io.feed("GAS");
AdafruitIO_Feed *tempCOMPARE = io.feed("tempCOMPARE");
AdafruitIO_Feed *gasCOMPARE = io.feed("gasCOMPARE");

#define gas_PIN 35
#define DHTPIN 25
#define DHTTYPE DHT22
#define buzzer 19
#define btnControl 17
#define btnUp 18
#define btnDown 16
#define fan 5
#define pump 33
#define flame_PIN 32
#define btn_emergency 4

float temp, humid;
const int pwmFreq = 5000;  // Tần số PWM
const int pwmResolution = 8; // Độ phân giải (0-255)
int gasValue, rawValue, gasCompare = 50, tempCompare = 50, flameValue, duty = 0;
bool check = true, isBuzzerOn = false, checkEmergency = false;
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE); 

void mqtt(){
  static unsigned long lastSendTime = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastSendTime >= 10000) {
  HUMIDITY->save(humid);
  TEMPERATURE->save(temp);
  GAS->save(gasValue);
  lastSendTime = currentTime;
  }
}
void handleMessage(AdafruitIO_Data *data) {
  lcd.clear();
  tempCompare = data->toInt();
  
}
void handleMessage1(AdafruitIO_Data *data1) {
  lcd.clear();
  gasCompare = data1->toInt();
}
void lcdgas(){
  lcd.setCursor(0, 0);
  lcd.print("Gas: " + String(gasValue));
  lcd.setCursor(0, 1);
  lcd.print("Gas Compare: " + String(gasCompare));
}

void lcddht(){
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print("oC ");
  lcd.setCursor(0, 1);
  lcd.print("Humid:");
  lcd.print(humid);
  lcd.print("%");
  lcd.setCursor(14, 0);
  lcd.print("CP:");
  lcd.setCursor(14, 1);
  lcd.print(tempCompare);
}

void lcdDisplay(){
  if(digitalRead(btnControl) == LOW){
    lcd.clear();
    check = !check;
    Serial.println(check);
    while (digitalRead(btnControl) == LOW);
  }

  if (check == true)
  { 
    lcddht();
  }else
  { 
    lcdgas();
  }
}

void Control(){
  if (digitalRead(btnUp) == LOW)
  { lcd.clear();
    if (check == true)
    { Serial.println(check);
      tempCompare += 1;
      tempCOMPARE->save(tempCompare);
    }
    else if (check == false)
    { Serial.println(check);
      gasCompare += 1;
      gasCOMPARE->save(gasCompare);
    }
    while (digitalRead(btnUp) == LOW);
  }
  else if (digitalRead(btnDown) == LOW)
  { lcd.clear();
    if (check == true)
    {Serial.println(check);
      tempCompare -= 1;
      tempCOMPARE->save(tempCompare);
    }
    else if (check == false)
    {Serial.println(check);
      gasCompare -= 1;
      gasCOMPARE->save(gasCompare);
    }
    while (digitalRead(btnDown) == LOW);
  }
}

void dhtsensor() {
  temp = dht.readTemperature();
  humid = dht.readHumidity();
}

void gasSensor(){
  rawValue = analogRead(gas_PIN); // Đọc giá trị analog từ MQ2
  gasValue = (rawValue*100)/4095; // Chuyển giá trị analog sang giá trị ppm
}

void flameSensor(){
  flameValue = digitalRead(flame_PIN);
  Serial.print(flameValue);
}

void warning() {
  if (temp > tempCompare || gasValue > gasCompare || flameValue == LOW) {
    if (!isBuzzerOn) { // Chỉ bật còi nếu nó chưa bật
      tone(buzzer, 1000);
      isBuzzerOn = true;
    }
    duty = 0;
    ledcWrite(fan, duty);
    digitalWrite(pump, HIGH);
  } else {
    if (!checkEmergency) { // Chỉ tắt nếu không có tình huống khẩn cấp
      noTone(buzzer);
      isBuzzerOn = false;

      duty = 255;
      ledcWrite(fan, duty);
      digitalWrite(pump, LOW);
    }
  }
}

void emergency() {
  if (digitalRead(btn_emergency) == LOW) { 
    Serial.println("Emergency");
    checkEmergency = !checkEmergency;
    while (digitalRead(btn_emergency) == LOW);
  }
  
  if (checkEmergency) {
    duty = 0;
    ledcWrite(fan, duty);
    digitalWrite(pump, HIGH);
    if (!isBuzzerOn) { // Đảm bảo còi chỉ được bật một lần
      tone(buzzer, 1000);
      isBuzzerOn = true;
    }
  } else {
    if (!(temp > tempCompare || gasValue > gasCompare || flameValue == LOW)) {
      noTone(buzzer);
      isBuzzerOn = false;
      
      duty = 255;
      ledcWrite(fan, duty);
      digitalWrite(pump, LOW);
    }
  }
}


void setup() {
  Serial.begin(115200); 
  dht.begin();
  lcd.init();
  lcd.backlight();
  pinMode(buzzer, OUTPUT);
  pinMode(gas_PIN, INPUT);
  pinMode(btnControl, INPUT_PULLUP);
  pinMode(btnUp, INPUT_PULLUP);
  pinMode(btnDown, INPUT_PULLUP);
  pinMode(fan, OUTPUT);
  pinMode(pump, OUTPUT);
  pinMode(flame_PIN, INPUT);
  pinMode(btn_emergency, INPUT_PULLUP);
  ledcAttach(fan, pwmFreq, pwmResolution);

  Serial.println("Connecting to Adafruit IO...");
  io.connect();
  // Chờ kết nối
  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("Đã kết nối với Adafruit IO!");

  tempCOMPARE->get();
  gasCOMPARE->get();
  tempCOMPARE->onMessage(handleMessage);
  gasCOMPARE->onMessage(handleMessage1);
  
}

void loop() {

  io.run();
  warning();
  emergency();
  gasSensor();
  dhtsensor();
  flameSensor();

  // hiển thị lcd
  lcdDisplay();
  // tăng giảm gas và nhiệt độ
  Control();
  mqtt();

  }

