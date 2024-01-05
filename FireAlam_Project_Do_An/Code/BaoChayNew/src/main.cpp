#include <Arduino.h>
#include "LiquidCrystal_I2C.h"
#include <Wire.h>
#include "button.h"
#include "DHT.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Wifi_Manager_custom.h"
#include "SPIFFS.h"
/*GPIO*/
#define MODE_PIN 0
#define UP_PIN 32ULL
#define DOWN_PIN 33ULL
#define DHT_PIN 18
#define MQ2_PIN 34ULL
#define BUZZER 27
#define OUT1 14
#define OUT2 12
#define LED_G 17
#define LED_R 16
#define LED_WF 4
LiquidCrystal_I2C lcd(0x27, 16, 2);
// uint8_t StatusAlarm;

uint8_t is_mode;
uint8_t Status = 2;
uint32_t time_break_alarm;
uint8_t is_break_alarm;
uint64_t count;
// extern int fireAlarmTemperature;
// extern int fireAlarmAir;
int fireAlarmTemperature = 40;
int fireAlarmAir = 70;
uint8_t smoke;
uint8_t t;
uint8_t sta_out1;
uint8_t sta_out2;
/*DHT variable*/
DHT dht(DHT_PIN, DHT22);

void MQ2_init()
{
  pinMode(MQ2_PIN, INPUT);
}

int MQ2_ReadDataPER()
{
  // vì đây là giá trị đo được trong môi trường thử nghiệm chưa được kiểm nghiệm chính xác nên chúng em tạo ngưỡng kích hoạt để kiểm tra hoạt động của thiết bị
  int ADC_data = analogRead(MQ2_PIN);
  return (int)map(ADC_data, 0, 4095, 1, 100);
}

void ShowLCD_Normal(int temperature, int air)
{
  // lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nhiet Do:   ");
  lcd.print(temperature);
  lcd.write(223);
  lcd.print("C  ");
  lcd.setCursor(0, 1);
  lcd.print("NONG DO KHOI:");
  lcd.print(air);
  lcd.print("  ");
}
void ShowLCD_SetTemperature(int temperature)
{
  // lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("  SET NHIET DO  ");
  lcd.setCursor(0, 1);
  lcd.print("Nhiet Do:   ");
  lcd.print(temperature);
  lcd.write(223);
  lcd.print("C");
}
void ShowLCD_SetAir(int air)
{
  // lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("SET NONG DO KHOI   ");
  lcd.setCursor(0, 1);
  lcd.print("NONG DO KHOI:");
  lcd.print(air);
  lcd.print("  ");
  // lcd.write();
  // lcd.print("%");
}
void ShowLCD_TemperatureHigh()
{

  lcd.setCursor(0, 0);
  lcd.print(" CANH BAO CHAY  ");
  lcd.setCursor(0, 1);
  lcd.print(" CANH BAO CHAY  ");
}

