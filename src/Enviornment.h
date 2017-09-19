#ifndef env_h
#define env_h

#define DHTPIN A7
#define DHTTYPE DHT11  // sensor type.  Options: [DHT11, DHT22, DHT21, AM2301]
#define DHTPIN2 D3
#define DHTPIN3 D4

#define IR_PIN D6
#define LED_PIN D7

class ENV {
    private:
      uint8_t _targetTemperature, _targetHumidity;
      uint16_t _cycletime;
      uint32_t err, _lasttime;
      void Endtime(void);
      uint32_t CycleTimetoMillis(void);
      uint8_t _HoutofrangeCounter;
      uint8_t _ToutofrangeCounter;
      uint8_t _action;
      uint16_t _countThresh;
      uint8_t _debug;
      int _Tlastdir;
      int _Hlastdir;
      int _Tgap;
      int _Hgap;
      int _alarmThreshold;
      int _escalationThreshold;

      bool _DryOn;
      bool _ACon;
      bool _HeatOn;
      bool _FanOn;


      void EnvOff(void);
      void HeatOn(void);
      void ACOn(void);
      void FanOn(void);
      void DryOn(void);
      void ActionDescider(uint8_t comparator, uint16_t _countThresh);
      void countCrossedtarget(void);

 //      void ExhaustFanOn(void);
 //      void ExhaustFanoff(void);
 //      void CirculationFanOn(void);
 //      void CirculationFanOff(void);

    public:


      ENV(uint8_t tt, uint8_t ht, uint16_t ct, uint16_t countThresh);  //temperature target,  Humidity Target,  Cycle Time (seconds)
      void begin(void);
      uint8_t i_t_h[4]; // Internal
                        // i_t_h[1] Temperature
                        // i_t_h[2] Humitidy
                        // i_t_h[3] T Target
                        // i_t_h[4] H Target
      uint8_t a_t_h[4];  // Airflow
      uint8_t o_t_h[4];  // Outside

      int istats[4];//istats[1] -256 to 256  Target - Current value T-direction & T-difference from target,
                    //istats[2] H-direction & T-difference from target,
                    //istats[3] Temperature counts from last 0
                    //istats[4] Humidity counts from last 0 crossing

      int astats[4];  // Airflow stats
      int ostats[4];  // Outside stats
      bool Alarm;
      // number of out of range values before action is taken
      void Publish(String a);
      void readAllSensors();
      uint8_t TargetValueCompare(uint8_t *p, int *f);
      uint8_t DHT_Check(uint8_t dhtPin, uint8_t dhtType, uint8_t *a);
      void updateTargets(uint8_t tt, uint8_t ht, uint16_t ct, uint16_t countThresh, uint8_t debug);
      void loop(void);

};
#endif

//  32 bit error code
//  | 16 codes  | 16 codes  |    16   |    16      |    256    |   256    |
//  | Global ID | Error ID  |  count  | Condition  |  thresh   |   value  |
//  | unit4_t   | unit4_t   | unit4_t |  unit4_t   |  unit8_t  |  unit8_t |
//    0  -  3     4  - 7       8 - 11    12 - 15      16 - 23     24 - 32
// Global Id              ErrorID
// 0 = System         // 1=place holder  2=placeholder
// 1 = Sensor         // 1=DHT1 fail  2=DHT2 fail  3=DHT3 fail  4=DHT4 fail   5=RF fail   6=IR Fail
// 2 = Temperature    // 1=HeaterON  2=HeaterOff  3=ACon  4=ACoff  5=exhaustOn  6=exhaustOff
// 3 = Humidity       // 1=ExhaustOn  2=ExhaustOff  3=ACon  4=AcOff
// 4 = Device Actions //
// 5 = Critical
// 6 = Alarm
//  Error bits ? |  |   |
//  all Off
//  all on
//  temp < target && Humidity > target = Heateron   ->  if continues Exhaust fansOn
//  temp < target && Humidity < target = HeaterOn && Exhaust fan off
//  temp > target && Humidity > target = AC on,  -> if confintue Exhaust fanOn
//  temp > target && Humidity < target = AC on,  ->


// Flow
//  Define Target value   Example:  78F temperature,   Humidity 50%
//   Read Target sensors
//    Determine Actions
//      Take Action for X time
//
