#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "config.h"
#include "allowed.h"
//Pins to use
const int PinRele = D4;
const int PinLed = 2;

int long lt = 0;
String ipAddress = "";



//SSL CLient
WiFiClientSecure client;




//Connect to bot
UniversalTelegramBot bot(BOT_TOKEN, client);


int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done
bool Start = false;

WiFiClient clientUbi;

void setup() {
  //Configure digital Pin
  pinMode(PinRele, OUTPUT);
  digitalWrite(PinRele, HIGH);
  //pinMode(PinLed, OUTPUT);
  //Serial and WIFI connection
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  //SmasrtConfig
  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (cnt++ >= 10) {
      WiFi.beginSmartConfig();
      while (1) {
        delay(1000);
        if (WiFi.smartConfigDone()) {
          Serial.println("SmartConfig Success");
          break;
        }
      }
    }
  }
  WiFi.setAutoReconnect(true);
  Serial.println(F("WiFi connected"));
  IPAddress ip = WiFi.localIP();
  Serial.println(F("IP address: "));
  Serial.println(ip);
  ipAddress = ip.toString();
  flushOldMessages();
  sendConnectionReport();
}

void loop() {

  //Check new messages
  if (millis() > Bot_lasttime + Bot_mtbs)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    Bot_lasttime = millis();
  }


}

//Activate door switch
void openDoor() {
  digitalWrite(PinRele, LOW);
  delay(300);
  digitalWrite(PinRele, HIGH);
}

//Message to report connection
void sendConnectionReport() {
  String message = "IP:  ";
  message.concat(ipAddress);
  message.concat("\n");
  if (bot.sendMessage(ADMIN_ID, message, "Markdown")) { //Revisa si se envió el mensaje
    Serial.println("TELEGRAM Successfully sent");
  }
}

//Check if allowed ID
int checkAllowed(String &userID) {
  return allowedID.count(userID);
}
//Manage new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    String from_id = bot.messages[i].from_id;

    if (from_name == "") from_name = "Guest";

    if (text == "/open") { //Door open case
      if (checkAllowed(from_id)) {
        bot.sendMessage(chat_id, "Abriendo puerta", "");
        openDoor();
      }
      else {
        bot.sendMessage(chat_id, "No está en la lista de usuarios con acceso", "");
      }
    }
    if (text == "/help") { //Message with allowed commands
      String welcome = "Bot to open the door\n";
      welcome += "/open : Opens the door if the user has access";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}

//Remove old messages
void flushOldMessages() {
  int oldMessages = bot.getUpdates(bot.last_message_received + 1);
  while (oldMessages) {
    oldMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}
