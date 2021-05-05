#include <Preferences.h> //https://github.com/espressif/arduino-esp32/blob/master/LICENSE.md
#include <WiFiManager.h>//https://github.com/tzapu/WiFiManager/blob/master/LICENSE
#include <WiFi.h> //https://github.com/espressif/arduino-esp32/blob/master/LICENSE.md
#include <AsyncTCP.h> //https://github.com/me-no-dev/AsyncTCP/blob/master/LICENSE
#include <ESPAsyncWebServer.h> //https://github.com/me-no-dev/ESPAsyncWebServer/blob/master/src/ESPAsyncWebServer.h#L7-L19
#define Com_PORT 1000

//zmienne dla działania modułu
#define motor_start 25 // pin "zasilania"
#define  motor_dir 32//pin kierunku obrotu
bool up,go;//zmienne sterujące
uint32_t intUP,intDOWN;//domyślne czasy pracy urządzenia
uint64_t multiplier=1000;// mnożnik do millisekund

uint32_t tmp;//zmienne do obsługi czasu pracy silnika
uint32_t timeTo;

Preferences config_var;
AsyncWebServer Motor_Control(Com_PORT);
IPAddress local_ip(192,168,1,196);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);


void motorStop(){// funkcja dla zatrzymania silnika
  digitalWrite(motor_start,HIGH);
  digitalWrite(motor_dir,HIGH);
}


void motorGO(bool state){
  motorStop(); 
  if(state==true){
    digitalWrite(motor_dir,LOW);//zmiana kierunku
  }else {
    digitalWrite(motor_dir,HIGH);// zmiana kierunku
  }
   delay(300);
    digitalWrite(motor_start,LOW);//uruchomienie silnika
}
void save_timer(String message,int Value,bool timerSave){
     
      int tmp=message.toInt();
      Value=unsigned(tmp);
      if(timerSave==true){
        intUP=Value;
        config_var.putUInt("timeTo",Value);

      } else{
        intDOWN=Value;
        config_var.putUInt("timeDo",Value);
     
      }
     Serial.println(intUP);
     Serial.println(intDOWN);
  }
void setup() {
  pinMode(motor_dir, OUTPUT); // ustawienie pinów dla obsługi silnika
  pinMode(motor_start, OUTPUT); 
  digitalWrite(motor_dir,HIGH);
  digitalWrite(motor_start ,HIGH);
  Serial.begin(115200);
 
 WiFiManager manager;// manadżer sieci Wi-Fi
 manager.setClass("invert");
 
  //manager.resetSettings();
  manager.setAPStaticIPConfig(local_ip,gateway,subnet);
  manager.setSTAStaticIPConfig(local_ip,gateway,subnet);
  bool connection_result = manager.autoConnect("Moduł_wykonawczy","Pr0j3kt1");
  
  if(!connection_result){
    Serial.println("brak połączenia");
 }else{
    Serial.print("połączono z: ");
    Serial.println(WiFi.SSID());
    Serial.print("adres: ");
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.gatewayIP());
    Serial.println(WiFi.subnetMask());
    config_var.begin("timers",false);// uruchumienie preferencji modułu
    intUP=config_var.getUInt("timeTo",30);//odczyt danych konfiguracyjnych, w przypadku braku wstaw domyślne 30sek;
    intDOWN=config_var.getUInt("timeDo",30);
    
    }
    
    
    Motor_Control.on("/configTime",HTTP_GET ,[](AsyncWebServerRequest *request){
       request->send(200, "text/plain", "zmiana ustawień..."); 
      
      if (request->hasParam("Uptime")){
        AsyncWebParameter* p1 = request->getParam("Uptime");
        String placeholder=p1->value();
        save_timer(placeholder,intUP,false);
      }
      if (request->hasParam("Dotime")){
        AsyncWebParameter* p2 = request->getParam("Dotime");
        String placeholder=p2->value();    
        save_timer(placeholder,intDOWN,true);
      }
     
     
    });
    Motor_Control.on("/s",HTTP_GET ,[](AsyncWebServerRequest *request){
      request->send(200, "text/plain", "stop"); 
      motorStop();
      Serial.println("stop");
    });
    Motor_Control.on("/d",HTTP_GET ,[](AsyncWebServerRequest *request){ 
      request->send(200, "text/plain", "opuszczanie..."); 
      up=false;
      Serial.println("dół");
      motorGO(up);
      timeTo=intDOWN*multiplier;
      tmp=millis();
      go=true;
    });
    Motor_Control.on("/u",HTTP_GET ,[](AsyncWebServerRequest *request){
      request->send(200, "text/plain", "podnoszenie...");
      up=true;
      Serial.println("góra");
      motorGO(up);
      timeTo=intUP*multiplier;
      tmp=millis();
      go=true;
    });
    Motor_Control.begin();
}

void loop() {
 
   if(go==true){//uruchomienie odliczania
  
   if(millis()-tmp>=timeTo){//sprawdzenie czy czas minął
      motorStop();//jeśli tak to stop
      tmp=millis();// zapisz aktualny czas
      go=false;//wyjdź z instrukcji warunkowej do ponownej zmiany kierunku
   }
  }
}
