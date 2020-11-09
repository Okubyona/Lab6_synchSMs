/*	Author: Andrew Bazua
 *  Partner(s) Name:
 *	Lab Section:
 *	Assignment: Lab #6  Exercise #2
 *	Exercise Description: [ Create a simple light game that requires pressing a
    button on PA0 while the middle of three LEDs on PB0, PB1, and PB2 is lit.
    The LEDs light for 300 ms each in sequence. When the button is pressed, the
    currently lit LED stays lit. Pressing the button again restarts the game. ]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *  Demo Link:
 *  https://drive.google.com/file/d/1ww_Ni9QxYpjOQ9dPuJ06Ex9uC9DETu2a/view?usp=sharing
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

typedef enum States {Start, init, nextLED, pause, wait, reset} States;

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

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

    TimerSet(300);
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
    static unsigned char upDown;

    unsigned char A0 = ~PINA & 0x01;

    switch (state) {
        case Start:
            state = init;
            break;

        case init :
            state = nextLED;
            break;

        case nextLED :
            state = A0 ? pause: nextLED;
            break;

        case pause:
            state = A0 ? pause: wait;
            break;

        case wait:
            state = A0 ? reset: wait;
            break;

        case reset:
            state = nextLED;
            break;

        default: break;
    }

    switch (state) {
        case Start: break;

        case init :
            out = 0x01;
            upDown = 0x00;
            PORTB = out;
            break;

        case nextLED :
            if (upDown) { // B2 -> B1 -> B0
                out = out >> 0x01;
                if (out == 0x00) {
                    out = 0x02;
                    upDown = 0x00;
                }
            }
            else { // B0 -> B1 -> B2
                out = out << 1;
                if (out == 0x08) {
                    out = 0x02;
                    upDown = 0x01;
                }
            }
            PORTB = out;
            break;

        case pause: break;


        case wait: break;

        case reset: break;

        default: break;

    }

    return state;
}
