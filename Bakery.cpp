/**
 * @file Bakery.cpp
 * @author Petr Sopf (xsopfp00)
 * @author Jan Bartosek (xbarto92)
 * @brief Main program file, contains definitions of all functions and program logic
 */

#include <iostream>

#include "simlib.h"

//Number of shifts to be simulated
long numberOfShifts = 10;
//Number of furnaces to be used
long numberOfFurnaces = 4;

//Indicates, if Worker A should do bread as next
bool doBreadAsNext;
//Indicates, if Worker's A shift is over
bool isWorkerAShiftOver;
//Indicates, if Worker's B shift is over
bool isWorkerBShiftOver;
//Indicates, if import of baps is waiting
bool isBapDoughImportWaiting;

//Facility for baking bread
Facility bakeBread("Baking bread");
//Facility for baking rolls
Facility bakeRolls("Baking rolls");
//Facility for baking baps
Facility bakeBap("Baking bap");

//Store for all bread doughs waiting to be baked
Store breadDoughsWaiting(100);
//Store for all rolls doughs waiting to be baked
Store rollsDoughsWaiting(100);
//Store for all bap doughs waiting to be baked
Store bapDoughsWaiting(10);

//Store for all baked breads waiting to be pulled out of furnaces
Store bakedBreadWaiting(100);
//Store for all baked rolls waiting to be pulled out of furnaces
Store bakedRollsWaiting(100);
//Store for all baked baps waiting to be pulled out of furnaces
Store bakedBapsWaiting(10);

//Store of furnaces
Store furnace(4);

//Number of total breads done
long breadsDone = 0;
//Number of total rolls done
long rollsDone = 0;
//Number of total baps done
long bapsDone = 0;

class BapDoughImport : public Process {
    void Behavior() override {
        //Import arrives in exp(2 hours)
        Wait(Exponential(2 * 60));
        //Arrival of dough import
        isBapDoughImportWaiting = true;
    }
};

class BakingBread : public Process {
    void Behavior() override {
        //Bread is baking for 45 minutes
        Wait(45);
        //Baking of bread is completed
        Enter(bakedBreadWaiting, 1);
    }
};

class BakingRolls : public Process {
    void Behavior() override {
        //Rolls are baking for 13 minutes
        Wait(13);
        //Baking of roll is completed
        Enter(bakedRollsWaiting, 1);
    }
};

class BakingBap : public Process {
    void Behavior() override {
        //Baps are baking for 5 minutes
        Wait(5);
        //Baking of bap is completed
        Enter(bakedBapsWaiting, 1);
    }
};

class BreadDoughDries : public Process {
    void Behavior() override {
        //Bread doug dries for 45 minutes
        Wait(45);
        //Bread dough dries completed
        Enter(breadDoughsWaiting, 1);
    }
};

class RollsDoughDries : public Process {
    void Behavior() override {
        //Rolls doug dries for 45 minutes
        Wait(45);
        //Rolls dough dries completed
        Enter(rollsDoughsWaiting, 1);
    }
};

class WorkerAWork : public Process {
    void Behavior() override {
        isWorkerAShiftOver = false;
        doBreadAsNext = true;

        doAWorking:
        if (!isWorkerAShiftOver) {
            if (doBreadAsNext) { //Let's start making bread
                //Worker will do rolls as next
                doBreadAsNext = false;

                //Weighting raw materials
                Wait(Uniform(3, 5));

                //Mixer is mixing for 9 minutes
                Wait(9);

                //Make bread dough
                Wait(Uniform(60, 80) / 60);

                //Bread dough in kennel
                Wait(0.5);

                //Dough dries
                (new BreadDoughDries)->Activate();

                //Worker is going to look for another this to do
                goto doAWorking;
            } else { //Let's start making rolls
                //We will wo bread as next
                doBreadAsNext = true;

                //Weighting raw materials
                Wait(1);

                //Mixer is mixing for 9 minutes
                Wait(9);

                //Make balls from dough
                Wait(0.25);

                //Make rolls from balls
                Wait(0.25);

                //Bread dough in kennel
                Wait(0.5);

                //Dough dries
                (new RollsDoughDries)->Activate();

                //Worker is going to look for another this to do
                goto doAWorking;
            }
        }
    };
};


