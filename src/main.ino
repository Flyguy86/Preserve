#include "IRTransmitter.h"
#include "Enviornment.h"
#include "dht.h"
#include "RCswitch.h"

#define IR_PIN D6
#define LED_PIN D7
#define DHTPIN A7
#define DHTTYPE DHT11  // sensor type.  Options: [DHT11, DHT22, DHT21, AM2301]

unsigned int data[227]{3500,1650,500,1250,500,1250
,500,400,500,450,450,450
,450,1300,450,450,500,400
,500,1250,500,1250,500,400
,500,1250,500,450,450,450
,450,1300,450,1300,450,450
,500,1250,450,1300,450,450
,500,400,500,1250,500,450
,450,450,450,1300,450,450
,500,400,500,450,450,450
,450,450,450,450,500,450
,450,450,450,450,450,450
,450,450,500,450,450,450
,450,1300,450,450,450,450
,450,500,450,1300,450,450
,450,450,450,1300,450,450
,450,450,450,500,450,1300
,450,450,450,450,450,450
,450,500,400,1350,400,500
,450,450,450,450,450,500
,400,1350,400,500,450,450
,450,450,450,450,450,500
,400,1350,400,500,450,1300
,450,1300,450,1300,450,450
,450,450,450,500,400,500
,450,450,450,450,450,450
,450,500,400,500,450,450
,450,450,450,500,400,500
,450,450,450,450,450,450
,450,500,400,500,450,450
,450,450,450,500,400,500
,450,450,450,450,450,450
,450,500,450,1300,450,450
,450,450,450,450,450,1300
,450,1300,450,500,400,500
,450,450,450,1300,450,1300
,450,1300,450,450,450,1300
,450,450,450,500,450};

IRTransmitter transmitter(IR_PIN, LED_PIN);
ENV env(80, 30, 20, 10);
// DHT dht(DHTPIN, DHTTYPE);
uint32_t lasttime = 0;

void setup() {

    Serial.begin(9600);

    Particle.connect();
    waitFor(Particle.connected, 30000);

    Particle.function("send", send);
    Particle.function("Env", Envmonitor);
    Particle.function("ACOn", acon);
    Particle.function("Dryair", dry);
    Particle.function("Off", off);
    Particle.function("Fan on", fanon);
    Particle.function("HeaterOn", HeatOn);
    Particle.function("Status", status);
    Particle.function("RF-test", RFtest);
}

void loop() {

if(  millis() - lasttime > 6000 ){


  //6 second loop
  lasttime = millis();
}

env.loop();
}

int Envmonitor(String a){
    int t = parse_args_int(a, "t");
    int h = parse_args_int(a, "h");
    int c = parse_args_int(a, "c");
    int ct = parse_args_int(a, "ct");
    int db = parse_args_int(a, "db");

    env.updateTargets(t, h, c, ct, db);
    return 1;
}
int RFtest(String a ){
    RCSwitch mySwitch = RCSwitch();
    mySwitch.enableTransmit(TX);
    // Optional set number of transmission repetitions.
    mySwitch.setRepeatTransmit(5);
    mySwitch.switchOn("11111", "00010");
    delay(1000);
    mySwitch.switchOn("11111", "00010");
    delay(1000);
 return 1;
}
int dry(String a){

    transmitter.Transmit(transmitter.DryOn, sizeof(transmitter.DryOn) / sizeof( transmitter.DryOn[0] ));
    return 1;
}
int off(String a){

    transmitter.Transmit(transmitter.Off, sizeof(transmitter.Off) / sizeof( transmitter.Off[0] ));
    return 1;
}
int acon(String a ){

    transmitter.Transmit(transmitter.ACon, sizeof(transmitter.ACon) / sizeof( transmitter.ACon[0] ));
 //    Particle.publish("Sending IR1");
 return 1;
}
int fanon(String a ){

    transmitter.Transmit(transmitter.FanOn, sizeof(transmitter.FanOn) / sizeof( transmitter.FanOn[0] ));
 //    Particle.publish("Sending IR1");
 return 1;
}
int send(String a ){

    transmitter.Transmit(data, sizeof(data) / sizeof(data[0]));
 return 1;
}
int HeatOn(String a ){

    transmitter.Transmit(transmitter.HeaterOn, sizeof(transmitter.HeaterOn) / sizeof( transmitter.HeaterOn[0] ));
 return 1;
}
int status(String a){
    env.Publish("Status");
    return 1;
}

int parse_args_int(String &args, String q){
    if ( !args.length() || !q.length() || args.length() < q.length() )
    return -1;

    if ( q.substring(q.length() - 1, 1) != ":" ) q = q + ":";
    int ql = q.length();

    int r1 = args.indexOf(q);

    if ( r1 != -1 ) {
        int r2 = args.indexOf(",", r1+ql);
        if ( r2 == -1 ) r2 = args.length();
        return args.substring(r1+ql,r2).toInt();
    }
 return -1;
 }
