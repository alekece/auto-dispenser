#pragma once

#include <Stepper.h>

class UnipolarStepper : private Stepper {
   public:
    UnipolarStepper(unsigned int stepsPerRevolution, int pin1, int pin3, int pin2, int pin4);

    void setSpeed(int stepsPerMinute);
    void run();

   private:
    void saveEnergy() const;

    int revolutionSense_;
    int const pins_[4];
};
