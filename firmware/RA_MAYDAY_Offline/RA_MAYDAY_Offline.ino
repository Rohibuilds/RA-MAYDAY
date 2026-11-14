/* RA MAYDAY offline rescue centre — classic ESP32 receiver only.
   RF24 by TMRh20 required. Website/assets embedded; no filesystem upload.
   Current sender packet is unchanged. No Pi, AWS, router or internet needed.
*/
#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_timer.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "Dashboard.h"
#include "CommandBackground.h"

constexpr char AP_NAME[] = "RA-MAYDAY";
constexpr char AP_PASSWORD[] = "Mayday2026"; // Change before field use (8+ characters).
constexpr uint8_t NRF_CE=25, NRF_CSN=26, NRF_SCK=27, NRF_MISO=19, NRF_MOSI=23;
constexpr uint8_t RADIO_CHANNEL=76;
const uint8_t RADIO_ADDRESS[6]="MAY01";
struct __attribute__((packed)) SurvivorPacket {
  uint32_t sequence;
  float latitude, longitude, temperature, humidity, pressure;
  uint8_t flags;
  char survivorID[7];
};
static_assert(sizeof(SurvivorPacket)==32,"Sender packet must be exactly 32 bytes");
constexpr size_t MAX_SURVIVORS=8, HISTORY_SIZE=120, EVENT_SIZE=32;
struct Sample { SurvivorPacket p; uint64_t receivedAt; bool edge; };
struct Node { bool used; Sample latest; };
struct Event { char id[7]; uint64_t at; bool active; uint32_t sequence; };
struct SharedData {
  Node nodes[MAX_SURVIVORS]{};
  Sample history[HISTORY_SIZE]{};
  Event events[EVENT_SIZE]{};
  size_t head=0, count=0, eventHead=0, eventCount=0;
  uint32_t received=0, rejected=0, capacityDrops=0;
  bool radioReady=false;
};
// Explicit prototypes keep Arduino preprocessing compatible with custom types.
bool validID(const SurvivorPacket& p);
void storePacket(const SurvivorPacket& p);
void sampleJSON(String& out,const Sample& s);
SharedData liveData, snapshot;
SemaphoreHandle_t dataMutex;
RF24 radio(NRF_CE,NRF_CSN);
WebServer server(80);
uint64_t uptimeMs(){return (uint64_t)esp_timer_get_time()/1000ULL;}
bool initializeRadio(){
  if(!radio.begin(&SPI)||!radio.isChipConnected()) return false;
  // The PA+LNA receiver must send ACKs back across the same field link.
  // HIGH is a conservative default; use LOW for very-close bench tests.
  // A clean, adequately rated 3.3 V supply is mandatory for PA/LNA modules.
  radio.setPALevel(RF24_PA_HIGH);
  radio.setChannel(RADIO_CHANNEL);
  if(!radio.setDataRate(RF24_250KBPS)) return false;
  radio.setAddressWidth(5); radio.setCRCLength(RF24_CRC_16);
  radio.setAutoAck(true); radio.disableDynamicPayloads();
  radio.setPayloadSize(sizeof(SurvivorPacket));
  radio.openReadingPipe(1,RADIO_ADDRESS); radio.startListening();
  return true;
}
bool validID(const SurvivorPacket& p){
  bool end=false;
  if(!p.survivorID[0] || p.survivorID[6]!=0 || (p.flags & ~7)) return false;
  for(int i=0;i<6;i++){
    const char c=p.survivorID[i];
    if(!c){end=true;continue;}
    if(end || !((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='-'||c=='_')) return false;
  }
  return true;
}
void storePacket(const SurvivorPacket& p){
  xSemaphoreTake(dataMutex,portMAX_DELAY);
  if(!validID(p)){liveData.rejected++;xSemaphoreGive(dataMutex);return;}
  int index=-1, empty=-1;
  for(size_t i=0;i<MAX_SURVIVORS;i++){
    if(liveData.nodes[i].used && strcmp(liveData.nodes[i].latest.p.survivorID,p.survivorID)==0) index=i;
    if(!liveData.nodes[i].used && empty<0) empty=i;
  }
  if(index<0) index=empty;
  if(index<0){liveData.capacityDrops++;xSemaphoreGive(dataMutex);return;}
  Node& node=liveData.nodes[index];
  const bool sos=(p.flags&2)!=0;
  const bool edge=node.used ? sos!=bool(node.latest.p.flags&2) : sos;
  Sample sample{}; sample.p=p;sample.receivedAt=uptimeMs();sample.edge=edge;
  node.used=true;node.latest=sample;
  liveData.history[liveData.head]=sample;
  liveData.head=(liveData.head+1)%HISTORY_SIZE;
  if(liveData.count<HISTORY_SIZE)liveData.count++;
  liveData.received++;
  if(edge){
    Event& e=liveData.events[liveData.eventHead];
    memcpy(e.id,p.survivorID,7);e.at=sample.receivedAt;e.active=sos;e.sequence=p.sequence;
    liveData.eventHead=(liveData.eventHead+1)%EVENT_SIZE;
    if(liveData.eventCount<EVENT_SIZE)liveData.eventCount++;
  }
  xSemaphoreGive(dataMutex);
}
void radioWorker(void*){
  bool ready=false; uint32_t attempt=0,check=0;
  for(;;){
    uint32_t now=millis();
    if(!ready && (attempt==0 || now-attempt>=3000)){
      attempt=now;ready=initializeRadio();check=now;
      Serial.println(ready?"[NRF24] READY | MAY01 CH76 250kbps":"[NRF24] FAILED | retrying wiring/power check");
      xSemaphoreTake(dataMutex,portMAX_DELAY);liveData.radioReady=ready;xSemaphoreGive(dataMutex);
    }
    if(ready){
      for(int n=0;n<8&&radio.available();n++){
        SurvivorPacket p{};radio.read(&p,sizeof(p));storePacket(p);
      }
      if(now-check>=5000){
        check=now;ready=radio.isChipConnected();
        xSemaphoreTake(dataMutex,portMAX_DELAY);liveData.radioReady=ready;xSemaphoreGive(dataMutex);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(2));
  }
}
void number(String& out,float v,int digits=2){out+=isfinite(v)?String(v,digits):String("null");}
void sampleJSON(String& out,const Sample& s){
  const SurvivorPacket& p=s.p;
  bool gps=(p.flags&1)&&isfinite(p.latitude)&&isfinite(p.longitude)&&fabsf(p.latitude)<=90&&fabsf(p.longitude)<=180;
  out+="{\"survivorID\":\"";out+=p.survivorID;out+="\",\"sequence\":";out+=String(p.sequence);
  out+=",\"capturedAt\":";out+=String((double)s.receivedAt,0);
  out+=",\"gpsValid\":";out+=gps?"true":"false";
  out+=",\"latitude\":";if(gps)number(out,p.latitude,6);else out+="null";
  out+=",\"longitude\":";if(gps)number(out,p.longitude,6);else out+="null";
  out+=",\"temperature\":";number(out,p.temperature);
  out+=",\"humidity\":";number(out,p.humidity);
  out+=",\"pressure\":";number(out,p.pressure);
  out+=",\"sos\":";out+=(p.flags&2)?"true":"false";
  out+=",\"sensorOK\":";out+=(p.flags&4)?"true":"false";
  out+=",\"sosEdge\":";out+=s.edge?"true":"false";out+='}';
}
void telemetry(){
  xSemaphoreTake(dataMutex,portMAX_DELAY);snapshot=liveData;xSemaphoreGive(dataMutex);
  String out; if(!out.reserve(60000)){server.send(503,"text/plain","Memory busy; retry");return;}
  out="{\"serverTime\":";out+=String((double)uptimeMs(),0);
  out+=",\"radioReady\":";out+=snapshot.radioReady?"true":"false";
  out+=",\"clients\":";out+=String(WiFi.softAPgetStationNum());
  out+=",\"heap\":";out+=String(ESP.getFreeHeap());
  out+=",\"received\":";out+=String(snapshot.received);
  out+=",\"rejected\":";out+=String(snapshot.rejected);
  out+=",\"capacityDrops\":";out+=String(snapshot.capacityDrops);
  out+=",\"survivors\":[";
  bool comma=false;for(const Node& n:snapshot.nodes)if(n.used){if(comma)out+=',';sampleJSON(out,n.latest);comma=true;}
  out+="],\"samples\":[";
  for(size_t i=0;i<snapshot.count;i++){if(i)out+=',';sampleJSON(out,snapshot.history[(snapshot.head+HISTORY_SIZE-1-i)%HISTORY_SIZE]);}
  out+="],\"events\":[";
  for(size_t i=0;i<snapshot.eventCount;i++){
    if(i)out+=',';const Event& e=snapshot.events[(snapshot.eventHead+EVENT_SIZE-1-i)%EVENT_SIZE];
    out+="{\"id\":\"";out+=e.id;out+="\",\"at\":";out+=String((double)e.at,0);
    out+=",\"active\":";out+=e.active?"true":"false";out+='}';
  }
  out+="]}";server.sendHeader("Cache-Control","no-store");server.send(200,"application/json",out);
}
void setup(){
  Serial.begin(115200);
  dataMutex=xSemaphoreCreateMutex();
  if(!dataMutex){Serial.println("[FATAL] mutex allocation failed");while(true)delay(1000);}
  SPI.begin(NRF_SCK,NRF_MISO,NRF_MOSI,NRF_CSN);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IPAddress(192,168,4,1),IPAddress(192,168,4,1),IPAddress(255,255,255,0));
  if(!WiFi.softAP(AP_NAME,AP_PASSWORD,1,false,4)){
    Serial.println("[FATAL] Hotspot failed; check password length");while(true)delay(1000);
  }
  server.on("/",HTTP_GET,[]{server.send_P(200,"text/html; charset=utf-8",DASHBOARD_HTML);});
  server.on("/command-grid.svg",HTTP_GET,[]{server.send_P(200,"image/svg+xml",COMMAND_BACKGROUND);});
  server.on("/api/telemetry",HTTP_GET,telemetry);
  server.onNotFound([]{server.send(404,"text/plain","Open http://192.168.4.1/");});
  server.begin();
  if(xTaskCreate(radioWorker,"radio",4096,nullptr,2,nullptr)!=pdPASS)Serial.println("[FATAL] Radio task allocation failed");
  Serial.println("\nRA MAYDAY | OFFLINE RESCUE CENTRE\n[WIFI] RA-MAYDAY\n[WEB] http://192.168.4.1\n[READY] Dashboard available even without radio data");
}
void loop(){server.handleClient();delay(2);}
