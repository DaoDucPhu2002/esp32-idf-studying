#include "TEE_EC200.h"
#include "HWConfig.h"

extern const char* mqttServer;
extern const int   mqttPort;
extern const char* mqttUser;
extern const char* mqttPassword;

extern const char* topicSever4G;
extern char* txt_subscribe;

#define  dbSerial Serial
// const char topicSever[] = "mmm/fire_alarm_gsafev3/data";
// char txt_subscribe[] = "mmm/123456789012/cmd";

//extern char* Esp8266ID;

//for testing
const char test_mqttServer[] = "test.mosquitto.org";
const int   test_mqttPort = 1883;
//const char* test_mqttUser = "lctech_rmq";
//const char* test_mqttPassword = "lctechrmq123@";


int START_PIN = 16;
EC200 gsm;
unsigned long previousMillis_timeout = 0; 

void event_null(String data){}
EC200::EC200()
{
	Event_debug =  event_null;
	//netwk_4G_status = 0;
}
#ifdef SW_SERIAL
void EC200:: begin(SoftwareSerial *serial,long baud)
{
	serial->begin(baud);
	_Serial = serial;	
	netwk_4G_status = 0;
}
#endif

void EC200:: begin(HardwareSerial *serial,long baud)
{
	serial->begin(baud);
	_Serial = serial;
	netwk_4G_status = 0;
}
#if ATLSOFTSERIAL 
void EC200:: begin(AltSoftSerial *serial,long baud)
{
	serial->begin(baud);
	_Serial = serial;
	
}
#endif
void EC200:: debug (String data)
{
	//(*Event_debug)(data);
	#ifdef DEBUG
	dbSerial.print(data);
	#endif
}
void EC200:: debugln (String data)
{
	//(*Event_debug)(data);
	#ifdef DEBUG
	dbSerial.println(data);
	#endif
}

