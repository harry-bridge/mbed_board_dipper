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
DigitalIn externalButton(p23, PullUp);

PwmOut speakerPin(p26);
DigitalOut pulsePin(p21);
DigitalOut dirPin(p22);

class Stepper {
public:

    int stepsPerRev;
    int stepsPerMM;

    int mmToSteps(float mmToMove) {
        return mmToMove * stepsPerMM;
    }

    // Constantly move steps
    // dir 1 for down dir 0 for up
    int move(int dir = 0, float speed = 1) {
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

        return 1; // Finished moving
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
    zAxis.stepsPerRev = 10000;
    zAxis.stepsPerMM = 2000;

    int startMainMove = 0;
    int stepCounter = 0;

    makeNoise();

    while (1) {
        if (joystickCenter || !externalButton) {
            wait(0.1);
            while (joystickCenter || !externalButton) {
                pc.printf("Button is pressed\r\n");
                startMainMove = 1;
            }
        }


        if (startMainMove == 1) {
            // How far to move down in mm
            int dippingHeight = 100;
            // Speeds to dip and retract in mm/s
            float dipSpeed = 1.0;
            float retractSpeed = 0.25;
            // How long to stay in the goo for
            int dwellTime = 180; // Wait 3 minutes


            pc.printf("Started the dipper\r\n");
            led1 = 1;
            makeNoise(1); // Tell the user it's going in
            int finishedDip = 0;
            int finishedRetract = 0;

            finishedDip = zAxis.moveSteps(zAxis.mmToSteps(dippingHeight), 0, zAxis.mmToSteps(dipSpeed));

            if (finishedDip == 1) {
                pc.printf("Finished Dip, waiting\r\n");
                wait(dwellTime);
                pc.printf("Retracting\r\n");
                makeNoise(1); // Tell the user it's coming back out
                finishedRetract = zAxis.moveSteps(zAxis.mmToSteps(dippingHeight), 1, zAxis.mmToSteps(retractSpeed));
            }

            if (finishedRetract == 1) {
                pc.printf("Finished retracting\r\n");
                startMainMove = 0;
                led1 = 0;
                makeNoise(5); // Tell the user it's finished dipping
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
