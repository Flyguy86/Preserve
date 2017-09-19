#ifndef dht_h
#define dht_h

#define MAXTIMINGS 85
#define cli noInterrupts
#define sei interrupts
#define DHT11 11
#define DHT22 22
#define DHT21 21
#define AM2301 21
#define NAN 9999

class DHT {
    private:
        uint8_t data[6];
        uint8_t _pin, _type, _count;
        bool read(void);
        unsigned long _lastreadtime;
        bool firstreading;
//        Humidity_rate_of_Change();
//        Temperature_rate_of_Change();
//        uint8_t Temperature_history[20];
//        uint8_t Humidity_history[20];
//        uint8_t TH_time_history[20];
//
    public:
        DHT(uint8_t pin, uint8_t type, uint8_t count=6);
        void begin(void);
        int readTemperature(bool S=false);
        int convertCtoF(int);
        uint8_t readHumidity(void);
        uint8_t * raw(void);

};
#endif