void EC200::SetPowerKeyPin(int pin)
{
	START_PIN = pin;
}
bool EC200::PowerOn()
{	
	String req="";
	//_Serial->println(F("AT"));
	
	pinMode(START_PIN, OUTPUT);
	
	digitalWrite(START_PIN, HIGH);
	delay(1000);
	digitalWrite(START_PIN, LOW);
	//delay(1000);
				
	/*
	while(!_Serial->available())
	{
		delay(1);
	}
	start_time_out();
	*/
	
	unsigned long pv_out = millis(); 
	
	while(1)
	{
		if(_Serial->available())
		{
			req = _Serial->readStringUntil('\n');	
			// debugln(req);
			if(req.indexOf(F("RDY")) != -1)
			{
				start_time_out();
				//debugln(F("Power ON"));				
				return(true);
			}
			if(req.indexOf(F("POWERED DOWN")) != -1)
			{
				//start_time_out();
				debugln(F("Power OFF"));
				pinMode(START_PIN, OUTPUT);
				digitalWrite(START_PIN, HIGH);
				delay(1000);
				digitalWrite(START_PIN, LOW);
				delay(1000);
			}						
		}
		unsigned long current_out = millis();
			//debugln("x");
		if(current_out - pv_out >= (8000)) 
		{
				digitalWrite(START_PIN, HIGH);
				delay(1000);
				digitalWrite(START_PIN, LOW);
				//delay(500);
				//debugln(F("Power Retry"));
				pv_out = current_out;
		}
	}
}
bool EC200::PowerOff()
{
	pinMode(START_PIN, OUTPUT);
	digitalWrite(START_PIN, HIGH);
    delay(1000);
	digitalWrite(START_PIN, LOW);
	delay(1000);
	while(!_Serial->available())
	{
		delay(1);
	}
	while(1)
	{
		String req = _Serial->readStringUntil('\n');
		//debugln(req);		
	    if(req.indexOf(F("RDY")) != -1)
		{
			//debugln(F("Power ON"));
			pinMode(START_PIN, OUTPUT);
			digitalWrite(START_PIN, HIGH);
			delay(1000);
			digitalWrite(START_PIN, LOW);
			delay(1000);
		}
		if(req.indexOf(F("POWERED DOWN")) != -1)
		{
			//debugln(F("Power OFF"));
			return(true);
		}
	}
	
}
bool EC200::WaitReady()
{
	start_time_out();
	_Serial->println(F("AT+COPS? "));
	while(1)
	{
		if(_Serial->available())	{
			String req = _Serial->readStringUntil('\n');	
			debugln(req);
			if(req.indexOf(F("OK")) != -1)
			{
				debugln(F("\r\nEC200 Ready..."));
				debugln(F("\r\nClose Echo"));
				_Serial->println(F("AT+CREG?"));
				wait_ok(1000);
				
				return(false);
			}
			else if(req.indexOf(F("POWERED DOWN")) != -1)
			{
				digitalWrite(START_PIN, HIGH);
				delay(1000);
				digitalWrite(START_PIN, LOW);
				delay(1000);
				PowerOn();
			}
			else
			{
				//indicate network registered OK
				netwk_4G_status = 1;
				debugln(F("4G network OK"));
				return(true);
			}
		}
		if(time_out(3000))
		{
			debugln(F("\r\nNo response--> Please check Hardware"));
			return(false);
		}
		debugln(F("."));
	}
	return(true);
}
String EC200::GetOperator()
{
	_Serial->flush();
	_Serial->println(F("AT+COPS?"));
	while(!_Serial->available())
	{}
	
	start_time_out();
	while(1)
	{
		String req = _Serial->readStringUntil('\n');	
		//String req2 = _Serial->readStringUntil('\n');
		if(req.indexOf(F("+COPS:")) != -1)
		{
			//debugln(req);
			//debugln(req2);
			//+COPS: 0,0,"TRUE-H",2
			char comma1 = req.indexOf(F(","));
			char comma2 = req.indexOf(F(","),comma1+1);
			char comma3 = req.indexOf(F(","),comma2+1);
			String  operate_name = req.substring(comma2+2,comma3-1);
			String  acc_tech = req.substring(comma3+1);
			return(operate_name + "," + acc_tech);			
			//return(req.substring(req.indexOf(F("\""))));
		}
		if(time_out(1000))
		{
			return(F(""));
		}
		
	}
	return(F(""));
}
String EC200::GetIMEI()
{
	_Serial->flush();
	_Serial->println(F("AT+GSN"));		// AT+CGSN=? AT+CGSN
	// while(!_Serial->available())
	// {}
	start_time_out();
	while(1)
	{
		String req = _Serial->readStringUntil('\n');	
		//String req2 = _Serial->readStringUntil('\n');
		if(req.length() >=14 )
		{
			//debugln(req);
			//debugln(req2);
			//+COPS: 0,0,"TRUE-H",2		
			return(req);
		}
		if(time_out(3000))
		{
			return(F(""));
		}
		
	}
	return(F(""));
}
unsigned char EC200::SignalQuality()
{
	unsigned char ret = 101;
	_Serial->println(F("AT+CSQ"));
	while(!_Serial->available())
	{}
	start_time_out();
	while(1)
	{
		String req = _Serial->readStringUntil('\n');	 
		if(req.indexOf(F("+CSQ")) != -1)
		{
			ret = req.substring(req.indexOf(F(" "))+1,req.indexOf(F(","))).toInt();
			return(ret);
		}
		if(time_out(1000))
		{
			return(ret);
		}		
	}
	return(ret);
}
void EC200::start_time_out()
{
	previousMillis_timeout = millis();
}
bool EC200::time_out(unsigned long timeout_interval)
{
	unsigned long currentMillis = millis();
	if(currentMillis - previousMillis_timeout >= timeout_interval) 
	{
		previousMillis_timeout = currentMillis;
		return(true);
	}
	return(false);
}
bool EC200:: wait_ok(long time)
{
	return wait_ok_(time,true);
}
bool EC200:: wait_ok_ndb(long time)
{
	return wait_ok_(time,false);
}
	
bool EC200:: wait_ok_(long time,bool ack)
{
	unsigned long previousMillis = millis(); 
	while(1)
	{
		String req = _Serial->readStringUntil('\n');
		if(req.indexOf(F("OK")) != -1)
		{
			if(ack)
			debugln(F("OK"));
			return(1);
		}
		if(req.indexOf(F("ERROR")) != -1)
		{
			if(ack)
			debugln(F("Error"));
			return(0);
		}
		//debugln(req);	
		unsigned long currentMillis = millis();
		if(currentMillis - previousMillis >= time) 
		{
			previousMillis = currentMillis;
			if(ack)
			debugln(F("Error"));
			return(0);
		}			

	}
	
}

