#include <avr/io.h>
#include <util/delay.h>

#define SERVO_MIN 1000 // Minimum pulse width in microseconds (1ms)
#define SERVO_MAX 2000 // Maximum pulse width in microseconds (2ms)
#define SERVO_STEP 10  // Step size for sweeping in microseconds
#define F_CPU 16000000UL // Define CPU frequency (16 MHz)

void servo_init() {
    // Configure Timer1 for Fast PWM mode, non-inverted
    TCCR1A |= (1 << WGM11) | (1 << COM1A1);    // Fast PWM, non-inverted mode
    TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11); // Prescaler = 8
    ICR1 = 20000; // Top value for 50Hz (20ms period)

    // Set the servo pin (PB1/OC1A) as output
    DDRB |= (1 << PB1);
}

// void set_servo_angle(uint16_t pulse_width) {
//     OCR1A = pulse_width; // Set the pulse width (1ms to 2ms)
// }

void set_servo_angle(uint8_t angle) {
    // Map angle (0-180) to pulse width (1000-2000 microseconds)
    uint16_t pulse_width = SERVO_MIN + ((uint32_t)(SERVO_MAX - SERVO_MIN) * angle) / 180;
    OCR1A = pulse_width; // Set the pulse width
}