#include <Wire.h> 
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "MPU6050.h"
  
#ifdef ESP8266
#include< ESP8266WiFi.h>
#else 
#include <WiFi.h>
#endif

#include <PubSubClient.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <WebServer2.h>
#include <WiFiManager2.h>
//#include <ESPmDNS.h>
#include <DNSServer2.h>
#include "time.h"


const char* ssid = "FPT Telecom-453E";
const char* password = "123456788";
const char* default_mqtt_server = "broker.hivemq.com";
const char* default_mqtt_port = "1883";
const char* device_id = "meo_esp32";
String hotspot_name_prefix = "ESP_8266_";
char mqtt_server[255];
char mqtt_port[6];

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 25200;
const int   daylightOffset_sec = 0;

WiFiClient espClient;
PubSubClient client(espClient);

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif


MPU6050 accelgyro(0x69); // <-- use for AD0 high

int16_t ax, ay, az;
int16_t gx, gy, gz;

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

const byte DS1307 = 0x68;

const byte NumberOfFields = 7;
int second, minute, hour, day, wday, month, year;

bool wifi=0, mqtt=0,home=1;

const char *menu_strlist = 
  "acel/gyro demo\n"
  "Infrared demo\n"
  "Heart-rate demo\n"
  "Setting\n"
  "About\n"
  "Back"
  ;
  
const char *settiing_strlist =
  "Turn on/off WIFI\n"
  "Settime\n"
  "Back"
  ;

void setup_wifi() {
/*  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  if (WiFi.status() != WL_CONNECTED) {
    
  }
  // We start by connecting to a WiFi network
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
  */
  while (1){
	  WiFiManager wifiManager;
	  WiFiManagerParameter custom_text("<br/><p>Enter MQTT Server/IP and Port Number</p>");
	  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", default_mqtt_server, 255);
	  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", default_mqtt_port, 6);
	  wifiManager.addParameter(&custom_text);
	  wifiManager.addParameter(&custom_mqtt_server);
	  wifiManager.addParameter(&custom_mqtt_port);
	  //wifiManager.resetSettings(); //uncomnent khi muốn để default mqtt broker
	  if (!wifiManager.autoConnect(hotspot_name_prefix.c_str())&&u8g2.userInterfaceMessage("Failed to connect to","WIFI",""," Retry \n Cancel ") == 2) {
		turn_off_wifi();
		return;
	  }
	  else{ 
      strcpy(mqtt_server, custom_mqtt_server.getValue());
      strcpy(mqtt_port, custom_mqtt_port.getValue());
	    break;
	  }
	  }

  Serial.println("connected...yeey :)");
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  wifi=true;
}

void turn_off_wifi(){
	WiFi.disconnect(true);
	WiFi.mode(WIFI_OFF);
	wifi=false;
}	
void callback(char* topic, byte* message, unsigned int length) { //ESP32
//void callback(String topic, byte* message, unsigned int length) { //ESP8266
 // Serial.print("Message arrived on topic: ");
  //Serial.print(topic);
 // Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
   // Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  //Serial.println();
  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if (topic == "receive") {
    u8g2.clearBuffer();          // clear the internal memory
    u8g2.setFont(u8g2_font_6x12_tr); // choose a suitable font
    u8g2.setCursor(0,20);
    u8g2.print(messageTemp);
    //char a = (char) messageTemp ;
    //u8g2.drawStr(0,10,messageTemp);  
    u8g2.sendBuffer();    
    delay(3500);
  }
  Serial.println();
   if (topic == "settime") {
	 u8g2.clearBuffer();          // clear the internal memory
    u8g2.setFont(u8g2_font_6x12_tr); // choose a suitable font
    u8g2.setCursor(0,20);
	u8g2.print("setting time.............");
	u8g2.sendBuffer();
	byte hour = ((int)message[0] -48)*10;
	hour += ((int)message[1] -48);
	byte minute = ((int)message[2] -48)*10;
	minute += ((int)message[3] -48);
	byte date = ((int)message[4] -48)*10;
	date +=((int)message[5] -48);
	byte month = ((int)message[6] -48)*10;
	month +=((int)message[7] -48)*10;
	byte year = ((int)message[8] -48)*10;
	year +=((int)message[9] -48);
	byte weekd = ((int)message[9] -48);
	delay(500);
	setTime(hour,minute,0, weekd,day,month,year);
}}

