/*	Author: Andrew Bazua
 *  Partner(s) Name:
 *	Lab Section:
 *	Assignment: Lab #6  Exercise #1
 *	Exercise Description: [ Create a synchSM to blink three LEDs connected to
        PB0, PB1, and PB2 in sequence, 1 second each. Implement that synchSM
        in C using the method defined in class. In addition to demoing your
        program, you will need to show that your code adheres entirely to
        the method with no variations. ]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

typedef enum States {Start, init, nextLED} States;

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0;   // Current internal count of 1 ms ticks.

void TimerOn() {
    TCCR1B = 0x0B;

    OCR1A = 125;

    TIMSK1 = 0x02;

    TCNT1=0;

    _avr_timer_cntcurr = _avr_timer_M;

    SREG |= 0x80;

}

void TimerOff() {
    TCCR1B = 0x00;
}

void TimerISR() {
    TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
    // CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
    _avr_timer_cntcurr--;   // Count down to 0 rather than up to TOP
    if (_avr_timer_cntcurr == 0) {
        TimerISR();
        _avr_timer_cntcurr = _avr_timer_M;
    }

}

void TimerSet(unsigned long M) {
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
}

int Tick(int state);

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;

    TimerSet(1000);
    TimerOn();

    States state = Start;

    /* Insert your solution below */
    while (1) {
        state = Tick(state);
        while (!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}

int Tick(int state) {
    static unsigned char out;

    switch (state) {
        case Start:
            state = init;
            break;

        case init :
            state = nextLED;
            break;

        case nextLED :
            state = nextLED;
            break;

        default : break;
    }

    switch (state) {
        case Start: break;

        case init :
            out = 0x01;
            PORTB = out;
            break;

        case nextLED :
            out = out << 1;
            if (out == 0x08) { out = 0x01; }
            PORTB = out;
            break;

        default : break;

    }

    return state;
}
