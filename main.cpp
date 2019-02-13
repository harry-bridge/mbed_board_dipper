#include "mbed.h"
#include "rtos.h"

Serial pc(USBTX, USBRX);

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);
DigitalIn joystickDown(p12, PullDown);
DigitalIn joystickCenter(p14, PullDown);
DigitalIn joystickUp(p15, PullDown);

PwmOut speakerPin(p26);
DigitalOut pulsePin(p21);
DigitalOut dirPin(p22);

class Stepper {
public:

    int stepsPerRev = 10000;
    float mmPerStep = 0.0005;

    int mmToSteps(float mmToMove) {
        return mmToMove * 2000;
    }

    // Constantly move steps
    // dir 1 for down dir 0 for up
    int move(int dir, float speed = 1) {
        float stepDelay = (1 / speed) * 0.5;

        dirPin = dir;

        pulsePin = 1;
        wait(stepDelay);
        pulsePin = 0;
        wait(stepDelay);

        return 1; // Gone one whole microstep
    }

    // Move at a speed in mm/sec
    // dir 1 for down dir 0 for up
    int moveSteps(int steps, int dir = 0, float speed = 1) {
        int currentMicroStep = 0;
        float stepDelay = (1 / speed) * 0.5;

        dirPin = dir;

        while (currentMicroStep < steps) {
            pulsePin = 1;
            wait(stepDelay);
            pulsePin = 0;
            wait(stepDelay);

            currentMicroStep++;

//            pc.printf("%i\r\n", currentMicroStep);
        }

        return 1;
    }
};

// number of beeps, speed in hz, freq in hz
void makeNoise(int beeps = 2, float speed = 20.0, float freq = 2000.0) {
    speakerPin.period(1.0 / freq);
    int beepCounter = 0;

    while (beepCounter < beeps) {
        speakerPin.write(0.5f);
        wait(1 / speed);
        speakerPin.write(0.0f);
        wait(1 / speed);
        beepCounter ++;
    }
}

// main() runs in its own thread in the OS
int main() {
    Stepper zAxis;

    int startMainMove = 0;
    int stepCounter = 0;

    pc.printf("Started the dipper\r\n");

    makeNoise();

    while (1) {
        if (joystickCenter) {
            wait(0.1);
            if (joystickCenter) {
                startMainMove = 1;
                while (joystickCenter);
            }
        }


        if (startMainMove == 1) {
            led1 = 1;
            int dippingHeight = zAxis.mmToSteps(100);
            int finishedUp = zAxis.moveSteps(dippingHeight, 0, zAxis.mmToSteps(2));

            if (finishedUp) {
                wait(60);
                int finishedDown = zAxis.moveSteps(dippingHeight, 1, zAxis.mmToSteps(0.5));


                if (finishedDown) {
                    startMainMove = 0;
                    led1 = 0;
                    makeNoise(5);
                }
            }
        }


        if (joystickUp) {
            wait(0.1);
            led2 = 1;
            while (joystickUp) {
                stepCounter += zAxis.move(0, zAxis.mmToSteps(5));
            }
            pc.printf("Counter is: %i\r\n", stepCounter);
            makeNoise(1);
        } else {
            led2 = 0;
        }


        if (joystickDown) {
            wait(0.1);
            led3 = 1;
            while (joystickDown) {
                stepCounter -= zAxis.move(1, zAxis.mmToSteps(5));
            }
            pc.printf("Counter is: %i\r\n", stepCounter);
            makeNoise(1);
        } else {
            led3 = 0;
        }
    }
}
