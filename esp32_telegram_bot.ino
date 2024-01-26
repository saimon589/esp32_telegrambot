// Libraries import
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>

// assing the wifi name and password.
// refill your wifi name and password.
const char* ssid = "";
const char* password = "";

// define the telegram token and chat id.
// refill your telegram token and chat id.
#define BOT_TOKEN ""
#define CHAT_ID ""

#define DHTPIN 26
#define DHTTYPE DHT11

// define the instances
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
DHT dht(DHTPIN, DHTTYPE);

const int motionSensor = 27;
const int ledPin = 2;
bool motionDetected = false;
int botReqDelay = 1000;
unsigned long lastimeBotRun;

void IRAM_ATTR detectsMovements(){
  motionDetected = true;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  dht.begin();
  pinMode(motionSensor, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovements, RISING);
  wifi_connect();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(millis()>botReqDelay+lastimeBotRun){
    int numNewMessages = bot.getUpdates(bot.last_message_received+1);
    while(numNewMessages){
      handleNewMessages(numNewMessages);
      readDHTSensor(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received+1);
    }
    lastimeBotRun = millis();
  }
}

// function for wifi connecting.
void wifi_connect(){
  Serial.print("Wifi conneting to : ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);

  }
  Serial.println("");
  Serial.println("Wifi is conneted.");
  Serial.print("IP Address : ");
  Serial.println(WiFi.localIP());
  bot.sendMessage(CHAT_ID, "Bot startup!!!");
}

// function to send the motion sensor data to telegram.
void motionState(){
  if(motionDetected){
    bot.sendMessage(CHAT_ID, "Motion was detected!!");
    Serial.println("Motion was detected!!");
    motionDetected = false;
  }
}

// function to read the data from DHT11 sensor.
void readDHTSensor(int numNewMessages){
  delay(2000);
  float temp = dht.readTemperature();
  float humi = dht.readHumidity();
  String temp_str = String(temp);
  String humi_str = String(humi);
  if(isnan(temp) || isnan(humi)){
    Serial.print("Faild to read from dht.");
    return;
  }
  for(int i=0; i<numNewMessages; i++){
    String chat_id = String(bot.messages[i].chat_id);
    if(chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorize user!!!");
      continue;
    }
    String text = bot.messages[i].text;
    if(text=="temp"){
      bot.sendMessage(chat_id, "Current Temp is "+temp_str);
    }else if(text=="humi"){
      bot.sendMessage(chat_id, "Current Humi is "+humi_str);
    }
  }
}
// funtion to read and sent the messages.
void handleNewMessages(int numNewMessages){
  for(int i=0; i<numNewMessages; i++){
    String chat_id = String(bot.messages[i].chat_id);
    if(chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorize user!!!");
      continue;
    }
    String text = bot.messages[i].text;
    Serial.print(text);
    String from_name = bot.messages[i].from_name;
    if(text == "/start"){
      String welcome = "Welcome" + from_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "led on to turn GPIO ON \n";
      welcome += "led off to turn GPIO OFF \n";
      welcome += "temp to get Temperature \n";
      welcome += "humi to get Humidity \n";
      welcome += "state to request current GPIO state \n";
      bot.sendMessage(chat_id, welcome, "");
    }
    if(text == "led on"){
      bot.sendMessage(chat_id, "Relay is ON");
      digitalWrite(ledPin, HIGH);
    }
    else if(text=="led off"){
      bot.sendMessage(chat_id, "Relay is OFF");
      digitalWrite(ledPin, LOW);
    }else if(text == "state"){
      if(digitalRead(ledPin)){
        bot.sendMessage(chat_id, "Currnet state is 'ON'");
      }else{
        bot.sendMessage(chat_id, "Currnet state is 'OFF'");
      }
    }
  }
}

