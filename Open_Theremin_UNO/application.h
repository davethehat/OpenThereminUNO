#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <avr/io.h>

#include "build.h"

enum AppState {CALIBRATING = 0, PLAYING};
enum AppMode  {TRIM_PITCH = 0, TRIM_VOLUME, PITCH_ONLY, NORMAL};

class Application {
  public:
    Application();
    
    void setup();
    void loop();
    
  private:
    static const uint16_t MAX_VOLUME = 4095;
    static const uint32_t TRIM_PITCH_FACTOR = 33554432;
    
    static const int16_t BUTTON_PIN = 4;
    static const int16_t LED_PIN    = 5;
    static const int16_t CV_PIN     = 6;
    
#if EXTENDED
    static const int16_t VOLUME_POT = 0;
    static const int16_t WAVE_SELECT_POT = 1;
#endif
    
#if SERIAL_ENABLED    
    static const int BAUD = 115200;
#endif

    AppState _state;
    AppMode  _mode;
        
    void calibrate();
    AppMode nextMode();
    
    void initialiseTimer();
    void initialiseInterrupts();

    static const float HZ_ADDVAL_FACTOR = 2.09785;
    static const float MIDDLE_C = 261.6;

    void playNote(float hz, uint16_t milliseconds, uint8_t volume);
    void hzToAddVal(float hz);
    void playStartupSound();
    void playCalibratingCountdownSound();
    void playModeSettingSound();
};

#endif // _APPLICATION_H
