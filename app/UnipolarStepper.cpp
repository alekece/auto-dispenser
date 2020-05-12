#include "UnipolarStepper.h"

#include <Arduino.h>

UnipolarStepper::UnipolarStepper(
    unsigned int stepsPerRevolution, int pin1, int pin3, int pin2, int pin4) :
    Stepper(stepsPerRevolution, pin1, pin3, pin4, pin2),
    revolutionSense_{0},
    pins_{pin1, pin2, pin3, pin4}
{
}

void UnipolarStepper::setSpeed(int stepsPerMinute)
{
    if (!stepsPerMinute) {
        saveEnergy();
        revolutionSense_ = 0;
    }
    else {
        // turns clockwise if @p stepsPerMinute is positive otherwise counter clockwise
        revolutionSense_ = stepsPerMinute > 0 ? 1 : -1;
        // Arduino stepper only accepts positive value
        Stepper::setSpeed(abs(stepsPerMinute));
    }
}


void UnipolarStepper::run()
{
    step(revolutionSense_);
}

void UnipolarStepper::saveEnergy() const
{
    for (auto pin : pins_) {
        digitalWrite(pin, LOW);
    }
}