// Change the function below if you want to subscribe to more topics with your ESP8266
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    /*
      YOU MIGHT NEED TO CHANGE THIS LINE, IF YOU'RE HAVING PROBLEMS WITH MQTT MULTIPLE CONNECTIONS
      To change the ESP device ID, you will have to give a new name to the ESP8266.
      Here's how it looks:
       if (client.connect("ESP8266Client")) {
      You can do it like this:
       if (client.connect("ESP1_Office")) {
      Then, for the other ESP:
       if (client.connect("ESP2_Garage")) {
      That should solve your MQTT multiple connections problem
    */
  //  if (client.connect("ESP32_anhtu0310","pi","12345678")) {
  if (client.connect(device_id)) {
	Serial.println("connected");
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("receive");
	    client.subscribe("settime");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

uint8_t u8x8_GetMenuEvent(u8x8_t *u8x8) 
{
  if (touchRead(T5) == 5)  return U8X8_MSG_GPIO_MENU_SELECT;
  if (touchRead (T6) ==5)  return U8X8_MSG_GPIO_MENU_UP; 
  if (touchRead(T4) ==5)  return U8X8_MSG_GPIO_MENU_DOWN;  
  else return NULL; 
}


void setup()
{
//  void setSleepEnabled(bool enabled);
  u8g2.begin();
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000);
  accelgyro.initialize();
  setTime(14, 0, 0, 4, 29, 11, 17); // 14:00:00 T4 29-11-2017
  /* ask user whether turn on WIFi and MQTT or not */
  u8g2.setFont(u8g2_font_6x12_tr);      //u8g2_font_unifont_t_extended);
  if(u8g2.userInterfaceMessage("", "Turn on WIFI ?",""," Yes \n No ") == 1) {
	 setup_wifi();
	 ntp_time_update();
	if(u8g2.userInterfaceMessage("", "Connect to MQTT ?",""," Yes \n No ") == 1 ){
		client.setServer(mqtt_server, atoi(mqtt_port));
		client.setCallback(callback);
		mqtt=true;
	}
  }

}
 
void loop()
{
home = true;
 readDS1307();
 digitalClockDisplay();
 //accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
 // show menu list
 if(u8g2.getMenuEvent()== U8X8_MSG_GPIO_MENU_SELECT)
	 menu();
 
 if (mqtt&&!client.connected()) 
     reconnect();
 
 if (mqtt&&!client.loop())
	 client.connect(device_id);
    // client.connect("ESP32_anhtu0310","pi","12345678");
}

void menu (){
	while (1) {
	uint8_t current_selection = 0;
	u8g2.setFont( u8g2_font_6x12_tr);
	current_selection = u8g2.userInterfaceSelectionList(
    "Menu",
    current_selection, 
    menu_strlist);
	
	switch(current_selection) {
		case 1:
		
		break;
		case 2:
		//ir();
		break;
		case 3:
		
		break;
		case 4:
		setting();
		break;
		case 5:
		break;
		case 6:
		return;
		break;
		
	}
  }
}	
void readDS1307()
{
        Wire.beginTransmission(DS1307);
        Wire.write((byte)0x00);
        Wire.endTransmission();
        Wire.requestFrom(DS1307, NumberOfFields);
        
        second = bcd2dec(Wire.read() & 0x7f);
        minute = bcd2dec(Wire.read() );
        hour   = bcd2dec(Wire.read() & 0x3f); // chế độ 24h.
        wday   = bcd2dec(Wire.read() );
        day    = bcd2dec(Wire.read() );
        month  = bcd2dec(Wire.read() );
        year   = bcd2dec(Wire.read() );
        year += 2000;    
}
/* Chuyển từ format BCD (Binary-Coded Decimal) sang Decimal */
int bcd2dec(byte num)
{
        return ((num/16 * 10) + (num % 16));
}
/* Chuyển từ Decimal sang BCD */
int dec2bcd(byte num)
{
        return ((num/10 * 16) + (num % 10));
}
 
