#include <iostream>

#include "simlib.h"

long numberOfShifts;

bool doBreadAsNext;
bool isWorkerAShiftOver;
bool isWorkerBShiftOver;
bool isBapDoughImportWaiting;

bool isBakedBreadWaiting;
bool isBakedRollWaiting;
bool isBakedBapWaiting;

Facility BakeBread("Baking bread");
Facility BakeRolls("Baking rolls");
Facility BakeBap("Baking bap");

Store breadDoughsWaiting(100);
Store rollsDoughsWaiting(100);
Store bapDoughsWaiting(10);
Store furnace(4);

long breadsDone = 0;
long rollsDone = 0;
long bapsDone = 0;

class BapDoughImport : public Process {
    void Behavior() override {
        Wait(Exponential(2 * 60));
        //Arrival of dough import
        isBapDoughImportWaiting = true;
        printf("Přijel dovoz kaiserek.\n");
    }
};

class BakingBread : public Process {
    void Behavior() override {
        Wait(45);
        //Baking of bread is completed
        isBakedBreadWaiting = true;
    }
};

class BakingRolls : public Process {
    void Behavior() override {
        Wait(13);
        //Baking of roll is completed
        isBakedRollWaiting = true;
    }
};

class BakingBap : public Process {
    void Behavior() override {
        Wait(5);
        //Baking of bap is completed
        isBakedBapWaiting = true;
    }
};

class BreadDoughDries : public Process {
    void Behavior() override {
        //Bread doug dries
        Wait(45);
        printf("Těsto na chleba dokynulo.\n");

        Enter(breadDoughsWaiting, 1);
    }
};

class RollsDoughDries : public Process {
    void Behavior() override {
        //Bread doug dries
        Wait(45);
        printf("Těsto na rohlíky dokynulo.\n");

        Enter(rollsDoughsWaiting, 1);
    }
};

class WorkerAWork : public Process {
    void Behavior() override {
        printf("Worker A začala směna\n");
        isWorkerAShiftOver = false;
        doBreadAsNext = true;

        doAWorking:
        if (!isWorkerAShiftOver) {
            if (doBreadAsNext) { //Let's start making bread
                //Worker will do rolls as next
                doBreadAsNext = false;
                printf("Worker A jde připravovat chleba.\n");

                //Weighting raw materials
                Wait(Uniform(3, 5));
                printf("Worker A zvážil chleba.\n");

                //Mixer is mixing for 9 minutes
                printf("Worker A zapnul mixér na těsto chleba.\n");
                Wait(9);
                printf("Mixér dodělal těsto na chleba.\n");

                //Make bread dough
                Wait(Uniform(60, 80) / 60);
                printf("Worker A udělal těsto na chleba.\n");

                //Bread dough in kennel
                Wait(0.5);
                printf("Worker A dal těsto do kynárny.\n");

                //Dough dries
                (new BreadDoughDries)->Activate();

                //Worker is going to look for another this to do
                goto doAWorking;
            } else { //Let's start making rolls
                //We will wo bread as next
                doBreadAsNext = true;

                printf("Worker A jde připravovat rohlíky.\n");

                //Weighting raw materials
                Wait(1);

                //Start mixer
                printf("Worker A zapnul mixér na těsto rohlíku.\n");

                //Mixer is mixing for 9 minutes
                Wait(9);

                //Make balls from dough
                Wait(0.25);

                //Make rolls from balls
                Wait(0.25);

                //Bread dough in kennel
                Wait(0.5);

                //Release Worker A
                printf("Těsto na rohlíky je hotovo.\n");

                //Dough dries
                (new RollsDoughDries)->Activate();

                //Worker is going to look for another this to do
                goto doAWorking;
            }
        } else {
            printf("Worker A ukončil směnu.\n");
        }
    };
};


