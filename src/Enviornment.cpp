#include "application.h"
#include "Enviornment.h"
#include "dht.h"
#include "IRTransmitter.h"
#include "RCswitch.h"
IRTransmitter transmit(IR_PIN, LED_PIN);

ENV::ENV( uint8_t tt, uint8_t ht, uint16_t ct, uint16_t countThresh) {
  _targetTemperature = tt;
  _targetHumidity = ht;
  _cycletime = ct;
  _ToutofrangeCounter = 0;
  _HoutofrangeCounter = 0;
  _Tgap = 0;
  _Hgap = 0;
  _ACon = false;
  _HeatOn = false;
  _countThresh = countThresh;
  _debug = 0;
  _alarmThreshold = 200;
  _escalationThreshold = 100;
}
void ENV::begin(void){
    readAllSensors();
    Endtime();
}
void ENV::updateTargets(uint8_t tt, uint8_t ht, uint16_t ct, uint16_t countThresh, uint8_t debug){
  _targetTemperature = tt;
  _targetHumidity = ht;
  _cycletime = ct;
  _countThresh = countThresh;
  _debug = debug;
}
void ENV::Endtime(void){
    _lasttime = millis();
}
void ENV::loop(void){

   if(  millis() - _lasttime > CycleTimetoMillis() ) {
     readAllSensors();
     _action = TargetValueCompare(i_t_h, istats);
     ActionDescider(_action, _countThresh);

    if(_debug == 1){
      Publish("debug");
    }

     Endtime();
   }
}
void ENV::Publish(String a){
  String response;
  response = "return:" + String(_action) +
  " :Current T(f):"   + String( i_t_h[1] ) +
  " :Target Temp(f):" + String( _targetTemperature ) +
  " :Humid:"          + String( i_t_h[2]) +
  " :Target RH%:"     + String( _targetHumidity ) +
  " :Cycletime (sec):"+ String(_cycletime) +
  " :Action:"         + String(_action) +
  " :H out range cnt:"+ String(istats[3]) +
  " :T out range cnt:"+ String(istats[4]) +
  " :T Difference:"   + String(istats[1]) +
  " :H Difference:"   + String(istats[2]) +
  " DHT In:"          + String( i_t_h[1] ) + " - " + String( i_t_h[2] ) +
  " DHT Out:"         + String( o_t_h[1] ) + " - " + String( o_t_h[2] ) +
  " DHT Air:"         + String( a_t_h[1] ) + " - " + String( a_t_h[2] );
  Particle.publish(a, response);
}
void ENV::ActionDescider(uint8_t comparator, uint16_t _countThresh){

  if(istats[3] > _alarmThreshold){
    Publish("Above Max Thresh");
  }


  if(comparator == 1 || comparator == 2 || comparator == 3 ){
   if( istats[3] > _countThresh ){
    if(_HeatOn){
      EnvOff();
      Publish("High Thresh - Heat Off");

    }else{
      ACOn();
      Publish("High Temp - AC On");

       if( istats[3] > _escalationThreshold )
           Publish("Escalation");
    }
   }
  }

  if(comparator == 4 || comparator == 5 || comparator == 6){
    EnvOff();
  }

  if(comparator == 7 || comparator == 8 || comparator == 9 ){
    if( istats[3] > _countThresh ){
     if(_ACon){
       EnvOff();
       Publish("Low Thresh - AC Off");
     }else{
       HeatOn();
       Publish("Low Temp - Heat On");

       if( istats[3] > _escalationThreshold )
             Publish("Escalation");
     }
    }
  }

}
uint8_t ENV::TargetValueCompare(uint8_t *p, int *f){
 /*Return values
  1    tt<t th<h   AC on,  Exhaust fan Off
  2    tt<t th>h   AC on,  Exhaust fan On,  (Outside air check)  IF warm No Heater,  If cool Heater)
  3    th<t th=h   AC on,  Exhaust fan off
  4    tt=t th<h   Heater/AC off, exhaust fan off
  5    tt=t th>h   Heater/AC off, exhaust fan on (Outside air check)
  6    tt=t th=h   Heater/AC/Exhaust Off
  7    th>t th=h   Heater on,  Exhaust fan off
  8    tt>t th<h   Heater on,  Exhaust fan off
  9    tt>t th>h   Heater on,  Exhaust fan On
  10   Error
  */
  /* i_t_h arrays  *p
  p[1] = Temperature reading
  p[2] = Humidity reading
  p[3] = Temperature Target
  p[4] = Humidity Target
     istats arrays
  f[1]Temperature Difference from 0 target
  f[2]Humidity Difference from 0 target
  f[3] Temperature Counts since 0 or change direction
  f[4] Humidumity Counts since 0 or change direction
  *(f + 1)
  */
  // use to monitor the direction of change
  _Tgap = *(p + 1) - _targetTemperature;
  _Hgap = *(p + 2) - _targetHumidity;
  countCrossedtarget();
  *(f + 1) = _Tgap;
  *(f + 2) = _Hgap;

  // *(p +1) points to i_t_h arrays
  if( *(p + 1) < 120 && *(p + 1) > 0 ){
    if ( _targetTemperature < *(p + 1) && _targetHumidity  < *(p + 2)  ){
      *(f + 4) = _HoutofrangeCounter++;
      *(f + 3) = _ToutofrangeCounter++;
      return 1;
    }else if ( _targetTemperature < *(p + 1) && _targetHumidity  > *(p + 2)  ){
      *(f + 4) = _HoutofrangeCounter++;
      *(f + 3) = _ToutofrangeCounter++;
      return 2;
    }else if ( _targetTemperature < *(p + 1) && _targetHumidity  == *(p + 2)  ){
      *(f + 4) = _HoutofrangeCounter = 0;
      *(f + 3) = _ToutofrangeCounter++;
      return 3;
    }else if ( _targetTemperature == *(p + 1) && _targetHumidity  < *(p + 2)  ){
      *(f + 4) = _HoutofrangeCounter++;
      *(f + 3) = _ToutofrangeCounter = 0;
      return 4;
    }else if ( _targetTemperature == *(p + 1) && _targetHumidity  > *(p + 2)  ){
      *(f + 4) = _HoutofrangeCounter++;
      *(f + 3) = _ToutofrangeCounter = 0;
      return 5;
    }else if ( _targetTemperature == *(p + 1) && _targetHumidity  == *(p + 2)  ){
      *(f + 4) = _HoutofrangeCounter = 0;
      *(f + 3) = _ToutofrangeCounter = 0;
      return 6;
    }else if ( _targetTemperature > *(p + 1) && _targetHumidity  < *(p + 2)  ){
      *(f + 4) = _HoutofrangeCounter++;
      *(f + 3) = _ToutofrangeCounter++;
      return 7;
    }else if ( _targetTemperature > *(p + 1) && _targetHumidity  > *(p + 2)  ){
      *(f + 4) = _HoutofrangeCounter++;
      *(f + 3) = _ToutofrangeCounter++;
      return 8;
    }else if ( _targetTemperature > *(p + 1) && _targetHumidity  == *(p + 2)  ){
      *(f + 4) = _HoutofrangeCounter++;
      *(f + 3) = _ToutofrangeCounter++;
      return 9;
    }else{
      return 10;
    }

  }else{
      return 11;
  }

}
void ENV::countCrossedtarget(void){
 // Determind Change of direction, in difference from set target point.
 // On change of direction in gap from target,  turn Off the Env Controls
 // Reset the Out of Range counters

  // Determind direction of change.  _Tlastdir >0  or _Tlastdir < 0

  if(_Tlastdir > 0){   // Determin direction of change
     // Postive correction
     if(_Tlastdir < _Tgap){  // direction of change oppostite current change value
       EnvOff();
     }
  else if(_Tlastdir < 0){
    // Negative correction
    if(_Tlastdir > _Tgap){
      EnvOff();
    }
  }
 _Tlastdir = _Tgap;
 }
}
void ENV::EnvOff(void){
  _ACon = false;
  _HeatOn = false;
  _ToutofrangeCounter = 0;
  _HoutofrangeCounter = 0;
 //  Particle.publish("Env-Off", "True");

  transmit.Transmit(transmit.Off, sizeof(transmit.Off) / sizeof( transmit.Off[0] ) );
}
void ENV::HeatOn(void){
  _HeatOn=true;
 //  Particle.publish("Heater-On", "True");

  transmit.Transmit(transmit.HeaterOn, sizeof(transmit.HeaterOn) / sizeof( transmit.HeaterOn[0] ) );
}
void ENV::ACOn(void){
  _ACon = true;
 //  Particle.publish("AC-On", "True");

  transmit.Transmit(transmit.ACon, sizeof(transmit.ACon) / sizeof( transmit.ACon[0] ) );
}
void ENV::DryOn(void){
  _DryOn = true;
 //  Particle.publish("AC-On", "True");

  transmit.Transmit(transmit.DryOn, sizeof(transmit.DryOn) / sizeof( transmit.DryOn[0] ) );
}
void ENV::FanOn(void){
  _FanOn = true;
 //  Particle.publish("AC-On", "True");

  transmit.Transmit(transmit.FanOn, sizeof(transmit.FanOn) / sizeof( transmit.FanOn[0] ) );
}
uint32_t ENV::CycleTimetoMillis(void){
  return _cycletime * 1000;
}
void ENV::readAllSensors(void){
  DHT_Check(DHTPIN, DHTTYPE, i_t_h);
  DHT_Check(DHTPIN3, DHTTYPE, o_t_h);
  DHT_Check(DHTPIN2, DHTTYPE, a_t_h);

}
uint8_t ENV::DHT_Check(uint8_t dhtPin, uint8_t dhtType, uint8_t *p ){
  DHT dht(dhtPin, dhtType);
  dht.begin();
  int t = dht.readTemperature();

  if(t == 9999){
    return 256;
  }

  *(p + 1) = dht.readTemperature(true);
  *(p + 2) = dht.readHumidity();
 return t;
}
