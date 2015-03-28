#include "Arduino.h"

#include "ihandlers.h"
#include "mcpDac.h"
#include "timer.h"

#include "theremin_sintable.c"
#include "theremin_sintable2.c"
#include "theremin_sintable3.c"
#include "theremin_sintable4.c"
#include "theremin_sintable5.c"
#include "theremin_sintable6.c"
#include "theremin_sintable7.c"
#include "theremin_sintable8.c"

const int16_t* const wavetables[] PROGMEM = {
  sine_table,
  sine_table2,
  sine_table3,
  sine_table4,
  sine_table5,
  sine_table6,
  sine_table7,
  sine_table8
};

#define INT0_STATE    (PIND & (1<<PORTD2))
#define PC_STATE      (PINB & (1<<PORTB0))

volatile uint8_t vol8 = 0;
volatile uint16_t add_val = 0;

volatile uint16_t pitch = 0;            // Pitch value
volatile uint16_t pitch_counter = 0;    // Pitch counter
volatile uint16_t pitch_counter_l = 0;  // Last value of pitch counter

volatile bool volumeValueAvailable = 0;            // Volume read flag
volatile bool pitchValueAvailable = 0;          // Pitch read flag

volatile uint16_t vol;                 // Volume value
volatile uint16_t vol_counter = 0;
volatile uint16_t vol_counter_i = 0;   // Volume counter
volatile uint16_t vol_counter_l;       // Last value of volume counter

volatile uint8_t wavetableSelector = 0;            // wavetable selector

static volatile uint16_t pointer       = 0;  // Table pointer
static volatile uint8_t  debounce_p, debounce_v  = 0;  // Counters for debouncing

void ihInitialiseTimer() {
  /* Setup Timer 1, 16 bit timer used to measure pitch and volume frequency */
  TCCR1A = 0;                     // Set Timer 1 to Normal port operation (Arduino does activate something here ?)
  TCCR1B = (1<<ICES1)|(1<<CS10);  // Input Capture Positive edge select, Run without prescaling (16 Mhz)
  TIMSK1 = (1<<ICIE1);            // Enable Input Capture Inrrupt
}

void ihInitialiseInterrupts() {
  /* Setup interrupts for Wave Generator and Volume read */
  EICRA = (1<<ISC00)|(1<<ISC01)|(1<<ISC11)|(1<<ISC10) ; // The rising edges of INT0 and INT1 generate an interrupt request.
  EIMSK = (1<<INT0)|(1<<INT1);                          // Enable External Interrupt INT0 and INT1
}

/* 16 bit by 8 bit multiplication */
static inline uint32_t mul_16_8(uint16_t a, uint8_t b)
{
  uint32_t product;
  asm (
    "mul %A1, %2\n\t"
    "movw %A0, r0\n\t"
    "clr %C0\n\t"
    "clr %D0\n\t"
    "mul %B1, %2\n\t"
    "add %B0, r0\n\t"
    "adc %C0, r1\n\t"
    "clr r1"
    :
    "=&r" (product)
    :
    "r" (a), "r" (b));
  return product;
}

/* Externaly generated 31250 Hz Interrupt for WAVE generator (32us) */
ISR (INT1_vect) {
  // Interrupt takes up a total of max 25 us

  disableInt1();
  // Enable Interrupts to allow counter 1 interrupts
  interrupts();

  int16_t  waveSample;
  uint32_t scaledSample;
  uint16_t offset = (uint16_t)(pointer>>6) & 0x3ff;

  // Read next wave table value (3.0us)
  // The slightly odd tactic here is to provide compile-time expressions for the wavetable
  // positions. Making addr1 the index into the wavtables array breaks the time limit for
  // the interrupt handler
  switch (wavetableSelector) {
    case 1:  waveSample = (int16_t) pgm_read_word_near(wavetables[1] + offset); break; 
    case 2:  waveSample = (int16_t) pgm_read_word_near(wavetables[2] + offset); break; 
    case 3:  waveSample = (int16_t) pgm_read_word_near(wavetables[3] + offset); break; 
    case 4:  waveSample = (int16_t) pgm_read_word_near(wavetables[4] + offset); break; 
    case 5:  waveSample = (int16_t) pgm_read_word_near(wavetables[5] + offset); break; 
    case 6:  waveSample = (int16_t) pgm_read_word_near(wavetables[6] + offset); break; 
    case 7:  waveSample = (int16_t) pgm_read_word_near(wavetables[7] + offset); break; 
    default: waveSample = (int16_t) pgm_read_word_near(wavetables[0] + offset); break; 
  };

  if (waveSample > 0) {                   // multiply 16 bit wave number by 8 bit volume value (11.2us / 5.4us)
    scaledSample = mul_16_8(waveSample, vol8);
    scaledSample = scaledSample >> 9;
    scaledSample = scaledSample + 1748;
  } else {
    scaledSample = mul_16_8(-waveSample, vol8);
    scaledSample = scaledSample >> 9;
    scaledSample = 1748 - scaledSample;
  }

  mcpDacSend(scaledSample);        //Send result to Digital to Analogue Converter (audio out) (9.6 us)

  pointer = pointer + add_val;    // increment table pointer (ca. 2us)
  incrementTimer();               // update 32us timer

  if (PC_STATE) debounce_p++;
  if (debounce_p == 3) {
    noInterrupts();
    pitch_counter = ICR1;                      // Get Timer-Counter 1 value
    pitch = (pitch_counter - pitch_counter_l); // Counter change since last interrupt -> pitch value
    pitch_counter_l = pitch_counter;           // Set actual value as new last value
  };

  if (debounce_p == 5) {
    pitchValueAvailable = true;
  };

  if (INT0_STATE) debounce_v++;
  if (debounce_v == 3) {
    noInterrupts();
    vol_counter = vol_counter_i;            // Get Timer-Counter 1 value
    vol = (vol_counter - vol_counter_l);    // Counter change since last interrupt
    vol_counter_l = vol_counter;            // Set actual value as new last value
  };

  if (debounce_v == 5) {
    volumeValueAvailable = true;
  };

  noInterrupts();
  enableInt1();
}

/* VOLUME read - interrupt service routine for capturing volume counter value */
ISR (INT0_vect) {
  vol_counter_i = TCNT1;
  debounce_v = 0;
};


/* PITCH read - interrupt service routine for capturing pitch counter value */
ISR (TIMER1_CAPT_vect) {
  debounce_p = 0;
};


