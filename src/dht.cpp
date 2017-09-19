#include "application.h"
#include "dht.h"

DHT::DHT(uint8_t pin, uint8_t type, uint8_t count) {  // History array size
    _pin = pin;
    _type = type;
    _count = count;
    firstreading = true;
}

void DHT::begin(void) {
    // set up the pins!
    pinMode(_pin, INPUT);
    digitalWrite(_pin, HIGH);
    _lastreadtime = 0;
}

//boolean S == Scale.  True == Farenheit; False == Celcius
/*
Data consists of decimal and integral parts. A complete data transmission is 40bit, and the
sensor sends higher data bit first.
Data format:
  8bit integral RH data   [0]
+ 8bit decimal RH data    [1]
+ 8bit integral T data    [2]
+ 8bit decimal T data     [3]
+ 8bit check sum.         [4]
If the data transmission is right, the check-sum should be the last 8bit of
"8bit integral RH data + 8bit decimal RH data + 8bit integral T data + 8bit decimal T data".
*/
int DHT::readTemperature(bool S) {
    int _f;

    if (read()) {
        switch (_type) {
            case DHT11:
                _f = data[2];

                if(S)
                    _f = convertCtoF(_f);

                return _f;


            case DHT22:
            case DHT21:
                _f = data[2] & 0x7F;
                _f *= 256;
                _f += data[3];
                _f /= 10;

                if (data[2] & 0x80)
                    _f *= -1;

                if(S)
                    _f = convertCtoF(_f);

                return _f;
        }
    }

    return NAN;
}
int DHT::convertCtoF(int c) {
    return c * 9 / 5 + 32;
}

uint8_t * DHT::raw(void){
   if(read()){
    return data;
   }
}

uint8_t DHT::readHumidity(void) {
    uint8_t _f;
    if (read()) {
        switch (_type) {
            case DHT11:
                _f = data[0];
                return _f;


            case DHT22:
            case DHT21:
                _f = data[0];
                _f *= 256;
                _f += data[1];
                _f /= 10;
                return _f;
        }
    }

    return NAN;
}
bool DHT::read(void) {
    uint8_t laststate = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0, i;
    unsigned long currenttime;

    // pull the pin high and wait 250 milliseconds
    digitalWrite(_pin, HIGH);
    delay(250);

    currenttime = millis();
    if (currenttime < _lastreadtime) {
        // ie there was a rollover
        _lastreadtime = 0;
    }

    if (!firstreading && ((currenttime - _lastreadtime) < 2000)) {
        //delay(2000 - (currenttime - _lastreadtime));
        return true; // return last correct measurement
    }

    firstreading = false;
    Serial.print("Currtime: "); Serial.print(currenttime);
    Serial.print(" Lasttime: "); Serial.print(_lastreadtime);
    _lastreadtime = millis();

    data[0] = data[1] = data[2] = data[3] = data[4] = 0;

    // now pull it low for ~20 milliseconds
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    delay(20);
    cli();
    digitalWrite(_pin, HIGH);
    delayMicroseconds(40);
    pinMode(_pin, INPUT);

    // read in timings
    for ( i=0; i< MAXTIMINGS; i++) {
        counter = 0;

        while (digitalRead(_pin) == laststate) {
            counter++;
            delayMicroseconds(1);

            if (counter == 255)
                break;
        }

        laststate = digitalRead(_pin);

        if (counter == 255)
            break;

        // ignore first 3 transitions
        if ((i >= 4) && (i%2 == 0)) {
            // shove each bit into the storage bytes
            data[j/8] <<= 1;

            if (counter > _count)
                data[j/8] |= 1;

            j++;
        }
    }

    sei();


    // check we read 40 bits and that the checksum matches
    if ((j >= 40) &&  (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)))
        return true;


    return false;
}
