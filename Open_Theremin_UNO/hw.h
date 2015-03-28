#ifndef _HW_H
#define _HW_H

#define HW_BUTTON_STATE    (PIND & (1<<PORTD4))
#define HW_BUTTON_PRESSED  (HW_BUTTON_STATE == LOW)
#define HW_BUTTON_RELEASED (HW_BUTTON_STATE != LOW)

#define HW_LED_ON          (PORTD |= (1<<PORTD5))
#define HW_LED_OFF         (PORTD &= ~(1<<PORTD5))

#endif // _HW_H

