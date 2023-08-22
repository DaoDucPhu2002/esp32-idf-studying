#include "HWConfig.h"
#include "TEE_EC200.h"

const char mqttServer[] = "test.mosquitto.org";
const int mqttPort = 1883;
const char mqttUser[] = "lctech_rmq";
const char mqttPassword[] = "lctechrmq123@";

char topicSever[] = "mmm/GSafe_v3/data"; // mmm/GSafe_v3_data_4g/data
char txt_subscribe[] = "mmm/123456789012/cmd";

char topicSever4G[] = "mmm/GSafe_v3_4g/data";
// char txt_subscribe[] = "mmm/123456789012/cmd"

#define OTA_CURRENT_VERSION "23.07.021"
// const char* ota_uri = "http://api.lctech.vn/firmware/esp32/OTA";
const char *ota_uri = "http://test.api.lctech.vn/firmware/esp32/OTA";
// const char* ota_uri = "http://test.api.lctech.vn:8080/firmware/esp32/OTA";
// char* ESP_ID = "123456789012";
char ESP_ID[] = "123456789012";
char IMEI[] = "000000000000001";

Ticker ticker;

WiFiClient espClient;
PubSubClient mqtt(espClient);

uint8_t checkBoard = 0;

uint32_t lastcheckOTA;

int error_mqtt;
unsigned int lenth;
uint32_t lastTimeWifi;
uint8_t autoWifi = 0, autoSsid;
#define timeloopOTA (1000 * 60 * 5)
// bool in_setupWifiManager = false;
bool net4Gconnected = false, WifiConnected = false, rfOk = false;

extern EC200 gsm;

static const char GSM_OK[] PROGMEM = "OK" GSM_NL;
static const char GSM_ERROR[] PROGMEM = "ERROR" GSM_NL;
String MODEM_OK = "OK";
String MODEM_ERROR = "ERROR";

static const char OK_RESPOND[] PROGMEM = "OK";
static const char PRODUCT_ID[] PROGMEM = "ATI\r\n";
static const char MQTT_CONFG[] PROGMEM = "AT+QMTCFG=recv/mode,0,0,1";

#define PWR_ON_4G 1  // ON = 1, Start On = 0;
#define PWR_OFF_4G 4 // OFF = 4, Start Off = 3;

hw_timer_t *timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

uint32_t timeDelay4G, timeOut4G, lastTimeOut4G;
uint8_t autoInit4G, stepAuto4G, keyTest;

uint32_t Vbat, holdCharg;
uint8_t cntAdc, numBytesSave, deviceStatus, lenHexString;
uint16_t numTask1, cntDisconnect, numTaskS, numTimeSend4G, reInit4G;
uint32_t timeLoopAlarm, timeLoopStatus, timeLoopBAT, timeAutoBat, numSendData;
uint32_t lastPress = 0, lastPressReset;
bool keyBell;
uint16_t holdKey, adcACOK;

// memory EEPROM
// 0-99 wifi
// 100 - 200, 100 check value of setting
//

#define EEPROM_SETTING 100
// #define TIME_ALARM_ON    3000
// #define TIME_ALARM_OFF   2000

uint32_t timeAlarmOn, timeAlarmOff, timeActiveAlarm, timeAlarm, timeCheckAlarm;
uint32_t test;
// #define TIME_ALARM      (timeAlarmOn + timeAlarmOff)

#define TIME_CHECK_STATUS 100
#define TIME_CHECK_ALRAM 100
#define TIME_LOOP_WIFI 30000
// #define TIME_ACTIVE_ALARM   3000
#define TIME_HOLD_SENSOR (timeActiveAlarm / TIME_CHECK_ALRAM)
#define TIME_WIFI (TIME_LOOP_WIFI / TIME_CHECK_ALRAM)

char ssid[eepromTextVariableSize] = "GSafe check";
char pass[eepromTextVariableSize] = "@12345678";