int8_t EC200:: waitResponse(uint32_t timeout_ms, String expect_str) {

    //String data[100],buffer;
	String data;
	//data = &buffer;
	data.reserve(64);
    uint8_t  index       = 0;
    uint32_t startMillis = millis();
    do {
      yield();
      while (_Serial->available() > 0) {
        yield();
        uint8_t a = _Serial->read();
        if (a <= 0) continue;  // Skip 0x00 bytes, just in case
        data += static_cast<char>(a);
		//data = String(data+a);
		//data += a;
		//buffer(*a);
		//data.concat(buffer);
        if (data.endsWith(expect_str)) {
          index = 1;
          goto finish;
        } else if (data.endsWith(String("ERROR"))) {
          index = 2;
          goto finish;
        } 
      }
    } while (millis() - startMillis < timeout_ms);
  finish:
	// dbSerial.print(F("_DBG:"));
	// dbSerial.println(data);
	// dbSerial.print("_Index:");
	// dbSerial.println(String(index));
	debug(F("_DBG:"));
	debugln(data); 
	debug("_Index:");
	debugln(String(index));
    if (!index) {
      data.trim();
      if (data.length()) { 
		dbSerial.println("### Unhandled:"); 
		}
      data = "";
    }

    return index;
  }

unsigned char EC200:: event_input()
{
	while(_Serial->available())
	{
		String req = _Serial->readStringUntil('\n');	
	   
	    if(req.indexOf(F("RING")) != -1)
		{
			event_type = EVENT_RING;
			return(EVENT_RING);//EVENT_RING
		}
		else if(req.indexOf(F("+CMTI: \"ME\"")) != -1)
		{
			event_type = EVENT_SMS;
			char index = req.indexOf(F(","));
			index_new_SMS =  req.substring(index+1).toInt();
			
			return(EVENT_SMS);//EVENT_SMS
		}
	}
	event_type = EVENT_NULL;
	return(EVENT_NULL);//EVENT_NULL
}

/* Code Serial */
String EC200:: readStringUntil(char data)
{
	 return _Serial->readStringUntil(data);
}
size_t EC200:: print(String data)
{
     return _Serial->print(data);
}
size_t EC200:: println(String data)
{
     return _Serial->println(data);
}

