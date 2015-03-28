#include "Arduino.h"

#include "application.h"

#include "hw.h"
#include "mcpDac.h"
#include "ihandlers.h"
#include "timer.h"

const AppMode AppModeValues[] = {TRIM_PITCH, TRIM_VOLUME, PITCH_ONLY, NORMAL};

static int32_t pitchCalibrationBase = 0;
static int32_t volCalibrationBase   = 0;

Application::Application() 
  : _state(PLAYING),
    _mode(NORMAL) {
};

void Application::setup() {
#if SERIAL_ENABLED
  Serial.begin(Application::BAUD);
#endif

  pinMode(Application::BUTTON_PIN, INPUT_PULLUP);            
  pinMode(Application::LED_PIN,    OUTPUT);
  
  initialiseTimer();  
  initialiseInterrupts();

  interrupts();
  
  mcpDacInit();
  
#if CV_ENABLED
  initialiseCVOut();
#endif

  playStartupSound();
  calibrate();
}

void Application::initialiseTimer() {
  ihInitialiseTimer();
}

void Application::initialiseInterrupts() {
  ihInitialiseInterrupts();
}

#if CV_ENABLED                                 // Initialise PWM Generator for CV oitput
void initialiseCVOut() {
   TCCR0A = (1<<COM0A1)|(1<<WGM01)|(1<<WGM00); // Set PWM on OC0A
   TCCR0B = (1<<CS00);                         // Set Clock with No prescaling on Timer 0
   TIMSK0=0;                                   // No interrups
   pinMode(CV_PIN, OUTPUT);                    // Set Pin D6 as output
}
#endif

AppMode Application::nextMode() {
  return _mode == NORMAL ? TRIM_PITCH : AppModeValues[_mode + 1];
}

void Application::loop() {
  int32_t pitch_v = 0, pitch_l = 0;            // Last value of pitch  (for filtering)
  int32_t vol_v = 0,   vol_l = 0;              // Last value of volume (for filtering)
  uint16_t volumePotValue = 0;                 // State of volume potentiometer

#if !EXTENDED
  wavetableSelector = DEFAULT_WAVETABLE;
  volumePotValue  = 1024;
#endif

  mloop:                   // Main loop avoiding the GCC "optimization"

#if EXTENDED  
  volumePotValue = analogRead(VOLUME_POT);
  wavetableSelector = analogRead(WAVE_SELECT_POT) >> 7;
#endif

  if (_state == PLAYING && HW_BUTTON_PRESSED) {
    _state = CALIBRATING;
    resetTimer();
  }

  if (_state == CALIBRATING && HW_BUTTON_RELEASED) {
    if (timerExpired(1500)) {
      HW_LED_ON;
      playCalibratingCountdownSound();

      calibrate();
      
      _mode=NORMAL;
      HW_LED_OFF;
    } 
    _state = PLAYING;
  };

  if (_state == CALIBRATING && timerExpired(6000)) {
    _mode = nextMode();

    playModeSettingSound();
    
    while (HW_BUTTON_PRESSED) 
      ; // NOP
    _state = PLAYING;
  };

#if CV_ENABLED
  OCR0A = pitch & 0xff;
#endif

#if SERIAL_ENABLED
  if (timerExpired(TICKS_100_MILLIS)) {
    resetTimer();
    Serial.write(pitch & 0xff);              // Send char on serial (if used)
    Serial.write((pitch >> 8) & 0xff);
  }
#endif

  if (pitchValueAvailable) {                        // If capture event

    pitch_v=pitch;                         // Averaging pitch values
    pitch_v=pitch_l+((pitch_v-pitch_l)>>2); 
    pitch_l=pitch_v;    

    // set wave frequency for each mode
    switch (_mode) {
      case TRIM_PITCH  : setWavetableSampleAdvance(TRIM_PITCH_FACTOR/pitch_v);  break;
      case TRIM_VOLUME : /* NOTHING! */;                                        break;
      case PITCH_ONLY  : setWavetableSampleAdvance((pitchCalibrationBase-pitch_v)/2+200); break;
      case NORMAL      : setWavetableSampleAdvance((pitchCalibrationBase-pitch_v)/2+200); break;
    };
    
    pitchValueAvailable = false;
  }

  if (volumeValueAvailable) {
    vol = max(vol, 5000);

    vol_v=vol;                  // Averaging volume values
    vol_v=vol_l+((vol_v-vol_l)>>2);
    vol_l=vol_v;

    switch (_mode) {                 
      case TRIM_PITCH:  vol_v = MAX_VOLUME;                                                      break;
      case TRIM_VOLUME: vol_v = MAX_VOLUME; setWavetableSampleAdvance(TRIM_PITCH_FACTOR/vol_v);  break;
      case PITCH_ONLY:  vol_v = MAX_VOLUME;                                                      break;
      case NORMAL:      vol_v = MAX_VOLUME-(volCalibrationBase-vol_v);                                     break;
    };

    // Limit and set volume value
    vol_v = min(vol_v, 4095);
    vol_v = vol_v - (1 + MAX_VOLUME - (volumePotValue << 2));
    vol_v = max(vol_v, 0);
    vol8 = vol_v >> 4;
    
    volumeValueAvailable = false;
  }

  goto mloop;                           // End of main loop 
}

void Application::calibrate()           
{
  resetPitchFlag();
  resetTimer();  
  savePitchCounter();
  while (!pitchValueAvailable && timerUnexpiredMillis(10))
    ; // NOP
  pitchCalibrationBase = pitch;

  resetVolFlag();
  resetTimer();
  saveVolCounter();
  while (!volumeValueAvailable && timerUnexpiredMillis(10))
    ; // NOP
  volCalibrationBase = vol;
}

void Application::hzToAddVal(float hz) {
  setWavetableSampleAdvance((uint16_t)(hz * HZ_ADDVAL_FACTOR));
}

void Application::playNote(float hz, uint16_t milliseconds = 500, uint8_t volume = 255) {
  vol8 = volume;
  hzToAddVal(hz);
  millitimer(milliseconds);
  vol8 = 0;
}

void Application::playStartupSound() {
  playNote(MIDDLE_C, 150);
  playNote(MIDDLE_C * 2, 150);
  playNote(MIDDLE_C * 4, 150);
}

void Application::playCalibratingCountdownSound() {
  for (int i = 0; i < 5; i++) {
    playNote(MIDDLE_C, 500);
    millitimer(150);
  }
  playNote(MIDDLE_C * 2, 1000);
}

void Application::playModeSettingSound() {
  for (int i = 0; i <= _mode; i++) {
    playNote(MIDDLE_C * 2, 200);
    millitimer(100);
  }
}




