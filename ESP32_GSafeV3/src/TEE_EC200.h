#ifndef EC200_h
#define EC200_h

#include <Arduino.h>
#include <Stream.h>

// #define SW_SERIAL	1

#ifdef SW_SERIAL
#include <SoftwareSerial.h>
#endif // SW_SERIAL

#define EVENT_NULL	0
#define EVENT_RING	1
#define EVENT_SMS	2
#define UFS "UFS"
#define RAM "RAM"
#define COM "COM"
class EC200 
{
public:
	EC200();
	#ifdef SW_SERIAL
	 void begin(SoftwareSerial *serial,long baud);	
	#endif
	void begin(HardwareSerial *serial,long baud);
	#if ATLSOFTSERIAL 
	void begin(AltSoftSerial *serial,long baud);
	#endif
	void (*Event_debug)(String data);
	void debug (String data);
	void debugln (String data);
	void SetPowerKeyPin(int pin);
	bool PowerOn();
	bool PowerOff();
	bool WaitReady();
	String GetIMEI();
	String GetOperator();
	String MQTT_setup(char* esp_id, char* topic_srv, String mqqtt_data);
	int MQTT_Close();
	int MQTT_Open(char* mqtt_server, int mqtt_port, char* esp_id, char* topic_srv,char* user,char* passwd);
	int MQTT_Publish(char* subcribe_srv, char* topic_srv, char* mqqtt_data,unsigned int length);
	unsigned char SignalQuality();
	bool wait_ok_(long time,bool db);
	bool wait_ok(long time);
	bool wait_ok_ndb(long time);
	int8_t waitResponse(uint32_t timeout_ms, String expect_str);
	unsigned char event_input();
	unsigned char event_type;
	
	unsigned char index_new_SMS;
	void start_time_out();
	bool time_out(unsigned long timeout_interval);
	
	int  peek();
	virtual size_t write(uint8_t byte);
	int read();
    int available();
    virtual void flush();
	void print(unsigned char data,int type);
	void print(int data,int type);
	void print(unsigned int data,int type);
	void print(long data,int type);
	size_t print(String data);
	size_t println (String data);
	size_t print(String data,int type);
	size_t println (String data,int type);
	String readStringUntil(char data);
	void  my_flush();
protected:
	 Stream *_Serial;
	 int netwk_4G_status;
};

#endif