void EC200:: print(unsigned char data,int type)
{
	_Serial->print(data,type);
}
void EC200:: print(int data,int type)
{
	_Serial->print(data,type);
}
void EC200:: print(unsigned int data,int type)
{
	_Serial->print(data,type);
}
void EC200:: print(long data,int type)
{
	_Serial->print(data,type);
}
size_t EC200:: print(String data,int type)
{
   int i=0;
   while(data[i])
   {
		_Serial->print(data[i],type);
		i++;   
   }
   return(i-1);
   
   // Serial.print(data,type);
	// return _Serial->print(data,type);
}
size_t EC200:: println(String data,int type)
{
	int i=0;
	while(data[i])
	{
		_Serial->print(data[i],type);
		i++;   
	}
	_Serial->println("");
    return(i+1);


	//return _Serial->println(data,type);
}
int EC200:: peek()
{
     return _Serial->peek();
}
size_t EC200:: write(uint8_t byte)
{
     return _Serial->write(byte);
}
int EC200:: read()
{
     return _Serial->read();
}
int EC200:: available()
{
     return _Serial->available();
}
void EC200:: flush()
{
     _Serial->flush();
}
void EC200 :: my_flush()
{
	while(gsm.available())
	{
		gsm.read();
	}
}
int EC200::MQTT_Close()	{
	//close MQTT connection
	#ifdef DEBUG
	dbSerial.println(F("MQTT_CLOSE_CONN"));
	#endif // DEBUG
	
	_Serial->flush();
	_Serial->println("AT+QMTDISC=0");
	//if(1 != wait_ok(1000))	{
	//waitResponse(1000,String("OK"));
	
	return 1;
}
int EC200::MQTT_Open(char* mqtt_server, int mqtt_port, char* esp_id, char* topic_srv,char* user,char* passwd)	{
	// String txtSend;
	#ifdef DEBUG
	dbSerial.println(F("MQTT_CONFIG"));
	#endif // DEBUG
	_Serial->flush();
	// _Serial->println(F("AT+QMTCFG=\"recv/mode\",0,0,1"));  // AT+QMTCFG=recv/mode,0,0,1
	_Serial->println(F("AT+QMTCFG=recv/mode,0,0,1"));
	if(1!= waitResponse(1000,String("OK"))) {
		return 1;
	}
	// _Serial->flush();
	// _Serial->println(F("AT+QMTCFG=\"keepalive\",0,200"));
	// if(1!= waitResponse(1000,String("OK"))) {
	// 	return 2;
	// }

	// _Serial->flush();
	// _Serial->println(F("AT+QMTCFG=\"timeout\",0,60,10,1"));
	// if(1!= waitResponse(1000,String("OK"))) {
	// 	return 3;
	// }
	#ifdef DEBUG
	dbSerial.println(F("MQTT_OPEN_PORT"));
	#endif
	_Serial->flush();
	//_Serial->println(F("AT+QMTOPEN=0,test.mosquitto.org,1883"));
	// _Serial->print(F("AT+QMTOPEN=0,\""));
	// txtSend = "AT+QMTOPEN=0," + ((char*)(mqtt_server)) + "," + String(mqtt_port);
	_Serial->print(F("AT+QMTOPEN=0,"));
	_Serial->print((char*)mqtt_server);
	// _Serial->print(F("\","));
	_Serial->print(F(","));
	_Serial->println(String(mqtt_port));
	if(1 != waitResponse(10000,String("+QMTOPEN: 0,0"))) {
	// if(1 != waitResponse(10000,String("OK"))) {
		return 4;
	}
	#ifdef DEBUG
	dbSerial.println(F("MQTT_CILENT_ID"));
	#endif // DEBUG
	_Serial->flush(); 
	//_Serial->println(F("AT+QMTCONN=0,esp8266_wifi_mac081522"));
	// _Serial->print(F("AT+QMTCONN=0,\""));
	// _Serial->print((char*)esp_id);
	// _Serial->print(F("\",\""));
	// _Serial->print((char*)user);
	// _Serial->print(F("\",\""));
	// _Serial->print((char*)passwd);
	// _Serial->println(F("\""));
	_Serial->print(F("AT+QMTCONN=0,"));
	_Serial->print((char*)esp_id);
	_Serial->print(F(","));
	_Serial->print((char*)user);
	_Serial->print(F(","));
	_Serial->println((char*)passwd);
	//_Serial->println();
	if(1 != waitResponse(10000,String("+QMTCONN: 0,0,0"))) {
	// if(1 != waitResponse(10000,String("OK"))) {
		return 5;
	}

	//success case
	return 0;
}
int EC200::MQTT_Publish(char* subcribe_srv, char* topic_srv, char* mqqtt_data,unsigned int length)	{
	#ifdef DEBUG
	dbSerial.println(F("MQTT_SUB"));
	#endif // DEBUG
	_Serial->flush();
	//_Serial->print(F("AT+QMTSUB=0,1,mmm/fire_4G/data,2"));
	// _Serial->print(F("AT+QMTSUB=0,1,\""));
	// _Serial->print(subcribe_srv);
	// _Serial->println(F("\",0"));
	_Serial->print(F("AT+QMTSUB=0,1,"));
	_Serial->print(subcribe_srv);
	_Serial->println(F(",0"));
	if(1 != waitResponse(10000,String("+QMTSUB:"))) {
	// if(1 != waitResponse(10000,String("OK"))) {
		return 3;
	}
	// delay(5000);

	dbSerial.println(F("MQTT_PUB"));
	_Serial->flush();
	//_Serial->println(F("AT+QMTPUBEX=0,0,0,0,mmm/fire_4G/data,2"));
	// _Serial->print(F("AT+QMTPUBEX=0,0,0,0,\""));
	// _Serial->print((char*)topic_srv);
	// _Serial->print(F("\","));
	_Serial->print(F("AT+QMTPUBEX=0,0,0,0,"));
	_Serial->print((char*)topic_srv);
	_Serial->print(F(","));
	_Serial->println(String(length));
	// delay(1000);
	if(1 != waitResponse(10000,String(">"))) {
		return 1;
	}
	_Serial->println((char*)mqqtt_data);
	#ifdef DEBUG
		dbSerial.print(F("data to server:"));
		dbSerial.println((char*)mqqtt_data);
	#endif // DEBUG
	// _Serial->flush();
	// dbSerial.println(F("MQTT_DATA_WRITE"));

	//success case
	return 0;
}
String EC200::MQTT_setup(char* esp_id, char* topic_srv, String mqqtt_data)	{
	// uint8_t cmd_stt = 0;
	#ifdef DEBUG
	dbSerial.println(F("MQTT_CONFIG"));
	#endif // DEBUG
	_Serial->flush();
	_Serial->println(F("AT+QMTCFG=recv/mode,0,0,1"));
	if(1!= waitResponse(1000,String("OK"))) {
	//if(1 != wait_ok(1000))	{
		goto setup_end;
	}
	//delay(1500);
	_Serial->flush();
	_Serial->println(F("AT+QMTCFG=\"keepalive\",0,200"));
	if(1!= waitResponse(1000,String("OK"))) {
	//if(1 != wait_ok(1000))	{
		goto setup_end;
	}

	//delay(1500);
	_Serial->flush();
	_Serial->println(F("AT+QMTCFG=\"timeout\",0,60,10,1"));
	if(1!= waitResponse(1000,String("OK"))) {
	//if(1 != wait_ok(1000))	{
		goto setup_end;
	}
	
	//delay(2000);
	#ifdef DEBUG
	dbSerial.println(F("MQTT_OPEN_PORT"));
	#endif // DEBUG
	_Serial->flush();
	//_Serial->println(F("AT+QMTOPEN=0,test.mosquitto.org,1883"));
	_Serial->print(F("AT+QMTCONN=0,"));
	_Serial->print(test_mqttServer);
	_Serial->print(F(","));
	_Serial->println(String(test_mqttPort));
	//_Serial->println(F("AT+QMTOPEN=0,"+ mqttServer + String(mqttPort)));
	//if(1 == waitResponse(1000,String("+QMTOPEN:"))) {
	if(1 != waitResponse(10000,String("+QMTOPEN: 0,0"))) {
	//if(1 != wait_ok(1000))	{
		goto setup_end;
	}
	
	//delay(2000);
	#ifdef DEBUG
	dbSerial.println(F("MQTT_CILENT_ID"));
	#endif // DEBUG
	_Serial->flush(); 
	//_Serial->println(F("AT+QMTCONN=0,esp8266_wifi_mac081522"));
	_Serial->print(F("AT+QMTCONN=0,"));
	_Serial->println((char*)esp_id);
	if(1 != waitResponse(10000,String("+QMTCONN: 0,0,0"))) {
	//if(1 == waitResponse(5000,String("+QMTCONN:"))) {
	//if(1 == waitResponse(5000,String("+QMT"))) {
	//if(1 != wait_ok(5000))	{
		goto setup_end;
	}
	
	//delay(2000);
	#ifdef DEBUG
	dbSerial.println(F("MQTT_TOPIC"));
	#endif // DEBUG
	_Serial->flush();
	//_Serial->print(F("AT+QMTSUB=0,1,mmm/fire_4G/data,2"));
	_Serial->print(F("AT+QMTSUB=0,1,"));
	_Serial->print(topic_srv);
	_Serial->println(",2");
	if(1 != waitResponse(10000,String("+QMTSUB: 0,1,0,"))) {
	//if(1 != wait_ok(1000))	{
		goto setup_end;
	}
	//delay(1000);
	#ifdef DEBUG
	dbSerial.println(F("MQTT_DATA_OPEN"));
	#endif // DEBUG
	_Serial->flush();
	//_Serial->println(F("AT+QMTPUBEX=0,0,0,0,mmm/fire_4G/data,2"));
	_Serial->print(F("AT+QMTPUBEX=0,0,0,0,"));
	_Serial->print(topic_srv);
	_Serial->println(",29");
	if(1 != waitResponse(10000,String(">"))) {
	//if(1 != wait_ok(1000))	{
		goto setup_end;
	}

	//delay(1000);
	#ifdef DEBUG
	dbSerial.println(F("MQTT_DATA_WRITE"));
	#endif // DEBUG
	_Serial->flush();
	_Serial->println(mqqtt_data);
	//if(1 != wait_ok(1000))	{
	if(1 != waitResponse(1000,String("OK"))) {
		goto setup_end;
	}
	
	setup_end:
	//close MQTT connection
	//dbSerial.println(F("MQTT_CLOSE_CONN"));
	//_Serial->flush();
	//_Serial->println("AT+QMTDISC=0");
	//if(1 != wait_ok(1000))	{
	//waitResponse(1000,String("OK"));
	
	return(F(""));
}
