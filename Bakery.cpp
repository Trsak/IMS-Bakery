#include <iostream>

#define I_REALLY_KNOW_HOW_TO_USE_WAITUNTIL

#include "simlib.h"

bool isMixerUsed = false;

bool isBreadDoughMixed = false;
bool isRollsDoughMixed = false;

bool isWorkerAShiftOver = false;
bool isWorkerBShiftOver = false;

Facility StartWithBread("Start making bread");
Facility Mixer("Mixer");

Facility WorkerA("Worker A");
Facility WorkerB("Worker B");

class BreadDoughDries : public Process {
    void Behavior() override {
        //Bread dough dries
        Wait(45);
        printf("Těsto na chleba dokynulo.\n");
    }
};

class RollDoughDries : public Process {
    void Behavior() override {
        //Bread dough dries
        Wait(45);
        printf("Těsto na rohlíky dokynulo.\n");
    }
};

class BreadWork : public Process {
    void Behavior() override {
        //Seize mixer
        isMixerUsed = true;
        Seize(Mixer);

        //Mixer is mixing for 9 minutes
        Wait(9);

        //Mixing is done, seize Worker A
        printf("Mixování chleba je hotovo.\n");
        isBreadDoughMixed = true;

        //Release mixer
        Release(Mixer);
        isMixerUsed = false;
    }
};

class RollsWork : public Process {
    void Behavior() override {
        //Seize mixer
        isMixerUsed = true;
        Seize(Mixer);

        //Mixer is mixing for 9 minutes
        Wait(9);

        //Mixing is done, seize Worker A
        printf("Mixování rohlíku je hotovo.\n");
        isRollsDoughMixed = true;

        //Release mixer
        Release(Mixer);
        isMixerUsed = false;
    }
};

class WorkerAWork : public Process {
    void Behavior() override {
        waiting:
        while (!WorkerA.Busy()) {
            if (isBreadDoughMixed) {
                //Worker takes dough out of mixer
                Seize(WorkerA, 4);

                //Make bread dough
                Wait(Uniform(60, 80) / 60);

                //Bread dough in kennel
                Wait(0.5);

                //Release Worker A
                printf("Těsto na chleba je hotovo.\n");
                Release(WorkerA);

                //Bread doug dries
                (new BreadDoughDries)->Activate();
                isBreadDoughMixed = false;
                goto waiting;
            } else if (isRollsDoughMixed) {
                //Worker takes dough out of mixer
                Seize(WorkerA, 4);

                //Make balls from dough
                Wait(0.25);

                //Make rolls from balls
                Wait(0.25);

                //Bread dough in kennel
                Wait(0.5);

                //Release Worker A
                printf("Těsto na rohlíky je hotovo.\n");
                Release(WorkerA);

                //Rolls doug dries
                (new RollDoughDries)->Activate();
                isRollsDoughMixed = false;
                goto waiting;
            } else if (isWorkerAShiftOver) {
                printf("Worker A ukončil směnu.\n");
                break;
            } else if (!StartWithBread.Busy()) { //Let's start making bread
                Seize(StartWithBread);
                Seize(WorkerA, 2);
                printf("Worker A jde připravovat chleba.\n");

                //Weighting raw materials
                Wait(Uniform(3, 5));

                //Wait for mixer
                Release(WorkerA);
                WaitUntil(!isMixerUsed);

                //Start mixer
                printf("Worker A zapnul mixér na těsto chleba.\n");
                (new BreadWork)->Activate();
                goto waiting;
            } else { //Let's start making rolls
                Release(StartWithBread);
                Seize(WorkerA, 1);

                printf("Worker A jde připravovat rohlíky.\n");

                //Weighting raw materials
                Wait(1);

                //Wait for mixer
                Release(WorkerA);
                WaitUntil(!isMixerUsed);

                //Start mixer
                printf("Worker A zapnul mixér na těsto rohlíku.\n");
                (new RollsWork)->Activate();
                goto waiting;
            }
        }
    };
};

class WorkingShift : public Process {
    void Behavior() override {
        printf("Začala směna\n");
        (new WorkerAWork)->Activate();

        //Eight hours of shift
        Wait(8 * 60);

        //End of shift for worker A
        isWorkerAShiftOver = true;
        printf("Zaměstnanci A skončila směna\n");

        //An hour left for Worker B
        Wait(60);

        //End of shift for worker B
        isWorkerBShiftOver = true;
        printf("Zaměstnanci B skončila směna\n");
    }
};

int main() {
    Init(0);
    (new WorkingShift)->Activate();
    Run();
    return 0;
}