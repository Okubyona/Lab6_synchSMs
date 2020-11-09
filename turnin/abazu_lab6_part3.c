/*	Author: Andrew Bazua
 *  Partner(s) Name:
 *	Lab Section:
 *	Assignment: Lab #6  Exercise #3
 *	Exercise Description: [ Buttons are connected to PA0 and PA1. Output for
    PORTB is initially 7. Pressing PA0 increments PORTB once (stopping at 9).
    Pressing PA1 decrements PORTB once (stopping at 0). If both buttons are
    depressed (even if not initially simultaneously), PORTB resets to 0. Now
    that we have timing, only check to see if a button has been pressed every
    100 ms. Additionally, if a button is held, then the count should continue
    to increment (or decrement) at a rate of once per second.]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *  Demo Link:
 *  https://drive.google.com/file/d/1RyyC7KCdkmdE2tQ2U1sJes80vJW8Y2AZ/view?usp=sharing
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

typedef enum States {init, waitA, increment, waitInc, decrement, waitDec, reset} States;

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
    DDRB = 0xFF; PORTB = 0x00; // Using portC for ease of filming

    TimerSet(100);
    TimerOn();

    States state = init;

    /* Insert your solution below */
    while (1) {
        state = Tick(state);
        while (!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}

int Tick(int state) {
    static unsigned char holdcrement;
    static unsigned char count;

    unsigned char A0 = ~PINA & 0x01;
    unsigned char A1 = ~PINA & 0x02;

    switch (state) {        //TRANSITIONS
        case init:
            state = waitA;
            count = 0x07;
            break;

        case waitA:
            if (A0 && !A1){ state = increment; }
            else if (!A0 && A1) { state = decrement; }
            else if (A0 && A1) { state = reset; }
            else { state = waitA; }
            break;

        case increment:
            if (A0 && !A1) { state = waitInc; }
            else if (A0 && A1) { state = reset; }
            else if (!A0 && !A1) { state = waitA; }
            break;

        case waitInc:
            if (!A0 && !A1) { state = waitA; }
            else if (A0 && !A1) { state = waitInc; }
            else if (A0 && A1) { state = reset; }
            break;

        case decrement:
            if (!A0 && A1) { state = waitDec; }
            else if (A0 && A1) { state = reset; }
            else if (!A0 && !A1) { state = waitA; }
            break;

        case waitDec:
            if (!A0 && !A1) { state = waitA; }
            else if (A0 && A1) { state = reset; }
            else if (!A0 && A1) { state = waitDec; }
            break;

        case reset:
            state = (A0 || A1)? reset: waitA;
            break;

        default: break;
    }

    switch (state) {
        case init:
            holdcrement = 0x00;
            count = 0x07;
            break;

        case waitA:
            holdcrement = 0;
            break;
        case increment:
            if (count < 9) { ++count; }
            break;

        case waitInc:
            ++holdcrement;
            if (holdcrement == 10) { // decrement after 1 second of holding
                 if (count < 9) { ++count; }
                holdcrement = 0;
            }
            break;

        case decrement:
            if (count > 0) { --count; }
            break;

        case waitDec:
            ++holdcrement;
            if (holdcrement == 10) { // decrement after 1 second of holding
                if ( count > 0) { --count; }
                holdcrement = 0;
            }
            break;
        case reset:
            count = 0x00;
            break;
        default: break;
    }

    PORTB = count;

    return state;
}