class WorkerBWork : public Process {
    void Behavior() override {
        //Wait one hour before starting
        Wait(60);

        isWorkerBShiftOver = false;

        Seize(bakeBread);

        doBWorking:
        if (bakedBreadWaiting.Used() > 0) { //Baked bread is waiting to be pulled out of furnace
            Leave(bakedBreadWaiting, 1);

            Wait(0.5);
            breadsDone += 1;
            Leave(furnace, 1);

            goto doBWorking;
        } else if (bakedRollsWaiting.Used() > 0) { //Baked rolls are waiting to be pulled out of furnace
            Leave(bakedRollsWaiting, 1);

            Wait(0.5);
            rollsDone += 1;
            Leave(furnace, 1);

            goto doBWorking;
        } else if (bakedBapsWaiting.Used() > 0) { //Baked baps are waiting to be pulled out of furnace
            Leave(bakedBapsWaiting, 1);

            Wait(0.5);
            bapsDone += 1;
            Leave(furnace, 1);

            goto doBWorking;
        } else if (isBapDoughImportWaiting) {
            isBapDoughImportWaiting = false;

            //Picking dough up delivery
            Wait(Uniform(1, 2));

            Wait(3);

            Enter(bapDoughsWaiting, 5);

            goto doBWorking;
        }
        if (isWorkerBShiftOver) { //Worker shift is over
            if (!furnace.Empty()) { //Worker must wait for everything to ba baked
                Wait(5);
                goto doBWorking;
            }
        } else {
            if (!furnace.Full() && bakeBread.Busy() && breadDoughsWaiting.Used() > 0) { //If there is available furnace and bread dough
                Leave(breadDoughsWaiting, 1);
                Release(bakeBread);
                Seize(bakeRolls);
                Enter(furnace, 1);

                Wait(1);

                (new BakingBread)->Activate();
                goto doBWorking;
            } else if (!furnace.Full() && bakeRolls.Busy() && rollsDoughsWaiting.Used() > 0) { //If there is available furnace and rolls dough
                Leave(rollsDoughsWaiting, 1);
                Release(bakeRolls);
                Seize(bakeBap);
                Enter(furnace, 1);

                Wait(1);

                (new BakingRolls)->Activate();
                goto doBWorking;
            } else if (!furnace.Full() && bakeBap.Busy() && bapDoughsWaiting.Used() > 0) { //If there is available furnace and baps dough
                Leave(bapDoughsWaiting, 1);
                Release(bakeBap);
                Seize(bakeBread);
                Enter(furnace, 1);

                Wait(1);

                (new BakingBap)->Activate();
                goto doBWorking;
            } else { //Current pastry can not be make, try to make another one
                Wait(1);

                if (bakeBread.Busy()) {
                    Release(bakeBread);
                    Seize(bakeRolls);
                } else if (bakeRolls.Busy()) {
                    Release(bakeRolls);
                    Seize(bakeBap);
                } else if (bakeBap.Busy()) {
                    Release(bakeBap);
                    Seize(bakeBread);
                }

                goto doBWorking;
            }
        }
    }
};

class WorkingShift : public Process {
    void Behavior() override {
        startShift:
        //Start of shift, clear everything
        breadDoughsWaiting.Clear();
        rollsDoughsWaiting.Clear();
        bapDoughsWaiting.Clear();
        furnace.Clear();
        furnace.SetCapacity(static_cast<unsigned long>(numberOfFurnaces));

        isBapDoughImportWaiting = false;
        bakedBreadWaiting.Clear();
        bakedRollsWaiting.Clear();
        bakedBapsWaiting.Clear();

        bakeBread.Clear();
        bakeRolls.Clear();
        bakeBap.Clear();

        //Start shift for A worker
        (new WorkerAWork)->Activate();

        //Later shift start for worker B
        (new WorkerBWork)->Activate();

        //Bap dough import
        (new BapDoughImport)->Activate();

        //six hours of shift for worker A
        Wait(6 * 60);

        //End of shift for worker A
        isWorkerAShiftOver = true;

        //3 hours left for Worker B
        Wait(3 * 60);

        //End of shift for worker B
        isWorkerBShiftOver = true;

        //15 hours until another shift
        Wait(15 * 60);

        --numberOfShifts;
        if (numberOfShifts > 0) {
            //Goto next shift
            goto startShift;
        }
    }
};

int main(int argc, char **argv) {
    if (argc > 3) {
        //check for too many arguments
        fprintf(stderr, "ERROR: Too many arguments!\n");
        exit(EXIT_FAILURE);
    }

    char *p;
    long numberOfShiftsPre = 10;

    if (argc > 1) { //Parse number of shifts argument
        numberOfShifts = strtol(argv[1], &p, 10);
        numberOfShiftsPre = numberOfShifts;

        if (*p || numberOfShifts < 1) {
            fprintf(stderr, "ERROR: First argument (number of shifts) can only be integer bigger then zero!\n");
            exit(EXIT_FAILURE);
        }
    }

    if (argc > 2) { //Parse number of furnaces argument
        numberOfFurnaces = strtol(argv[2], &p, 10);

        if (*p || numberOfShifts < 1) {
            fprintf(stderr, "ERROR: Second argument (number of furnaces) can only be integer bigger then zero!\n");
            exit(EXIT_FAILURE);
        }
    }

    Init(0);
    (new WorkingShift)->Activate();
    Run();

    //Print info
    printf("==================== Pekarna ====================\n");
    printf("Simulováno smen: %ld\n", numberOfShiftsPre);
    printf("Pocet peci: %ld\n", numberOfFurnaces);
    printf("Upeceno varek chleba: %ld (prumer %.2f na smenu)\n", breadsDone,
           breadsDone * 1.0 / numberOfShiftsPre * 1.0);
    printf("Upeceno varek rohliku: %ld (prumer %.2f na smenu)\n", rollsDone, rollsDone * 1.0 / numberOfShiftsPre * 1.0);
    printf("Upeceno varek kaiserek: %ld (prumer %.2f na smenu)\n", bapsDone, bapsDone * 1.0 / numberOfShiftsPre * 1.0);
    printf("=================================================\n");

    return 0;
}