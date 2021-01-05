/* 
 * File:   LCD_clock.c
 * Author: Shufen Situ
 *
 * Created on November 11, 2020, 3:06 PM
 */

#define F_CPU 8000000UL
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd.h"

//define pins
#define hour_button PD3
#define minute_button PD1
#define second_button PD2
#define reset_button PD0

//Initialize the variables hh, mm, ss
int hh = 00;
int mm = 00;
int ss = 00;

/*The following function initializes TTCR1B, TIMSK1, OCR1A and TCCR1B*/
void init() {
    TCCR1B |= (1 << WGM12); //configure timer for CTC mode
    TIMSK1 |= (1 << OCIE1A); //set the ISR COMPACT vect, enable CTC interrupt
    OCR1A = 7811; //set up the target value of the timer
    TCCR1B |= ((1 << CS10) | (1 << CS12)); //set the prescaler to 1024
    sei();
}

/*Create a global variable that forms a handle to the LCD "stream*/
FILE lcd_str = FDEV_SETUP_STREAM(lcd_putchar, NULL, _FDEV_SETUP_WRITE);

/*The following function will be called when the reset button is pressed. The lcd clock is now in 
 reset mode. When the reset button is pressed again, the lcd clcok will exit reset mode.*/
void check_button() {
    while (1) {
        /*Reset hours*/
        if ((PIND & _BV(hour_button)) == 0) {
            _delay_ms(10);
            if ((PIND & _BV(hour_button)) != 0) {
                if ((0 <= hh)&&(hh < 23)) {
                    hh++;
                } else {
                    hh = 00;
                }
                lcd_init();
                fprintf(&lcd_str, "    %02d:%02d:%02d    \x1b\xc0 Resetting Hour ", hh, mm, ss);
            }
        }

        /*Reset minutes*/
        if ((PIND & _BV(minute_button)) == 0) {
            _delay_ms(10);
            if ((PIND & _BV(minute_button)) != 0) {
                if ((0 <= mm)&&(mm < 59)) {
                    mm++;
                } else if (mm == 59) {
                    mm = 00;
                }
                lcd_init();
                fprintf(&lcd_str, "    %02d:%02d:%02d   \x1b\xc0 Resetting Min  ", hh, mm, ss);
            }
        }

        /*Reset seconds*/
        if ((PIND & _BV(second_button)) == 0) {
            _delay_ms(10);
            if ((PIND & _BV(second_button)) != 0) {
                if ((0 <= ss)&&(ss < 59)) {
                    ss++;
                } else if (ss == 59) {
                    ss = 00;
                }
                lcd_init();
                fprintf(&lcd_str, "    %02d:%02d:%02d    \x1b\xc0 Resetting Sec  ", hh, mm, ss);
            }
        }

        /*Exit reset modes*/
        if ((PIND & _BV(reset_button)) == 0) {
            _delay_ms(10);
            if ((PIND & _BV(reset_button)) == 1) {
                lcd_init();
                fprintf(&lcd_str, "    %02d:%02d:%02d    \x1b\xc0  Button Status ", hh, mm, ss);
                break;
            }
        }
    }
}

/*The following function update time by incrementing the values of seconds, minutes, and hours*/
void set_current_time() {
    ss++;
    if ((ss > 59) && (mm < 59)) {
        mm++;
        ss = 0;
    } else if ((ss > 59)&&(mm == 59) && (hh < 23)) {
        mm = 0;
        ss = 0;
        hh++;
    } else if ((ss > 59)&&(mm == 59) && (hh == 23)) {
        mm = 0;
        ss = 0;
        hh = 0;
    }
    lcd_init();
    fprintf(&lcd_str, "    %02d:%02d:%02d    \x1b\xc0  Button Status ", hh, mm, ss);
}

/* The following function initializes the LCD*/
int main() {
    init();
    DDRD &= ~(1 << hour_button); /*Set pin direction to input*/
    DDRD &= ~(1 << minute_button); /*Set pin direction to input*/
    DDRD &= ~(1 << second_button); /*Set pin direction to input*/
    DDRD &= ~(1 << reset_button); /*Set pin direction to input*/
    PORTD |= (1 << hour_button); /*default input is high*/
    PORTD |= (1 << minute_button); /*default input is high*/
    PORTD |= (1 << second_button); /*default input is high*/
    PORTD |= (1 << reset_button); /*default input is high*/
    lcd_init();
    fprintf(&lcd_str, "    %02d:%02d:%02d    \x1b\xc0  Button Status ", hh, mm, ss);
    //The while loop checks if the reset button is pressed
    while (1) {
        if ((PIND & _BV(reset_button)) == 0) {
            _delay_ms(10);
            if ((PIND & _BV(reset_button)) == 1) {
                cli();
                lcd_init();
                fprintf(&lcd_str, "    %02d:%02d:%02d   \x1b\xc0Resetting Time", hh, mm, ss);
                _delay_ms(100);
                check_button(); //Check if other buttons are pressed
                sei();
            }
        }
    }
}

/*The following is the interrupt function.*/
ISR(TIMER1_COMPA_vect) {
    set_current_time();
}