WebServer serverAP(80); // the Access Point Server
IPAddress local_ip(192, 168, 8, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

boolean accessPointMode = false; // is true every time the board is started as Access Point
boolean debug = true;
unsigned long lastUpdatedTime = 0;
unsigned long lastTime = 0;

int pushDownCounter = 0;
int lastConnectedStatus = 0;

uint8_t __attribute__((aligned(4))) dataload[numBytes];

uint8_t HexString[sizeof(dataload) * 2]; // 2 nibbles/byte + null terminator

typedef union type8to32
{
  uint8_t uint8[16];
  uint16_t uint16[8];
  uint32_t uint32[4];
};
type8to32 eepRead;

struct gsafe_msg_data
{                        // total = 35 byte
  uint8_t alarmStatus;   //  1
  uint16_t batVoltage;   //  2
  uint8_t batPercent;    //  1
  uint8_t pwStatus;      //  1
  uint8_t Status;        //  1
  uint8_t LEDStatus;     //  1
  int8_t rssi4G;         //  1
  int8_t rssiWF;         //  1
  uint16_t sensorEOL;    //  2
  uint16_t sensorStatus; //  2
  uint8_t lineStatus[numLine];
  uint16_t lineNotUse;
  uint8_t holdStatus[numLine];
  uint8_t keySensor[numLine], timeLow[numLine], timeHigh[numLine], timeHold[numLine];
  bool enableBell; //  1
  bool powerBAT;
  uint8_t startCheckAlarm;
  uint16_t timeCounter;
  uint16_t VolLowPwBAT;
  int8_t lowRssiWiFi, keyTest;
  uint8_t relayStatus;
  uint32_t zoneArlam;
  uint32_t zoneWarning;
};
struct gsafe_msg_data gdata; // staMode

// typedef union type8to32
// {
//     uint8_t uint8[16];
//     uint16_t uint16[8];
//     uint32_t uint32[4];
// }__attribute__((packed));
type8to32 eepData;

void initStart(char rewrite)
{
  byte x;
  LAMP_OFF;
  BELL_ON;
  OUT1_OFF;
  OUT2_OFF;
  VBAT_OFF;
  LEDR_OFF;
  LEDG_OFF;
  reInit4G = 0;
  adcACOK = 4000;
  gdata.enableBell = 1;
  gdata.powerBAT = 0;
  gdata.Status = NORMAL;
  gdata.lowRssiWiFi = -80; // 75
  numBytesSave = 12;
  gdata.zoneArlam = 0;
  gdata.zoneWarning = 0;
  // rewrite = 1;
  if ((EEPROM.read(EEPROM_SETTING - 1) == 0xFF) | (rewrite))
  {
    timeAlarmOn = 3000;
    timeAlarmOff = 2000;
    timeAlarm = timeAlarm + timeAlarmOff;
    timeActiveAlarm = 3000;
    gdata.timeCounter = 8640; // 6day * 24h * 60min;
    gdata.VolLowPwBAT = 740;
    gdata.lineNotUse = 0;
    GSafe_Setting_Save();
    debugSerial("Save to EEPROM");
    delay(10);
  }
  else
  {
    GSafe_Setting_Read();
    debugSerial("Read from EEPROM");
  }
  x = getDebugFromEeprom();
  if (x > 10)
  {
    saveDebugToEeprom(0);
    debug = true;
    delay(10);
  }
  else if (x < 3)
  {
    saveDebugToEeprom(x + 1);
    debug = true;
    debugSerial("Debug mode");
  }
  else
  {
    debug = false;
  }
  if (DEBUG)
    debug = true;
  // sendTimePCln("time Alarm On:",timeAlarmOn);
  // sendTimePCln("time Alarm Off:",timeAlarmOff);
  // sendTimePCln("time Active Alarm:",timeActiveAlarm);
  // digitalWrite(LED_D2, LOW);
  //  timeAutoBat = gdata.timeCounter*60*1000;
  BELL_OFF;
  stepAuto4G = 0;
}
void InitIO()
{
  // Init Output PIN
  pinMode(LED_D2, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(SA0, OUTPUT);
  pinMode(SA1, OUTPUT);
  pinMode(SA2, OUTPUT);
  pinMode(SA3, OUTPUT);
  pinMode(PWR_KEY, OUTPUT);
  pinMode(BOOT_EN, OUTPUT);
  pinMode(CHARGE, OUTPUT);
  pinMode(LAMP, OUTPUT);
  pinMode(BELL, OUTPUT);
  pinMode(RL_OUT1, OUTPUT);
  pinMode(RL_OUT2, OUTPUT);
  pinMode(BEEP, OUTPUT);
  // Init Input PIN
  pinMode(EOLINE, INPUT);
  pinMode(INSEN, INPUT);
  pinMode(RF_REMOTE, INPUT_PULLDOWN);
  esp_task_wdt_init(20, true); // enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);      // add current thread to WDT watch
  VBAT_ON;
  // BEEP_ON;
  //  Init ADC
  // set the resolution to 12 bits (0-4096)
  // EEPROM.begin(512);
  analogReadResolution(12);
  // net4Gconnected = false;
  initStart(0);
  // for(int i=0; i<numLine;i++)gdata.lineUse[i]=1;
}
void tick()
{
  int state = digitalRead(LED_D2); // get the current state of GPIO1 pin
  digitalWrite(LED_D2, !state);    // set pin to the opposite state
}
char h2c(char c)
{
  return "0123456789ABCDEF"[0x0F & (unsigned char)c];
}
uint8_t updateHexString(uint8_t value, uint8_t index)
{
  HexString[index++] = h2c(value >> 4);
  HexString[index++] = h2c(value & 0x0F);
  return (index);
}
int convert_u8_hex(uint8_t *raw_data, uint8_t *conv_data, uint16_t data_length)
{
  // char size_of_pack[data_length];
  uint16_t cnt_cvt = 0;
  char tempChars[3]; // 2 nibbles/byte + null terminator

  // check condition
  //  if(data_length > sizeof(raw_data))
  //    return 1;
  //  if(sizeof(conv_data) < (2* data_length))
  //    return 2;
  // convert char array to hex string
  memset(conv_data, 0, sizeof(conv_data));
  for (cnt_cvt = 0; cnt_cvt < data_length; cnt_cvt++)
  {
    sprintf(tempChars, "%02x", raw_data[cnt_cvt]);
    strncat((char *)conv_data, (char *)tempChars, 2);
  }
  return 0;
}
char *string2char(String ipString)
{                                                 // make it to return pointer not a single char
  char *opChar = new char[ipString.length() + 1]; // local array should not be returned as it will be destroyed outside of the scope of this function. So create it with new operator.
  memset(opChar, 0, ipString.length() + 1);

  for (int i = 0; i < ipString.length(); i++)
    opChar[i] = ipString.charAt(i);
  return opChar; // Add this return statement.
}
void debugSerial(String data)
{
  if (debug)
    dbSerial.println(data);
}
void debugSerial(char *data)
{
  if (debug)
    dbSerial.println(data);
}
void sendHexPC(uint8_t hex)
{
  if (hex < 16)
    dbSerial.print(('0'));
  dbSerial.print(hex, HEX);
  dbSerial.print(' ');
}
void sendToPCTest()
{
  uint8_t numRssi;

  if (gdata.rssiWF != 0)
    numRssi = 256 - gdata.rssiWF;
  else
    numRssi = 0;

  dbSerial.print("ID;"); // 0-2
  for (int line = 0; line < 12; line++)
    dbSerial.print(ESP_ID[line]); // 3-14
  dbSerial.print("-");
  for (int line = 0; line < 15; line++)
    dbSerial.print(IMEI[line]); // 16-30
  // dbSerial.print(" Status:"); dbSerial.print(gdata.Status);
  // dbSerial.print(" PS:");dbSerial.print(gdata.pwStatus);
  // if(gdata.pwStatus&PW_ACIN) dbSerial.print(" AC_OK"); else dbSerial.print(" AC_OF");
  // if(gdata.pwStatus&PW_CHARG) dbSerial.print(" CH_ON"); else dbSerial.print(" CH_OFF");
  // if(gdata.pwStatus&PW_BAT) dbSerial.print(" PW_BAT"); else dbSerial.print(" PW_AC");
  // if(gdata.pwStatus&BAT_OK) dbSerial.print(" BAT_OK:"); else dbSerial.print(" BAT_ER:");
  dbSerial.print("-");
  dbSerial.print(gdata.batVoltage / 100);
  dbSerial.print(gdata.batVoltage / 10 % 10);
  dbSerial.print(gdata.batVoltage % 10); // 34
  // dbSerial.print(gdata.batPercent);dbSerial.print('%');
  dbSerial.print("-");
  dbSerial.print(WifiConnected);
  dbSerial.print(numRssi / 10);
  dbSerial.print(numRssi % 10); //  38
  // dbSerial.print(key_adc/1000);dbSerial.print(key_adc/100%10);dbSerial.print(key_adc/10%10);dbSerial.print(key_adc%10);
  numRssi = gdata.rssi4G;
  if (numRssi > 99)
    numRssi = 0;
  dbSerial.print("-");
  dbSerial.print(net4Gconnected);
  dbSerial.print(numRssi / 10);
  dbSerial.print(numRssi % 10); //  42
  // sendTimePC(" Status:",gdata.Status);
  dbSerial.print(rfOk + keyTest * 2); // 43
  for (int line = 0; line < numLine; line++)
  {
    // dbSerial.print(line+1); dbSerial.print(": ");
    dbSerial.print(gdata.lineStatus[line]); // 44-59
  }                                         // 59
  // // dbSerial.println();
  // dbSerial.print("-");
  // dbSerial.write(gdata.zoneArlam/256); dbSerial.write(gdata.zoneArlam%256);
  // // dbSerial.print(gdata.zoneArlam);
  // dbSerial.print("-");
  // dbSerial.write(gdata.zoneWarning/256);  dbSerial.write(gdata.zoneWarning%256);
  // dbSerial.print(gdata.zoneWarning);
  uint16_t adc = analogRead(CH_AC); // VBAT_FB  CH_AC
  dbSerial.print(" ");
  dbSerial.print(adc);
  adc = analogRead(KEY_IN); // VBAT_FB  CH_AC   KEY_IN
  dbSerial.print(" ");
  dbSerial.print(adc);
  if (gdata.pwStatus & PW_ACIN)
    dbSerial.print(" AC_OK");
  else
    dbSerial.print(" AC_OF");
  if (gdata.pwStatus & PW_CHARG)
    dbSerial.print(" CH_ON");
  else
    dbSerial.print(" CH_OFF");
  if (gdata.pwStatus & PW_BAT)
    dbSerial.print(" PW_BAT");
  else
    dbSerial.print(" PW_AC");
  if (gdata.pwStatus & BAT_OK)
    dbSerial.print(" BAT_OK ");
  else
    dbSerial.print(" BAT_ER ");
  // dbSerial.print("W "); dbSerial.print(gdata.zoneWarning);
  // dbSerial.print("A "); dbSerial.print(gdata.zoneArlam);
  dbSerial.print(millis() / 1000);
  dbSerial.println();
  // debugSerial(HexString);
}
void sendToPC()
{
#ifdef DEBUG
  dbSerial.print("ID1:");
  for (int line = 0; line < 12; line++)
    dbSerial.print(ESP_ID[line]);
  dbSerial.print("-");
  for (int line = 0; line < 15; line++)
    dbSerial.print(IMEI[line]);
  dbSerial.print("-S");
  dbSerial.print(gdata.Status);
  dbSerial.print("-P");
  dbSerial.print(gdata.pwStatus);
  // if(gdata.pwStatus&PW_ACIN) dbSerial.print(" AC_OK"); else dbSerial.print(" AC_OF");
  // if(gdata.pwStatus&PW_CHARG) dbSerial.print(" CH_ON"); else dbSerial.print(" CH_OFF");
  // if(gdata.pwStatus&PW_BAT) dbSerial.print(" PW_BAT"); else dbSerial.print(" PW_AC");
  // if(gdata.pwStatus&BAT_OK) dbSerial.print(" BAT_OK:"); else dbSerial.print(" BAT_ER:");
  dbSerial.print("-B");
  dbSerial.print(gdata.batVoltage / 100);
  dbSerial.print(gdata.batVoltage / 10 % 10);
  dbSerial.print(gdata.batVoltage % 10);
  // dbSerial.print(gdata.batPercent);dbSerial.print('%');
  dbSerial.print("-W");
  dbSerial.print(WifiConnected);
  dbSerial.print(',');
  dbSerial.print(String(gdata.rssiWF));
  dbSerial.print("-G:");
  dbSerial.print(net4Gconnected);
  dbSerial.print(',');
  dbSerial.print(gdata.rssi4G);
  // sendTimePC(" Status:",gdata.Status);
  dbSerial.print("-L");
  for (int line = 0; line < numLine; line++)
  {
    // dbSerial.print(line+1); dbSerial.print(": ");
    dbSerial.print(gdata.lineStatus[line]);
    // dbSerial.print(" ");
  }
  // dbSerial.println();
  dbSerial.print("-A");
  dbSerial.print(gdata.zoneArlam);
  dbSerial.print("-W");
  dbSerial.print(gdata.zoneWarning);
  dbSerial.println();
  // debugSerial(HexString);
#endif // DEBUG
}
void sendTimePC(char *data, uint32_t time)
{
  if (debug)
  {
    dbSerial.print(data);
    dbSerial.print(time);
  }
}
void sendTimePCln(char *data, uint32_t time)
{
  if (debug)
  {
    dbSerial.print(data);
    dbSerial.println(time);
  }
}
void beepOut(uint16_t dtime)
{
  if (BUZZER_EN)
  {
    BEEP_ON;
    delay(dtime);
    BEEP_OFF;
  }
}
/********************************** BEGIN Wifi Code *********************************************/
bool longPress()
{
  if (millis() - lastPress > 3000 && ((digitalRead(PIN_BUTTON) == 0) | (key_press() == KEY_MODE)))
  {
    beepOut(500);
    return true;
  }
  else if (millis() - lastPress > 3000 && ((key_press() == KEY_CHECK)))
  {
    ESP.restart();
    lastPress = 0;
    return false;
  }
  else if (millis() - lastPress > 10000 && ((key_press() == KEY_BELL)))
  {
    debug = true;
    saveDebugToEeprom(0);
    if (debug)
      dbSerial.print("Bell 10s");
    beepOut(500);
    lastPress = millis();
    // ESP.restart();
    return false;
  }
  else if ((digitalRead(PIN_BUTTON) == 1) & (key_press() != KEY_MODE) & (key_press() != KEY_CHECK) & (key_press() != KEY_BELL))
    lastPress = millis();

  return false;
}
void initAsAccessPoint()
{
  WiFi.softAP("GSafe Setup"); // or WiFi.softAP("ESP_Network","Acces Point Password");
  if (debug)
  {
    Serial.println("AccesPoint IP: " + WiFi.softAPIP().toString());
    Serial.println("Mode= Access Point");
  }
  // WiFi.softAPConfig(local_ip, gateway, subnet); // enable this line to change the default Access Point IP address
  delay(100);
}
void checkAccessMode()
{
  if (accessPointMode)
  { // start as Access Point
    ticker.attach(0.1, tick);
    initAsAccessPoint();
    serverAP.on("/", handle_OnConnect);
    serverAP.onNotFound(handle_NotFound);
    serverAP.begin();
    saveStatusToEeprom(0); // enable the Client mode for the the next board starting
  }
  else
  { // start as client
    accessPointMode = false;
    if (debug)
      Serial.println("Mode= Client");
    WiFi.begin(ssid, pass);
    ticker.detach();
    LED_ON();
    WiFi.mode(WIFI_STA);
    // Enter your client setup code here
  }
}
void checkStartWifi()
{
  //--- Check the first EEPROM byte. If this byte is "2" the board will start as Access Point
  int st = getStatusFromEeprom();
  if (st == 2)
    accessPointMode = true;
  else if (st != 0)
    saveSettingsToEEPPROM(ssid, pass); // run the void saveSettingsToEEPPROM on the first running or every time you want to save the default settings to eeprom
  Serial.println("\n\naccessPointMode=" + String(accessPointMode));

  readSettingsFromEEPROM(ssid, pass); // read the SSID and Passsword from the EEPROM
  if (debug)
    Serial.println(ssid);
  if (debug)
    Serial.println(pass);
  checkAccessMode();
}
void checkWiFiConnection()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    if (lastConnectedStatus == 1)
      Serial.println("WiFi disconnected\n");
    lastConnectedStatus = 0;
    if (debug)
      Serial.print(".");
    delay(500);
  }
  else
  {
    if (lastConnectedStatus == 0)
    {
      if (debug)
      {
        Serial.println("Mode= Client");
        Serial.print("\nWiFi connectd to :");
        Serial.println(ssid);
        Serial.print("\n\nIP address: ");
        Serial.println(WiFi.localIP());
      }
    }
    lastConnectedStatus = 1;
  }
}
// void playAccessPointLed() {
//   if (millis() - lastTime > 300) {
//     lastTime = millis();
//     digitalWrite(accessPointLed, !digitalRead(accessPointLed));
//   }
// }
// void checkAccessWifi(){
//   if (accessPointMode) {
//   serverAP.handleClient();
//   // playAccessPointLed(); // blink the LED every time the board works as Access Point
//   }
//   else {
//   checkWiFiConnection();
//   }
// }
// Wifi manager Setup
void setupWifiManager()
{
  if (longPress())
  {
    WiFi.mode(WIFI_STA);
    accessPointMode = true;
    checkAccessMode();
  }
  if (accessPointMode)
  {
    serverAP.handleClient();
  }

  else if (WiFi.status() != WL_CONNECTED) // && in_setupWifiManager == false)
  {
    WiFi_Connect(); /*Reconnect wifi */
  }
  else if (autoWifi > 0)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      ticker.detach();
      if (debug)
      {
        dbSerial.println("WiFi connected");
        dbSerial.println("IP address: ");
        dbSerial.println(WiFi.localIP());
      }
      // ticker.detach();
      digitalWrite(LED_D2, HIGH);
      autoWifi = 0;
    }
  }
  else if (WiFi.status() == WL_CONNECTED)
  {
    if (gdata.rssiWF > gdata.lowRssiWiFi)
      mqttConnect();
  }
}
void macRead()
{
  uint8_t i = 0;
  char macAdd[20];
  String txt = WiFi.macAddress(); // 44:17:93:EF:D1:70

  if (debug)
    dbSerial.println(txt);
  txt.toCharArray(macAdd, 18);
  memset(ESP_ID, 0, sizeof(ESP_ID));
  // for(i=0;i<12;i++) ESP_ID[i]=0;
  for (i = 0; i < 6; i++)
  {
    ESP_ID[i * 2] = macAdd[i * 3];         // h2c(macAdd[i]>>4);
    ESP_ID[i * 2 + 1] = macAdd[i * 3 + 1]; // h2c(macAdd[i]);
  }
  for (i = 0; i < 12; i++)
    txt_subscribe[i + 4] = ESP_ID[i];
  dbSerial.print("MAC: ");
  dbSerial.println(ESP_ID);
  dbSerial.print("OTA Version: ");
  dbSerial.println(OTA_CURRENT_VERSION);
}
void delayWifi(void)
{
  if ((autoWifi == 1) & ((millis() - lastTimeWifi) >= 500))
  {
    autoWifi = 2;
    lastTimeWifi = millis();
  }
  else if ((autoWifi > 1) & ((millis() - lastTimeWifi) >= 1000))
  {
    autoWifi++;
    if (autoWifi > 60)
      autoWifi = 0;
    lastTimeWifi = millis();
  }
}
void WiFi_Connect()
{
  if (autoWifi == 0)
  {
    WiFi.mode(WIFI_OFF); // Disconnect wifi
    autoWifi = 1;
    ticker.attach(0.5, tick);
  }
  else if (autoWifi == 2)
  {
    // attempt to connect to Wifi network
    WiFi.mode(WIFI_STA);
    debugSerial("Connecting to WiFi..");
    //    dbSerial.println(ssid);
    WiFi.begin(); // ssid,password);
    debugSerial(ssid);
    autoWifi = 3;
    //   dbSerial.println("Connecting to WiFi..");
    //   dbSerial.println(WiFi.SSID());
  }
  delayWifi();
}
void mqttConnect()
{
  if (mqtt.connected())
    return;
  mqtt.setServer(mqttServer, mqttPort);
  mqtt.setCallback(callback);
  delay(10);
  if (!mqtt.connected())
  {
    debugSerial("Connecting to MQTT...");

    if (mqtt.connect(ESP_ID, mqttUser, mqttPassword))
    {
      debugSerial("connected");
    }
    else
    {
      if (debug)
      {
        dbSerial.print("failed with state ");
        dbSerial.println(mqtt.state());
      }
      error_mqtt++;
      if (error_mqtt > 10)
      {
        error_mqtt = 0;
        autoWifi = 0;
        debugSerial("Restart Wifi");
        mqtt.disconnect();
        WiFi_Connect();
        WifiConnected = false;
        // break;
        //  ESP.restart();
      }
    }
  }
  if (mqtt.connected())
  {
    // beepOut(200);
    if (debug)
    {
      dbSerial.print("subscribe to:");
      dbSerial.println(txt_subscribe);
    }
    mqtt.subscribe(txt_subscribe);
    // dbSerial.println(ESP_ID);
    error_mqtt = 0;
  }
}
void ota_update()
{
  if (millis() - lastcheckOTA > timeloopOTA)
  {
    lastcheckOTA = millis();
    if ((WiFi.status() == WL_CONNECTED))
    {
      if (debug)
        dbSerial.println("start OTA update");
      WiFiClient client;
      // t_httpUpdate_return ret = httpUpdate.update(client, ota_uri);
      t_httpUpdate_return ret = httpUpdate.update(client, ota_uri, OTA_CURRENT_VERSION);
      switch (ret)
      {
      case HTTP_UPDATE_FAILED:
        if (debug)
          dbSerial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        if (debug)
          dbSerial.println("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        if (debug)
          dbSerial.println("HTTP_UPDATE_OK");
        break;
      }
    }
  }
}
void smartUpdateOTA()
{ // Smart config, update OTA, Auto connect Wifi
  setupWifiManager();
  ota_upcddate();
  mqtt.loop();
}
void sendToSever()
{ // this section send via Wifi
  if (mqtt.connected())
  {
    if (mqtt.publish((char *)topicSever, (uint8_t *)dataload, sizeof(dataload)))
    {
#ifdef DEBUG
      // dbSerial.println("Data Send Ok ");
      // dbSerial.println(millis()/1000); //deviceStatus
#endif // DEBUG
      WifiConnected = true;
      numSendData++;
    }
    else
    {
      //  debugSerial("Data Send failure! ");
      WifiConnected = false;
    }
  }
}
void wifi_mqtt_process()
{
  // if(gdata.rssiWF==0){
  //   cntDisconnect++;
  //   if(cntDisconnect>1000) ESP.restart();
  // }
  if ((gdata.rssiWF > gdata.lowRssiWiFi) & (gdata.rssiWF != 0))
  {
    if (mqtt.connected())
    {
      sendToSever();
      if (debug)
      {
        dbSerial.println("Wifi Send: ");
        for (int i = 0; i < numBytes; i++)
        {
          sendHexPC(dataload[i]);
        }
        dbSerial.println();
      }
    }
    cntDisconnect = 0;
  }
  else
  {
    debugSerial("Wifi poor, not send");
    WifiConnected = false;
  }
}
/*************************************** EC200 - 4G BEGIN ***************************/
void start_time_out_4g()
{
  lastTimeOut4G = millis();
}
void powerKey4G()
{
  digitalWrite(PWR_KEY, HIGH);
  delay(1000);
  digitalWrite(PWR_KEY, LOW);
}
bool read_gsm()
{
  String req = "";
  if (Serial4G.available())
  {
    if (stepAuto4G < 2)
    {
      timeOut4G = millis();
      req = Serial4G.readStringUntil('\n');
      // debugln(req);
      if (req.indexOf(F("RDY")) != -1)
      {
        return (true);
      }
      if (req.indexOf(F("POWERED DOWN")) != -1)
      {
        start_time_out_4g();
        debugSerial(F("Power OFF"));
        powerKey4G();
      }
      if (timeOut4G - lastTimeOut4G > 8000)
      {
        powerKey4G();
        start_time_out_4g();
      }
    }
  }
}
void init_4G()
{
  String txt;
  // this use for 4G modem
  gsm.begin(&Serial2, 115200);
  // assign pin control power on, different power enable
  gsm.SetPowerKeyPin(PWR_KEY);
  // command ping power on
  // gdata.rssi4G = gsm.SignalQuality();
  // if((gdata.rssi4G==0)|(gdata.rssi4G>=99))
  //  txt = gsm.GetIMEI();
  //  if(txt=="")
  gsm.PowerOn();
  // wait 4G connection ready
  //  timeDelay4G = millis();
  //  delay4Ginit();
  //  delay(8000);
  //  //gsm.WaitReady();
  //  txt = gsm.GetIMEI();
  txt = gsm.GetIMEI();
  for (int i = 0; i < 15; i++)
    IMEI[i] = txt[i];
  // debugSerial(txt);
  gdata.rssi4G = gsm.SignalQuality();
#ifdef DEBUG
  // dbSerial.println(F("IMEI: "));
  // dbSerial.println(txt);
  // dbSerial.println(F("GetOperator --> "));
  // dbSerial.println(gsm.GetOperator());
  // dbSerial.println(F("SignalQuality --> "));
  // gdata.rssi4G = gsm.SignalQuality();
// gdata.rssi4G = gsm.SignalQuality();
// dbSerial.println(gdata.rssi4G);
// //delay(500);
#endif
}
void autoStatus4G()
{
  // autoInit4G,stepAuto4G,timeDelay4G,timeOut4G
  String txt = "";
  if (stepAuto4G == 0)
  { // check SIM power ON
    /// gsm.begin(&Serial2,115200);
    /// gsm.SetPowerKeyPin(PWR_KEY);
    // txt = gsm.GetIMEI();  // get IMEI => Power ON
    // if(txt=="") //gsm.PowerOn();
    {
      powerKey4G();
      stepAuto4G = 1;
      delay(2000);
      start_time_out_4g();
    }
    // else {
    //   txt = gsm.GetIMEI();
    //   for(int i=0;i<15;i++)IMEI[i]=txt[i];
    //   stepAuto4G = 2;
    // }
  }
  else if (stepAuto4G == 1)
  { // Read RDY
    if (read_gsm())
    {
      stepAuto4G = 2; // RDY
      txt = gsm.GetIMEI();
      for (int i = 0; i < 15; i++)
        IMEI[i] = txt[i];
    }
  }
}
void readRSSI4G()
{
  gdata.rssi4G = gsm.SignalQuality();
  if (gdata.rssi4G == 0)
    gdata.rssi4G = gsm.SignalQuality();
}
void sendToSever4G()
{
  boolean sendWifi = false;
  readRSSI4G();
  // if(gdata.rssi4G<25)return;

  if ((gdata.Status == ALARM) & (numTimeSend4G % 10 == 0))
  {
    wifi_mqtt_process();
  }
  else if (numTimeSend4G % 30 == 0)
  {
    wifi_mqtt_process();
  }

  if ((gdata.rssi4G == 0) || (gdata.rssi4G >= 100) || (reInit4G > 100))
  {
    if (debug)
      Serial.println("PW Down SIM");
    gsm.PowerOff();
    esp_task_wdt_reset();
    wifi_mqtt_process();
    gsm.PowerOn();
    wifi_mqtt_process();
    reInit4G = 0;
    net4Gconnected = false;
    return;
  }
  else if (gdata.rssi4G == 99)
  {
    reInit4G++;
    return;
  }

  /* if wifi mqtt connected  send 300s, else 30s send again*/
  // numTimeSend4G++;
  if ((debug) & (numTimeSend4G % 10 == 0) & (millis() < 30000))
  {
    numTimeSend4G = 0;
  }
  // if((gdata.Status==ALARM)&(numTimeSend4G%10==0)){
  //     numTimeSend4G=0;
  // }
  else if (mqtt.connected()) //(WiFi.status() == WL_CONNECTED)&(mqtt.connected()))
  {
    if ((millis() < 300000) & (numTimeSend4G >= 30))
    {
      numTimeSend4G = 0;
    }
    else if (numTimeSend4G >= 300)
    {
      numTimeSend4G = 0;
    }
    else
      return;
  }
  else if ((gdata.Status == ALARM) & (numTimeSend4G >= 10))
  {
    numTimeSend4G = 0;
  }
  else if (numTimeSend4G >= 30)
  {
    numTimeSend4G = 0;
  }
  else
  {
    return;
  }
  // open Mqqtt
  gsm.my_flush();

  if (0 == gsm.MQTT_Open((char *)mqttServer, mqttPort, (char *)ESP_ID, (char *)topicSever4G, (char *)mqttUser, (char *)mqttPassword))
    debugSerial(F("Open Mqtt 4G OK"));
  else
  {
    debugSerial(F("Open Mqtt 4G False"));
    reInit4G += 5;
  }

  // if(0 == gsm.MQTT_Publish((char*)txt_subscribe, (char*)topicSever4G, (char*)HexString, sizeof(HexString))) {
  // if(0 == gsm.MQTT_Publish((char*)topicSever4G, "123456789901234567890123456789", numBytes)) {
  if (0 == gsm.MQTT_Publish((char *)txt_subscribe, (char *)topicSever4G, (char *)HexString, lenHexString))
  {
    debugSerial(F("Send Mqtt 4G OK"));
    net4Gconnected = true;
    debugSerial((char *)HexString);
    reInit4G = 0;
    numSendData++;
  }
  else
  {
    // dbSerial.println(F("Send Mqtt 4G False"));
    net4Gconnected = false;
    reInit4G += 5;
  }
}
/*************************************** EN200 - 4G END ************************/
int key_press()
{
  int key_adc = analogRead(KEY_IN);
  if (key_adc > 4000)
    return (0);
  if (key_adc < 2250)
    return (KEY_MODE);
  else if (key_adc > 2800)
    return (KEY_CHECK);
  else
    return (KEY_BELL);
  // if((key_adc>1500)&(key_adc<2250)){  // Key MODE 1.65V = 1840 1897   1800
  //   return(KEY_MODE);
  // }
  // else if((key_adc>2300)&(key_adc<2770)){  // Key BELL 2.24V = 2560 2645  2500 2738
  //   return(KEY_BELL);
  // }
  // else if((key_adc>2770)&(key_adc<3800)){  // Key CHECK 2.51V = 2900 2991 2850
  //   return(KEY_CHECK);
  // }
  // else if((key_adc>2000)&(key_adc<2050)){  // Key BELL + CHECK 2090
  //   return(KEY_B_C);
  // }
  // else if((key_adc>1520)&(key_adc<1620)){  // Key MODE + CHECK = 1574
  //   return(KEY_M_C);
  // }
  // else if((key_adc>1400)&(key_adc<1510)){  // Key MODE + BELL = 1460
  //   return(KEY_M_B);
  // }
  // else if((key_adc>1200)&(key_adc<1350)){  // Key MODE+BELL+CHECK = 1280
  //   return(KEY_M_B_C);
  // }
  // else {
  //   return 0;
  // }
}
int autoPower()
{
  // uint16_t ch_ac_adc;// = analogRead(CH_AC);
  VBAT_ON;
  gdata.pwStatus = 0; //
  // ch_ac_adc = analogRead(CH_AC);
  // gdata.pwStatus = PW_BAT;
  if (adcACOK < 2500)
  { // AC OK -  Adapter in
    gdata.pwStatus |= PW_ACIN;
    if (digitalRead(CHARGE))
    {
      gdata.pwStatus |= PW_CHARG;
    }

    if (gdata.batVoltage < 800)
    { // Charging when Vbat < 8.00
      CHARGE_ON;
    }
    // if((millis()-timeLoopBAT>timeAutoBat)&(gdata.powerBAT == 0)){
    //   timeLoopBAT = millis();
    //   gdata.powerBAT = 1;
    // }
    // if((gdata.powerBAT)&(gdata.batVoltage>800)){ // Power by Battery
    //   gdata.pwStatus |= PW_BAT;
    // if(gdata.batVoltage<gdata.VolLowPwBAT) {
    //   gdata.powerBAT = 0;
    //   VBAT_OFF;
    //   timeLoopBAT = millis();
    // }
    // else VBAT_ON;
    // }
  }
  else
  {
    gdata.pwStatus |= PW_BAT;
  }

  // if(gdata.pwStatus & PW_BAT) {
  //   if(digitalRead(CHANGE)) CHARGE_OFF;
  //   VBAT_ON;
  // } else VBAT_OFF;
  // VBAT_ON;
  // gdata.pwStatus |= ((gdata.batVoltage-640)/20)<<4;
  if (gdata.batVoltage >= 650)
    gdata.pwStatus |= BAT_OK;
}
void select_line(int line)
{
  int outLine;
  if (line < 8)
    outLine = 7 - line;
  else
    outLine = line;
  digitalWrite(SA0, bitRead(outLine, 0));
  digitalWrite(SA1, bitRead(outLine, 1));
  digitalWrite(SA2, bitRead(outLine, 2));
  digitalWrite(SA3, bitRead(outLine, 3));
  delay(2);
}
void read_sensor()
{
  int line;
  for (line = 0; line < 16; line++)
  {
    select_line(line);
    if (bitRead(gdata.lineNotUse, line))
      gdata.lineStatus[line] = NOT_USE; // ==NOT_USE
    else
    {
      if (digitalRead(INSEN) == LOW)
        gdata.lineStatus[line] = 0;
      else
        gdata.lineStatus[line] = (digitalRead(EOLINE) + digitalRead(INSEN) * 2);
    }
  }
}
void data_process()
{
  Vbat = analogRead(VBAT_FB) + 150;
  // cntAdc++;
  // if(cntAdc>9){
  // Vbat = Vbat/10 + 150;
  gdata.batVoltage = Vbat / 3 - Vbat / 80;
  //   Vbat = 0;
  //   cntAdc=0;
  // }
  // gdata.batVoltage =
  // if()bitSet(gdata.Status,)
  // gdata.Status = 123;

  adcACOK = analogRead(CH_AC);
  read_sensor();

  gdata.pwStatus = 0; //
  // ch_ac_adc = analogRead(CH_AC);
  // gdata.pwStatus = PW_BAT;
  if (adcACOK < 2500)
  { // AC OK -  Adapter in
    gdata.pwStatus |= PW_ACIN;
    if (digitalRead(CHARGE))
    {
      gdata.pwStatus |= PW_CHARG;
    }

    if (gdata.batVoltage < 800)
    { // Charging when Vbat < 8.00
      CHARGE_ON;
    }
  }
  else
  {
    gdata.pwStatus |= PW_BAT;
  }
  if (gdata.batVoltage >= 650)
    gdata.pwStatus |= BAT_OK;
}
void checkLineStatus()
{
  uint8_t line;
  // gdata.zoneArlam=0;
  // gdata.zoneWarning=0;
  for (line = 0; line < numLine; line++)
  {
    if (bitRead(gdata.lineNotUse, line))
    {
      gdata.holdStatus[line] = NOT_USE;
      bitClear(gdata.zoneArlam, line);
      bitClear(gdata.zoneWarning, line);
    }
    else
    { // Use line for check Alarm
      if (gdata.lineStatus[line] == ALARM)
      { // Detect Sensor active
        if (gdata.keySensor[line] != ALARM)
        {
          gdata.keySensor[line] = ALARM;
          gdata.timeHold[line] = 1;
        }
        else
        { // keySensor = ALARM
          gdata.timeHold[line]++;
          if (gdata.timeHold[line] >= TIME_HOLD_SENSOR)
          {
            gdata.timeHold[line] = TIME_HOLD_SENSOR;
            gdata.holdStatus[line] = ALARM;
            bitSet(gdata.zoneArlam, line);
          }
        }
      }
      else if (gdata.lineStatus[line] == ENDOFLINE)
      { // Detect Sensor OEL
        if (gdata.keySensor[line] != ENDOFLINE)
        {
          gdata.keySensor[line] = ENDOFLINE;
          gdata.timeHold[line] = 1;
        }
        else
        { // keySensor = ENDOFLINE
          gdata.timeHold[line]++;
          if (gdata.timeHold[line] >= TIME_HOLD_SENSOR)
          {
            gdata.timeHold[line] = TIME_HOLD_SENSOR;
            gdata.holdStatus[line] = ENDOFLINE;
            bitSet(gdata.zoneWarning, line);
          }
        }
      }
      else
      {
        gdata.timeHold[line] = 0;
        gdata.holdStatus[line] = NORMAL;
        gdata.keySensor[line] = NORMAL;
        // bitClear(gdata.zoneArlam,line);
        // bitClear(gdata.zoneWarning,line);
      }
    }
  }
}
void checkAlarm()
{
  int line;
  // uint32_t timeCheckAlarm;
  timeCheckAlarm = millis() - timeLoopAlarm;
  if (millis() - timeLoopAlarm > timeAlarm)
  {
    timeLoopAlarm = millis();
    timeAlarm = timeAlarmOn + timeAlarmOff;
    // timeCheckAlarm = 0;
  }
  gdata.Status = NORMAL;
  for (line = 0; line < numLine; line++)
  {
    if (gdata.holdStatus[line] == ENDOFLINE)
    {
      gdata.Status = ENDOFLINE;
    }
  }
  for (line = 0; line < numLine; line++)
  {
    if (gdata.holdStatus[line] == ALARM)
    {
      gdata.Status = ALARM;
    }
  }

  // Output Relay
  if (gdata.Status == ALARM)
  {
    gdata.LEDStatus = 1;
    if (timeCheckAlarm < timeAlarmOn)
    {
      // BELL_ON;
      if (gdata.enableBell)
      {
        BELL_ON;
        if (BUZZER_EN)
          BEEP_ON;
      }
      // else BELL_OFF;
    }
    else
    {
      BELL_OFF;
      BEEP_OFF;
    }

    if ((digitalRead(RF_REMOTE)) | (key_press() == KEY_BELL))
    {
      if (keyBell == 0)
      {
        gdata.enableBell = !gdata.enableBell;
        keyBell = 1;
        if (gdata.enableBell)
        {
          BELL_ON;
          if (BUZZER_EN)
            BEEP_ON;
        }
        else
        {
          BELL_OFF;
          BEEP_OFF;
        }
      }
    }
    else
      keyBell = 0;
    LAMP_ON;
    OUT1_ON;
    OUT2_ON;
  }
  else
  {
    if (gdata.Status == ENDOFLINE)
    {
    }
    gdata.enableBell = 1;
    gdata.LEDStatus = 2;
    BELL_OFF;
    LAMP_OFF;
    OUT1_OFF;
    OUT2_OFF;
    BEEP_OFF;
  }
}
void updateData2Sent()
{
  uint16_t index = 0;
  uint8_t line;
  uint32_t time1s = millis() / 1000;
  gdata.batPercent = (gdata.batVoltage - 640) / 2;
  if (gdata.batPercent > 100)
    gdata.batPercent = 100;
  gdata.rssiWF = WiFi.RSSI();

  dataload[index++] = numBytes; // 0
  for (line = 0; line < 12; line++)
    dataload[index++] = ESP_ID[line];
  for (line = 0; line < 15; line++)
    dataload[index++] = IMEI[line];
  dataload[index++] = gdata.rssiWF;   // 28
  dataload[index++] = gdata.rssi4G;   // 29
  dataload[index++] = gdata.pwStatus; // 30
  // dataload[index++] = gdata.batVoltage/256;
  // dataload[index++] = gdata.batVoltage%256;
  dataload[index++] = gdata.batPercent; // 31
  dataload[index++] = gdata.Status;     // 32
  dataload[index++] = numLine;          // 33
  for (line = 0; line < numLine; line++)
  {
    //  index = byteOfHeader + line * byteOfLine - 1;
    dataload[index++] = gdata.lineStatus[line];
  }
  // dataload[index++]  = (gdata.zoneArlam>>24)&0xFF;  //48
  // dataload[index++]  = (gdata.zoneArlam>>16)&0xFF;  //49
  // dataload[index++]  = (gdata.zoneArlam>>8)&0xFF;   //50
  // dataload[index++]  = (gdata.zoneArlam)&0xFF;      //51 numSendData

  // dataload[index++]  = (gdata.zoneWarning>>24)&0xFF;  //52
  // dataload[index++]  = (gdata.zoneWarning>>16)&0xFF;  //53
  // dataload[index++]  = (gdata.zoneWarning>>8)&0xFF;   //54
  // dataload[index++]  = (gdata.zoneWarning)&0xFF;      //55

  dataload[index++] = (numSendData >> 24) & 0xFF; // 48
  dataload[index++] = (numSendData >> 16) & 0xFF; // 49
  dataload[index++] = (numSendData >> 8) & 0xFF;  // 50
  dataload[index++] = (numSendData)&0xFF;         // 51 numSendData

  dataload[index++] = (time1s >> 24) & 0xFF; // 52
  dataload[index++] = (time1s >> 16) & 0xFF; // 53
  dataload[index++] = (time1s >> 8) & 0xFF;  // 54
  dataload[index++] = (time1s)&0xFF;         // 55

  // dataload[0] = numBytes;
  //  memset(HexString,0,sizeof(HexString));
  index = 0;
  index = updateHexString(dataload[0], index);
  for (line = 0; line < 27; line++)
  { // ID and IMEI
    HexString[index++] = dataload[line + 1];
  }
  for (line = 28; line < numBytes; line++)
  {
    index = updateHexString(dataload[line], index);
  }
  lenHexString = index;
}
void LED_Status(uint16_t cnt)
{
  if (deviceStatus == 0)
  { // don't connect to wifi
    if (cnt % 5 == 0)
    {
      LEDG_TOGGLE;
    }
    if (WiFi.status() == WL_CONNECTED)
    {
      deviceStatus = 1;
      BEEP_OFF;
    }
    else if ((net4Gconnected == false) & (gdata.Status != ALARM))
    {
      if (BUZZER_EN)
      {
        if ((cnt / 5) % 20 == 0)
          BEEP_TOGGLE;
        else
          BEEP_OFF;
      }
    }
  }
  else if (deviceStatus == 1)
  { // WifiConnected
    // if ((in_setupWifiManager == false)&(accessPointMode==false)) {
    if (accessPointMode == false)
    {
      if ((gdata.rssiWF < -30) & (gdata.rssiWF > -80))
      { // blink LED follow Streng of Wifi
        int x = (110 + gdata.rssiWF) / 4;
        if (cnt % 20 < x)
          LED_ON();
        else
          LED_OFF();
      }
      else
      {
        if (cnt % 20 < 1)
          LED_ON();
        else
          LED_OFF();
      }
    }
  }
  if (gdata.Status == NORMAL)
  {
    LEDG_ON;
    LEDR_OFF;
  }
  else if (gdata.Status == ALARM)
  {
    LEDR_ON;
    LEDG_OFF;
  }
  else if (gdata.Status == ENDOFLINE)
  {
    LEDG_ON;
    if (cnt % 2)
    {
      LEDR_TOGGLE;
    }
  }
}
void ESP_status(uint32_t sta)
{
  if (sta == ESP_OK)
  {
    debugSerial("ESP_OK");
  }
  else if (sta == ESP_ERR_INVALID_ARG)
  {
    debugSerial("ESP_ERR_INVALID_ARG");
  }
  else if (sta == ESP_ERR_INVALID_STATE)
  {
    debugSerial("ESP_ERR_INVALID_STATE");
  }
}
void taskStatus(void *xTaskParameters)
{
  while (1)
  {
    numTask1++;
    // longPressReset();
    data_process();
    checkLineStatus();
    if (gdata.startCheckAlarm)
      checkAlarm();
    else
    {
      BELL_OFF;
      LAMP_OFF;
      OUT1_OFF;
      OUT2_OFF;
    }
    if (numTask1 > 20)
    {
      if (digitalRead(RF_REMOTE))
        rfOk = true;
      else
        rfOk = false;
    }
    if ((numTask1 % 10) == 0)
    { // Onesec
      updateData2Sent();
      keyTest = key_press();
      if (debug)
        sendToPCTest();
      VBAT_ON;
      // Serial.print("Task Status is running on core ");
      // Serial.println(xPortGetCoreID());
      // autoPower();
    }
    // normal 30s send data, if alram 10s send data
    if (((numTask1 % TIME_WIFI) == 0) | ((gdata.Status == ALARM) & (numTask1 % 100 == 0)))
    {
      // autoPower();
      // yield();
      //  wifi_mqtt_process();
    }
    if ((gdata.startCheckAlarm == 0) & ((numTask1 % 50) == 0))
      gdata.startCheckAlarm = 1;
    gdata.relayStatus = (uint8_t)((digitalRead(LAMP) << 3) | (digitalRead(BELL) << 2) | (digitalRead(RL_OUT2) << 1) | digitalRead(RL_OUT1));
    LED_Status(numTask1);
    vTaskDelay(TIME_CHECK_STATUS / portTICK_PERIOD_MS);
  }
}
uint8_t check_crc_data(uint8_t data[], uint8_t len)
{
  uint8_t crc = 0;
  for (uint8_t i = 0; i < len; i++)
  {
    crc += data[i];
  }

  if (crc == data[len])
  {
    return 1;
  }
  return 0;
}
/************************************** CALL BACK *******************************/
void callback(char *topic, byte *payload, unsigned int length)
{
  unsigned int addSave, i, j;
  dbSerial.print("Message arrived: ");
  dbSerial.println(topic);

  dbSerial.print("Message len:");
  dbSerial.print(length);
  dbSerial.print(" ");
  for (i = 0; i < length; i++)
  {
    dbSerial.print((char)payload[i], HEX);
    dbSerial.print(" ");
  }

  if ((payload[0] == 0x68) && (payload[1] == 36))
  { // update Setting gdata
    for (i = 0; i < numLine; i++)
    {
      addSave = EEPROM_SETTING + i * 2;
      for (j = 0; j < 2; j++)
      { // numByte need change bit (uint32 = 4 byte)
        eepRead.uint8[1 - j] = payload[i * 2 + j + 2];
      }
      EEPROM.write(addSave, eepData.uint8[0]);
      EEPROM.write(addSave + 1, eepData.uint8[1]);
      //  EEUpdate4byte(addSave,eepRead.uint32[0]);
    }
    EEPROM.commit(); // Store data to EEPROM
    dbSerial.println("Update data");
    //  read_data_EE_EVM();
  }
  //  else if((payload[0]==0x69)&&(payload[1]==36))   { // update CMD for kWh Read
  //    for (i = 0; i < numLine; i++) {
  //      addSave = EE_ADD_EVM + numLine*4 + i*4;
  //      for(j=0;j<4;j++){                             // numByte need change bit (uint32 = 4 byte)
  //        eepRead.uint8[3-j] = payload[i*4+j+2];
  //      }
  //      EEUpdate4byte(addSave,eepRead.uint32[0]);
  //    }
  //    CMDUpdate = 1;
  //  }
  //  else if((payload[0]==0x56)&&(payload[1]==0x24))   { // update CMD for kWh after power cut off
  //    for (i = 0; i < numLine; i++) {
  //      addSave = EE_ADD_EVM + numLine*8 + i*4;      // 72 + phase*4 bytes
  //      for(j=0;j<4;j++){                             // numByte need change bit (uint32 = 4 byte)
  //        eepRead.uint8[3-j] = payload[i*4+j+2];
  //      }
  //      EEUpdate4byte(addSave,eepRead.uint32[0]);
  //      CMDUpdate = 1;
  //    }
  //    EEPROM.commit();    //Store data to EEPROM
  //    dbSerial.println("Update data");
  //    read_data_EE_EVM();
  //  }
  //  else if((payload[0]==0x55)&(payload[1]==11)){
  //    clearEepromEmeter();
  //    ESP.restart();
  //  }
  else if ((payload[0] == 0x68) & (payload[1] == 0x50))
  { // Reset
    //    clearEepromEmeter();
    ESP.restart();
  }

  dbSerial.println();
  dbSerial.println("-----------------------");
}
/*************************************** EEPROM *********************************/
//========================================== writeDefaultSettingsToEEPPROM
void saveSettingsToEEPPROM(char *ssid_, char *pass_)
{
  if (debug)
    Serial.println("\n============ saveSettingsToEEPPROM");
  writeEEPROM(1 * eepromTextVariableSize, eepromTextVariableSize, ssid_);
  writeEEPROM(2 * eepromTextVariableSize, eepromTextVariableSize, pass_);
}
//========================================== readSettingsFromEeprom
void readSettingsFromEEPROM(char *ssid_, char *pass_)
{
  readEEPROM(1 * eepromTextVariableSize, eepromTextVariableSize, ssid_);
  readEEPROM((2 * eepromTextVariableSize), eepromTextVariableSize, pass_);

  if (debug)
    Serial.println("\n============ readSettingsFromEEPROM");
  if (debug)
    Serial.print("\n============ ssid=");
  if (debug)
    Serial.println(ssid_);
  if (debug)
    Serial.print("============ password=");
  if (debug)
    Serial.println(pass_);
}
//================================================================
void writeEEPROM(int startAdr, int length, char *writeString)
{
  EEPROM.begin(eepromBufferSize);
  // yield();
  for (int i = 0; i < length; i++)
    EEPROM.write(startAdr + i, writeString[i]);
  EEPROM.commit();
  EEPROM.end();
}
//================================================================
void readEEPROM(int startAdr, int maxLength, char *dest)
{
  EEPROM.begin(eepromBufferSize);
  delay(10);
  for (int i = 0; i < maxLength; i++)
    dest[i] = char(EEPROM.read(startAdr + i));
  dest[maxLength - 1] = 0;
  EEPROM.end();
}
void saveStatusToEeprom(byte value)
{
  EEPROM.begin(eepromBufferSize);
  EEPROM.write(0, value);
  EEPROM.commit();
  EEPROM.end();
}
//===================================================================
byte getStatusFromEeprom()
{
  EEPROM.begin(eepromBufferSize);
  byte value = 0;
  value = EEPROM.read(0);
  EEPROM.end();
  return value;
}
//===================================================================
byte getDebugFromEeprom()
{
  EEPROM.begin(eepromBufferSize);
  byte value = 0;
  value = EEPROM.read(1);
  EEPROM.end();
  return value;
}
void saveDebugToEeprom(byte value)
{
  EEPROM.begin(eepromBufferSize);
  EEPROM.write(1, value);
  EEPROM.commit();
  EEPROM.end();
}
//===================================================================
void GSafe_Setting_Save()
{
  uint16_t add = EEPROM_SETTING;
  uint16_t i = 0; //,byte;
  EEPROM.begin(eepromBufferSize);
  EEPROM.write(EEPROM_SETTING - 1, 0x01);
  eepData.uint16[i++] = timeAlarmOn;
  eepData.uint16[i++] = timeAlarmOff;
  eepData.uint16[i++] = timeActiveAlarm;
  eepData.uint16[i++] = gdata.timeCounter;
  eepData.uint16[i++] = gdata.VolLowPwBAT;
  eepData.uint16[i++] = gdata.lineNotUse;
  // byte = (i-1)*2;
  for (i = 0; i < numBytesSave; i++)
  {
    EEPROM.write(add + i, eepData.uint8[i]);
  }
  EEPROM.commit();
  EEPROM.end();
}
void GSafe_Setting_Read()
{
  uint16_t add = EEPROM_SETTING;
  int16_t i; //,byte=numBytesSave;
  EEPROM.begin(eepromBufferSize);
  for (i = 0; i < numBytesSave; i++)
  {
    eepData.uint8[i] = EEPROM.read(add + i);
  }
  timeAlarmOn = eepData.uint16[0];
  timeAlarmOff = eepData.uint16[1];
  timeActiveAlarm = eepData.uint16[2];
  gdata.timeCounter = eepData.uint16[3];
  gdata.VolLowPwBAT = eepData.uint16[4];
  gdata.lineNotUse = eepData.uint16[5];
  if ((timeAlarmOn == 0xFFFF) & (timeAlarmOff == 0xFFFF) & (timeActiveAlarm == 0xFFFF) & (gdata.timeCounter == 0xFFFF) & (gdata.VolLowPwBAT == 0xFFFF))
    initStart(1);
  EEPROM.end();
}
// void write_EEPROM_Wifi(){
//   // uint16_t add = EEPROM_SETTING;
//   uint16_t i=0;//,byte;
//   String qsid = WiFi.SSID();
//   String qpass = WiFi.psk();
//   if (qsid.length() > 0 && qpass.length() > 0) {
//     debugSerial("clearing eeprom");
//     for (i = 0; i < 96; ++i) {
//       EEPROM.write(i, 0);
//     }
//     debugSerial(qsid);
//     debugSerial(qpass);
//     debugSerial("writing eeprom ssid:");
//     for (i = 0; i < qsid.length(); ++i)
//     {
//       EEPROM.write(i, qsid[i]);
//       #ifdef DEBUG
//       Serial.print("Wrote: ");
//       Serial.println(qsid[i]);
//       #endif // DEBUG
//     }
//     debugSerial("writing eeprom pass:");
//     for (i = 0; i < qpass.length(); ++i)
//     {
//       EEPROM.write(32 + i, qpass[i]);
//       #ifdef DEBUG
//       Serial.print("Wrote: ");
//       Serial.println(qpass[i]);
//       #endif // DEBUG
//     }
//     EEPROM.commit();
//     delay(100);
//     ESP.restart();
//   }
// }
// void write_EEPROM_Wifi_default(){
//   // uint16_t add = EEPROM_SETTING;
//   uint16_t i=0;//,byte;
//   debugSerial("clearing eeprom");
//   for (i = 0; i < 96; ++i) {
//     EEPROM.write(i, 0);
//   }
//   debugSerial("writing eeprom ssid:");
//   for (i = 0; i < sizeof(ssid); ++i)
//   {
//     EEPROM.write(i, ssid[i]);
//     #ifdef DEBUG
//     Serial.print("Wrote: ");
//     Serial.println(ssid[i]);
//     #endif // DEBUG
//   }
//   debugSerial("writing eeprom pass:");
//   for (i = 0; i < sizeof(pass); ++i)
//   {
//     EEPROM.write(32 + i, pass[i]);
//     #ifdef DEBUG
//     Serial.print("Wrote: ");
//     Serial.println(pass[i]);
//     #endif // DEBUG
//   }
//   EEPROM.commit();
//   delay(200);
//   ESP.restart();
// }
// uint8_t read_EEPROM_Wifi(){
//   String esid;
//   for (int i = 0; i < 32; ++i)
//   {
//     esid += char(EEPROM.read(i));
//   }
//   #ifdef DEBUG
//   Serial.println();
//   Serial.print("SSID: ");
//   Serial.println(esid);
//   Serial.println("Reading EEPROM pass");
//   #endif // DEBUG

//   String epass = "";
//   for (int i = 32; i < 96; ++i)
//   {
//     epass += char(EEPROM.read(i));
//   }
//   #ifdef DEBUG
//   Serial.print("PASS: ");
//   Serial.println(epass);
//   #endif // DEBUGSend Mqtt 4G OK
//   WiFi.begin(esid.c_str(), epass.c_str());
// }

//================ WiFi Manager necessary functions ==============
//==============================================
void handle_OnConnect()
{
  if (debug)
    Serial.println("Client connected: args=" + String(serverAP.args()));
  if (serverAP.args() >= 2)
  {
    handleGenericArgs();
    serverAP.send(200, "text/html", SendHTML(1));
  }
  else
    serverAP.send(200, "text/html", SendHTML(0));
}

//==============================================
void handle_NotFound()
{
  if (debug)
    Serial.println("handle_NotFound");
  serverAP.send(404, "text/plain", "Not found");
}

//=================================
void handleGenericArgs()
{ // Handler
  for (int i = 0; i < serverAP.args(); i++)
  {
    if (debug)
      Serial.println("*** arg(" + String(i) + ") =" + serverAP.argName(i));
    if (serverAP.argName(i) == "ssid")
    {
      if (debug)
        Serial.print("sizeof(ssid)=");
      Serial.println(sizeof(ssid));
      memset(ssid, '\0', sizeof(ssid));
      strcpy(ssid, serverAP.arg(i).c_str());
    }
    else if (serverAP.argName(i) == "pass")
    {
      if (debug)
        Serial.print("sizeof(pass)=");
      Serial.println(sizeof(pass));
      memset(pass, '\0', sizeof(pass));
      strcpy(pass, serverAP.arg(i).c_str());
    }
  }
  if (debug)
    Serial.println("*** New settings have received");
  if (debug)
    Serial.print("*** ssid=");
  Serial.println(ssid);
  if (debug)
    Serial.print("*** password=");
  Serial.println(pass);
  saveSettingsToEEPPROM(ssid, pass);
  ESP.restart();
}
//===================================
String SendHTML(uint8_t st)
{
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>ESP WiFi Manager</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 30px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += "label{display:inline-block;width: 160px;text-align: right;}\n";
  ptr += "form{margin: 0 auto;width: 360px;padding: 1em;border: 1px solid #CCC;border-radius: 1em; background-color: #6e34db;}\n";
  ptr += "input {margin: 0.5em;}\n";
  if (st == 1)
    ptr += "h3{color: green;}\n";
  ptr += "</style>\n";
  ptr += "<meta charset=\"UTF-8\">\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>GSafe WiFi Setup</h1>\n";
  if (st == 1)
    ptr += "<h3>WiFi settings has saved successfully!</h3>\n";
  else if (st == 2)
    ptr += "<h3>WIFI Credentials has saved successfully!</h3>\n";
  else
    ptr += "<h3>Enter the WiFi settings</h3>\n";
  ptr += "<form>";

  ptr += "<div><label for=\"label_1\">WiFi SSID</label><input id=\"ssid_id\" required type=\"text\" name=\"ssid\" value=\"";
  ptr += ssid;
  ptr += "\" maxlength=\"32\"></div>\n";

  ptr += "<div><label for=\"label_2\">WiFi Password</label><input id=\"pass_id\" type=\"text\" name=\"pass\" value=\"";
  ptr += pass;
  ptr += "\" maxlength=\"32\"></div>\n";

  ptr += "<div><input type=\"submit\" value=\"Submit\"accesskey=\"s\"></div></form>";
  ptr += "<h5></h5>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
