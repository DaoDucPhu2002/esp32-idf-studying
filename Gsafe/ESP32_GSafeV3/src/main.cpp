#include "main.h"

#define TIME_LOOP  1000
uint32_t loop_mqtt,looptest;
uint16_t numUpdate;
extern uint16_t adcACOK;
extern struct gsafe_msg_data gdata ; //staMode

void setup() {
 // in_smartconfig = false;
  InitIO(); 
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  xTaskCreate(taskStatus,"TaskStatus",20000,NULL,1,NULL);
  // xTaskCreate(taskStatus4G,"Task4G",20000,NULL,1,NULL);
  //xTaskCreate()
  // xTaskCreatePinnedToCore(taskStatus,"TaskStatus",20000,NULL,1
  
  // sendTimePC("Start: ", millis()/1000);
  // 
  
  // LAMP_ON;
  checkStartWifi();
  // sendTimePCln("Init Wifi: ", millis()/1000);
  macRead(); 
  BEEP_OFF;
  // loop_mqtt = millis()/1000;
  // sendTimePCln("Init 4G: ", loop_mqtt);

  init_4G();
 //autoStatus4G();
  // loop_mqtt = millis()/1000;
  // sendTimePCln("Init done: ", loop_mqtt);
  data_process();
  updateData2Sent();
  // sendToPC();
  // esp_task_wdt_init(20, true); //enable panic so ESP32 restarts
  // esp_task_wdt_add(NULL); //add current thread to WDT watch
}
// void testAdc(){
//   uint16_t adc;
//   if(millis()-looptest>1000){
//     looptest = millis();
//     adc = analogRead(CH_AC);
//     sendTimePC(" CH_AC:",adc); 
//     adc = analogRead(KEY_IN);
//     sendTimePC(" KEY_IN:",adc); 
//     adc = analogRead(VBAT_FB);
//     sendTimePCln(" VBAT_FB:",adc); 
//   }
// }
// void testIO(int x){
//   if(x){
//     digitalWrite(SA0,HIGH);
//     digitalWrite(SA1,HIGH);
//     digitalWrite(SA2,HIGH);
//     digitalWrite(SA3,HIGH);
//     digitalWrite(LAMP,HIGH);
//     digitalWrite(BELL,HIGH);
//     digitalWrite(RL_OUT1,HIGH);
//     digitalWrite(RL_OUT2,HIGH);
//   }
//   else{
//     digitalWrite(SA0,LOW);
//     digitalWrite(SA1,LOW);
//     digitalWrite(SA2,LOW);
//     digitalWrite(SA3,LOW);
//     digitalWrite(LAMP,LOW);
//     digitalWrite(BELL,LOW);
//     digitalWrite(RL_OUT1,LOW);
//     digitalWrite(RL_OUT2,LOW);
//   }
// }
// void testLine(int x){
//   select_line(x);
//   delay(50);
//   dbSerial.print("Line: ");dbSerial.print(x);
//   dbSerial.print(" EOL:");dbSerial.print(digitalRead(EOLINE));
//   dbSerial.print(" ALR:");dbSerial.println(digitalRead(INSEN));
// }

// void test_imei(){
//   String txt = gsm.GetIMEI();
//   debugSerial(txt);
// }

void loop() {
  smartUpdateOTA();
  //autoStatus4G();
  if ((millis() - loop_mqtt) > TIME_LOOP) {   // 1 sec
    loop_mqtt = millis();

    numTimeSend4G++;
    // adcACOK = analogRead(CH_AC); 
    // esp_task_wdt_reset();
    // sendToPCTest();      
    if(numTimeSend4G%10==0)
    {    // 10s= check send to Sever by 4G
     /* if wifi mqtt connected  send 300s, else 30s send again = edit in sendToSever4G()*/
      sendToSever4G(); // 
      wifi_mqtt_process();   
      // Serial.print("Task main is running on core ");
      // Serial.println(xPortGetCoreID());
      // dbSerial.print("Finish Send to Sever: ");dbSerial.println(millis()/1000);
    }
    //ESP_status(esp_task_wdt_reset());
    esp_task_wdt_reset();
   // tick();
    //yield();
  }  
}