class WorkerBWork : public Process {
    void Behavior() override {
        //Wait one hour before starting
        Wait(60);
        printf("Worker B začala směna\n");
        isWorkerBShiftOver = false;

        Seize(BakeBread);

        doBWorking:
        if (isBakedBreadWaiting) {
            isBakedBreadWaiting = false;
            printf("Worker B jde vytáhnout chléb z pece.\n");
            Wait(0.5);
            breadsDone += 1;
            Leave(furnace, 1);
            printf("Chléb je na prodejně.\n");
            goto doBWorking;
        } else if (isBakedRollWaiting) {
            isBakedRollWaiting = false;
            printf("Worker B jde vytáhnout rohlíky z pece.\n");
            Wait(0.5);
            rollsDone += 1;
            Leave(furnace, 1);
            printf("Rohlíky jsou na prodejně.\n");
            goto doBWorking;
        } else if (isBakedBapWaiting) {
            isBakedBapWaiting = false;
            printf("Worker B jde vytáhnout kaiserky z pece.\n");
            Wait(0.5);
            bapsDone += 1;
            Leave(furnace, 1);
            printf("Kaiserky jsou na prodejně.\n");
            goto doBWorking;
        } else if (isBapDoughImportWaiting) {
            isBapDoughImportWaiting = false;

            //Picking dough up delivery
            printf("Worker B přebírá zásilku kaiserek.\n");
            Wait(Uniform(1, 2));

            printf("Worker B převzal zasílku kaiserek.\n");
            Wait(3);

            Enter(bapDoughsWaiting, 5);
            printf("Worker B uložil kaiserky do mrazáku.\n");
            goto doBWorking;
        } else if (!isWorkerBShiftOver) {
            if (!furnace.Full() && BakeBread.Busy() && breadDoughsWaiting.Used() > 0) {
                Leave(breadDoughsWaiting, 1);
                Release(BakeBread);
                Seize(BakeRolls);
                Enter(furnace, 1);

                printf("Worker B jde péct chléb.\n");
                Wait(1);

                (new BakingBread)->Activate();
                goto doBWorking;
            } else if (!furnace.Full() && BakeRolls.Busy() && rollsDoughsWaiting.Used() > 0) {
                Leave(rollsDoughsWaiting, 1);
                Release(BakeRolls);
                Seize(BakeBap);
                Enter(furnace, 1);

                printf("Worker B jde péct rohlíky.\n");
                Wait(1);

                (new BakingRolls)->Activate();
                goto doBWorking;
            } else if (!furnace.Full() && BakeBap.Busy() && bapDoughsWaiting.Used() > 0) {
                Leave(bapDoughsWaiting, 1);
                Release(BakeBap);
                Seize(BakeBread);
                Enter(furnace, 1);

                printf("Worker B jde péct kaiserky.\n");
                Wait(1);

                (new BakingBap)->Activate();
                goto doBWorking;
            } else {
                printf("Worker B nemá co péct.\n");
                Wait(5);

                if (BakeBread.Busy()) {
                    Release(BakeBread);
                    Seize(BakeRolls);
                } else if (BakeRolls.Busy()) {
                    Release(BakeRolls);
                    Seize(BakeBap);
                } else if (BakeBap.Busy()) {
                    Release(BakeBap);
                    Seize(BakeBread);
                }

                goto doBWorking;
            }
        } else {
            printf("Worker B ukončil směnu.\n");
        }
    }
};

class WorkingShift : public Process {
    void Behavior() override {
        startShift:
        breadDoughsWaiting.Clear();
        rollsDoughsWaiting.Clear();
        bapDoughsWaiting.Clear();
        furnace.Clear();

        isBapDoughImportWaiting = false;
        isBakedBreadWaiting = false;
        isBakedRollWaiting = false;
        isBakedBapWaiting = false;

        BakeBread.Clear();
        BakeRolls.Clear();
        BakeBap.Clear();

        //Start shift for A worker
        (new WorkerAWork)->Activate();

        //Later shift start for worker B
        (new WorkerBWork)->Activate();

        //Bap dough import
        (new BapDoughImport)->Activate();

        //Eight hours of shift
        Wait(8 * 60);

        //End of shift for worker A
        isWorkerAShiftOver = true;
        printf("Worker A skončila směna\n");

        //An hour left for Worker B
        Wait(60);

        //End of shift for worker B
        isWorkerBShiftOver = true;
        printf("Worker B skončila směna\n");

        Wait(15 * 60);

        --numberOfShifts;
        if (numberOfShifts > 0) {
            goto startShift;
        }
    }
};

int main(int argc, char **argv) {
    if (argc > 2) {
        fprintf(stderr, "ERROR: Too many arguments!\n");
        exit(EXIT_FAILURE);
    }

    char *p;
    numberOfShifts = strtol(argv[1], &p, 10);
    long numberOfShiftsPre = numberOfShifts;

    if (*p || numberOfShifts < 1) {
        fprintf(stderr, "ERROR: First argument can only be integer bigger then zero!\n");
        exit(EXIT_FAILURE);
    }

    Init(0);
    (new WorkingShift)->Activate();
    Run();

    printf("==================== Pekárna ====================\n");
    printf("Simulováno směn: %ld\n", numberOfShiftsPre);
    printf("Upečeno várek chleba: %ld (průměr %.2f na směnu)\n", breadsDone,
           breadsDone * 1.0 / numberOfShiftsPre * 1.0);
    printf("Upečeno várek rohlíků: %ld (průměr %.2f na směnu)\n", rollsDone, rollsDone * 1.0 / numberOfShiftsPre * 1.0);
    printf("Upečeno várek kaiserek: %ld (průměr %.2f na směnu)\n", bapsDone, bapsDone * 1.0 / numberOfShiftsPre * 1.0);
    printf("=================================================\n");
    return 0;
}