Button_Typedef_t ButtonMode;
Button_Typedef_t ButtonUp;
Button_Typedef_t ButtonDown;
void btn_pressing_callback(Button_Typedef_t *ButtonX)
{
  if (Status == 2)
  {

    if (ButtonX->pin == MODE_PIN)
    {
      if (is_mode == 0)
      {
        // Set Nhiet Do
        is_mode = 1;
        // lcd.backlight();
      }
      else if (is_mode == 1)
      {
        // Set nong do khi
        //  lcd.noBacklight();
        is_mode = 2;
      }
      else if (is_mode == 2)
      {
        // Tro de ve che do bth
        is_mode = 0;
      }
    }
    if (ButtonX->pin == UP_PIN)
    {
      Serial.print("UP Press");
      if (is_mode == 1)
      {
        fireAlarmTemperature++;
      }
      else if (is_mode == 2)
      {
        fireAlarmAir++;
      }
    }
    if (ButtonX->pin == DOWN_PIN)
    {
      Serial.print("DOWN Press");
      if (is_mode == 1)
      {
        fireAlarmTemperature--;
      }
      else if (is_mode == 2)
      {
        fireAlarmAir--;
      }
    }
  }
  else
  {
    Serial.printf("is_break_alarm");
    Status = 2;
    sta_out1 = 1;
    sta_out2 = 1;
    digitalWrite(BUZZER, 0);
    digitalWrite(OUT1, sta_out1);
    digitalWrite(OUT2, sta_out2);
    digitalWrite(LED_G, 0);
    digitalWrite(LED_R, 1);
    time_break_alarm = millis();
  }
}
uint8_t loadData[6];
/*
  byte 1: Status
  byte 2: nhiet do
  byte 3: nong do
  byte 4: OUT1
  byte 5: OUT2
  byte 6: RSSI of wifi
*/
void Process_data()
{
  loadData[2] = Status;
  loadData[0] = t;
  loadData[1] = smoke;

  loadData[4] = (sta_out1 == 1) ? '0' : '1';
  loadData[5] = (sta_out2 == 1) ? '0' : '1';
  loadData[3] = wifi_getRSSI();
  loadData[6] = fireAlarmAir;
  loadData[7] = fireAlarmTemperature;
}
void setup()
{
  // wdt_enable(WDTO_1S);
  Serial.begin(115200);
  button_init(&ButtonMode, MODE_PIN);
  button_init(&ButtonUp, UP_PIN);
  button_init(&ButtonDown, DOWN_PIN);
  lcd.init(); // sua thu vien LiquidCrystal_I2C
  // Wire.begin(19, 21);
  lcd.backlight();
  dht.begin();
  MQ2_init();
  pinMode(BUZZER, OUTPUT);
  pinMode(OUT1, OUTPUT);
  pinMode(OUT2, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_WF, OUTPUT);
  digitalWrite(OUT1, 1);
  digitalWrite(OUT2, 1);

  Serial.println("Xin Chao");
  xTaskCreate(wifimanager, "WIFI", 4096, NULL, 3, NULL);
}

void loop()
{
  button_handle(&ButtonMode);
  button_handle(&ButtonUp);
  button_handle(&ButtonDown);
  t = dht.readTemperature();
  smoke = MQ2_ReadDataPER();
  Process_data();
  if (t > fireAlarmTemperature || smoke > fireAlarmAir)
  {

    Status = 1;
    is_break_alarm = 1;
  }

  if (Status == 1)
  {
    sta_out1 = 0;
    sta_out2 = 0;
    ShowLCD_TemperatureHigh();
    digitalWrite(BUZZER, 1);
    digitalWrite(OUT1, sta_out1);
    digitalWrite(OUT2, sta_out2);
    digitalWrite(LED_R, 0);
    digitalWrite(LED_G, 1);
  }
  if (Status == 2)
  {

    if (is_mode == 0)
    {
      ShowLCD_Normal(t, smoke);
    }
    if (is_mode == 1)
    {
      ShowLCD_SetTemperature(fireAlarmTemperature);
      Serial.printf("SET NGUONG NHIET DO: %d", fireAlarmTemperature);
    }
    if (is_mode == 2)
    {
      ShowLCD_SetAir(fireAlarmAir);
      Serial.printf("SET NGUONG NONG DO KHOI: %d", fireAlarmAir);
    }
  }
  if (count % 10 == 0) // 1s
  {
    Serial.printf("Nhiet Do: %d \nNONG DO KHOI: %d\n", t, smoke);
    Process_data();
    MQTT_Send(loadData);
    if (Status == 1)
    {
      Serial.printf("Da xay ra su co chay\n");
    }
  }
  if (WiFi.isConnected())
  {
    digitalWrite(LED_WF, 0);
  }
  else
  {
    digitalWrite(LED_WF, 1);
  }

  count++;

  vTaskDelay(10 / portTICK_PERIOD_MS);
}
