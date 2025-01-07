#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include "uart.h"
#include "rtc.h"
#include "servo.h"
#include "twi_master.h"
#include "lcd.h"

#define BUTTON_PIN PD7 // Define the push button pin
#define MATCH_HOUR1 10
#define MATCH_MINUTE1 38
#define MATCH_SECOND1 00
#define MATCH_HOUR2 10
#define MATCH_MINUTE2 38
#define MATCH_SECOND2 10

// Global variables
volatile bool button_pressed = false;
volatile bool check_time = false;
volatile bool time_matched1 = false;
volatile bool time_matched2 = false;

ISR(INT0_vect) {
    button_pressed = true; // Set the button pressed flag
}

void button_init(void) {
    // Set BUTTON_PIN as input with pull-up resistor
    DDRD &= ~(1 << BUTTON_PIN);
    PORTD |= (1 << BUTTON_PIN);

    // Enable external interrupt on falling edge for BUTTON_PIN
    // EICRA |= (1 << ISC01); // ISC01 = 1, ISC00 = 0 for falling edge
    // EIMSK |= (1 << INT0);  // Enable INT0

    // Enable pin change interrupt for PD7
    PCICR |= (1 << PCIE2);  // Enable pin change interrupt for PCINT[23:16]
    PCMSK2 |= (1 << PCINT23);  // Enable interrupt for PD7
}

void timer0_init(void) {
    // Set Timer0 to CTC mode, prescaler 1024
    TCCR0A |= (1 << WGM01);
    TCCR0B |= (1 << CS02) | (1 << CS00);

    // Set OCR0A for 1 second interrupt
    OCR0A = 156; // For 1 second (16MHz clock, prescaler 1024)

    // Enable Timer0 compare interrupt
    TIMSK0 |= (1 << OCIE0A);
}

ISR(TIMER0_COMPA_vect) {
    check_time = true; // Set the flag to check time in the main loop
}


int main(void) {
    // Initialize TWI (I2C), RTC, servo, timer, UART, LCD, and button
    tw_init(TW_FREQ_100K, true);
    ds1307_init();
    servo_init(); // Initialize servo
    timer0_init(); // Initialize Timer0
    uart_init(9600); // Initialize UART for debugging
    lcd_init(); // Initialize LCD
    button_init(); // Initialize button

    // Enable global interrupts
    sei();

    // Set an initial time (optional)
    DS1307_Time initial_time = {
        .seconds = 50,
        .minutes = 37,
        .hours = 10,
        .day = 5,     // Friday
        .date = 6,    // 6th of the month
        .month = 12,  // December
        .year = 24    // 2024
    };

    // Set the initial time to the RTC
    if (!ds1307_set_time(&initial_time)) {
        uart_println("RTC Set Time Error");
    } else {
        uart_println("RTC Set Time Success");
    }

    uart_println("Initialization Complete");
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_write("Init Complete");

    while (1) {
        if (check_time) {
            check_time = false; // Reset the flag
            DS1307_Time current_time;
            if (ds1307_get_time(&current_time)) {
                char time_str[17];
                snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", current_time.hours, current_time.minutes, current_time.seconds);
                uart_println(time_str);

                // Display time on LCD
                lcd_set_cursor(0, 0);
                lcd_write("                "); // Clear the line
                lcd_set_cursor(0, 0);
                lcd_write(time_str);

                char date_str[17];
                snprintf(date_str, sizeof(date_str), "%02d/%02d/20%02d", current_time.date, current_time.month, current_time.year);
                lcd_set_cursor(0, 1);
                lcd_write("                "); // Clear the line
                lcd_set_cursor(0, 1);
                lcd_write(date_str);

                if (current_time.hours == MATCH_HOUR1 &&
                    current_time.minutes == MATCH_MINUTE1 &&
                    current_time.seconds == MATCH_SECOND1) {
                    time_matched1 = true;
                }

                if (current_time.hours == MATCH_HOUR2 &&
                    current_time.minutes == MATCH_MINUTE2 &&
                    current_time.seconds == MATCH_SECOND2) {
                    time_matched2 = true;
                }
            } else {
                uart_println("RTC Read Error");
            }
        }

        if (button_pressed || time_matched1 || time_matched2) {
            uart_println("Activating Servo");
            lcd_set_cursor(1, 0);
            lcd_write("                "); // Clear the line
            lcd_set_cursor(1, 0);
            lcd_write("Activating Servo");

            // Move servo to 90 degrees (1.5ms pulse width)
            set_servo_angle(180);
            uart_println("Servo to 90 degrees");
            lcd_set_cursor(1, 0);
            lcd_write("                "); // Clear the line
            lcd_set_cursor(1, 0);
            lcd_write("Feeding Time");
            _delay_ms(2000); // Keep the servo in position for 1 second

            // Move servo back to 0 degrees (1ms pulse width)
            set_servo_angle(0);
            uart_println("Servo to 0 degrees");
            lcd_set_cursor(1, 0);
            lcd_write("                "); // Clear the line
            lcd_set_cursor(1, 0);
            lcd_write("Closing Time");
            _delay_ms(2000); // Keep the servo in position for 1 second

            uart_println("Servo Reset");
            lcd_set_cursor(1, 0);
            lcd_write("                "); // Clear the line
            lcd_set_cursor(1, 0);

            button_pressed = false; // Reset the button pressed flag
            time_matched1 = false; // Reset the time matched flag
            time_matched2 = false; // Reset the time matched flag
        }

        // Add a small delay to avoid excessive I2C communication
        _delay_ms(1000);
    }

    return 0;
}