void digitalClockDisplay(){
    // digital clock display of the time
    u8g2.clearBuffer();          // clear the internal memory
    u8g2.setFont(  u8g2_font_8x13B_tr);// u8g2_font_t0_11b_tr);//u8g2_font_6x12_tr);  //u8g2_font_ncenB08_tr); // choose a suitable font
    u8g2.setCursor(4,64);
    u8g2.print("T");
    u8g2.print(wday);
    u8g2.print("  ");
    u8g2.print(day);
    u8g2.print("/");
    u8g2.print(month);
    u8g2.print("/");
    u8g2.print(year);
    u8g2.setCursor(3,51);//47
    u8g2.setFont( u8g2_font_logisoso38_tn);// u8g2_font_freedoomr25_tn);
    printtime(&hour);
    u8g2.setCursor(75,51);
    printtime(&minute);
    u8g2.setCursor(56,21);
    u8g2.print(".");
    u8g2.setCursor(56,31);
     u8g2.print(".");
    u8g2.setCursor(64,39);
    u8g2.setFont( u8g2_font_logisoso16_tn);
    u8g2.setCursor(54,50);
    printtime(&second);
   
   
   // u8g2.setFont(     //u8g2_font_freedoomr10_tu);
	//u8g2.print(":");
    //printtime(&second);
//	if(wifi){
		//u8g2.setFont(u8g2_font_open_iconic_www_1x_t);
		//u8g2.drawGlyph(3,64,73);
	//}
   //u8g2.sendBuffer();    
	/*u8g2.setCursor(0, 64);
	u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.print(gx); u8g2.print(" ");
        u8g2.print(gy); u8g2.print(" ");
        u8g2.print(gz); u8g2.print(" ");
      */  
  u8g2.sendBuffer(); 
}
 
void printtime(int *value) {
if (*value <10) {
  u8g2.print("0"); u8g2.print(*value);}
  else u8g2.print(*value); }

void setting(){
	while (1){
	uint8_t current_selection = 0;
	//u8g2.setFont(u8g2_font_6x12_tr);
	current_selection = u8g2.userInterfaceSelectionList(
    "Setting",
    current_selection, 
    settiing_strlist);
	
	switch(current_selection) {
		case 1:
		if(wifi&&u8g2.userInterfaceMessage("", "Turn off WIFI ?",""," Yes \n No ") == 1) 
		  turn_off_wifi();
		else if (!wifi&&u8g2.userInterfaceMessage("", "Turn on WIFI ?",""," Yes \n No ") == 1)
		  setup_wifi();
		break;
		
		case 2:
		if(u8g2.userInterfaceMessage("", "Update Internet time?",""," Yes \n No ") == 1) 
		ntp_time_update();
		break;
		case 3:
		return;
		break;
		
	}
  }
}		

void setTime(byte hr, byte min, byte sec, byte wd, byte d, byte mth, byte yr)
{
        Wire.beginTransmission(DS1307);
        Wire.write(byte(0x00)); // đặt lại pointer
        Wire.write(dec2bcd(sec));
        Wire.write(dec2bcd(min));
        Wire.write(dec2bcd(hr));
        Wire.write(dec2bcd(wd)); // day of week: Sunday = 1, Saturday = 7
        Wire.write(dec2bcd(d)); 
        Wire.write(dec2bcd(mth));
        Wire.write(dec2bcd(yr));
        Wire.endTransmission();
} 
void ntp_time_update(){
	if (!wifi&&u8g2.userInterfaceMessage("You have to turn ","on Wifi first", "Turn on WIFI ?"," Yes \n No ") == 1)
	  setup_wifi();
	else if (!wifi)
	  return;
  
	configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
	struct tm timeinfo;
	if(!getLocalTime(&timeinfo))
		u8g2.userInterfaceMessage("Failed to update", " Internet time ?","","OK ");
	else { 
		setTime(timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec, timeinfo.tm_wday+1,timeinfo.tm_mday,timeinfo.tm_mon + 1,timeinfo.tm_year -100);
		u8g2.userInterfaceMessage("", "Updated time ","","OK ");
		}
}
	
	
