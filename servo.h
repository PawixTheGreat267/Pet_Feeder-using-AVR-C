#ifndef SERVO_H
#define SERVO_H

#include <avr/io.h>
#include <util/delay.h>

#define SERVO_MIN 1000 // Minimum pulse width in microseconds (1ms)
#define SERVO_MAX 2000 // Maximum pulse width in microseconds (2ms)
#define SERVO_STEP 10  // Step size for sweeping in microseconds


void servo_init(void);
void set_servo_angle(uint16_t pulse_width);

#endif