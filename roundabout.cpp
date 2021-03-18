#include "mpi.h"
#include <vector>
#include <iostream>
#include <queue>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <ctime>

// RUCH NA RONDZIE - MONTE CARLO
// MARCH 2020 MICHAL LOSKA & JERZY KLOS

#define NONE -1
constexpr int ROUNDABOUT_PART_SIZE = 4;
// best value for ITERATIONS is ROUNDABOUT_PART_SIZE * numberOfProcesses (eg. 20 for 5 procs.)
constexpr int ITERATIONS = 20;
constexpr double CAR_PROBABILITY = 0.5;

//  source /opt/nfs/config/source_mpich32.sh
//  mpic++ roundabout.cpp -o roundabout
//  mpiexec -np <amtOfProcesses> ./roundabout
//  eg. 4 processes = 4 entrances and 4 exits on the roundabout

void display (int ind, std::vector<int> & p_roundabout)
{
    printf("%d: [", ind);

    for(auto car = p_roundabout.begin(); car < p_roundabout.end(); ++car)
        car != p_roundabout.end() - 1 ? printf("%d,",*car) : printf("%d",*car);

    printf("]\n");
}

void moveCars(std::vector<int> & p_roundabout, int newCar)
{
    for(auto car = p_roundabout.end() - 1; car >= p_roundabout.begin(); --car)
        car == p_roundabout.begin() ? *car = newCar : *car = *(car - 1);
}


int main( int iCount, char ** pArgs ){
    srand( static_cast < unsigned int >( time( 0 ) ) );

    // --------------- MPI INITIALIZATION ------------------
    int numOfProcs, myId;
    MPI_Init(&iCount, &pArgs);
        //amount of currently running processes
    MPI_Comm_size(MPI_COMM_WORLD, &numOfProcs);
        //Id of the currently running Process
    MPI_Comm_rank(MPI_COMM_WORLD, &myId);
    // -----------------------------------------------------

    std::vector<int> currentRoundaboutPart(ROUNDABOUT_PART_SIZE, NONE);
    int incomingCar = NONE;
    int outgoingCar = NONE;
    int roundaboutOverlapped = 0;
    std::queue<int> carsAtEntrance;

    for (int i = 0; i < ITERATIONS; ++i)
    {

        if( ((double) rand() / (RAND_MAX)) < CAR_PROBABILITY)
        {
            carsAtEntrance.push(rand() % numOfProcs*ROUNDABOUT_PART_SIZE + (ROUNDABOUT_PART_SIZE-1));
        }

        // HANDLE THE FIRST PROCESS (FIRST PART OF THE ROUNDABOUT)
        // No Recv for the first process in the first iteration (nothing to receive from proc 4)
        if(i <= (ITERATIONS - 1) and roundaboutOverlapped and myId == 0)
        {
            MPI_Recv(&incomingCar, 1, MPI_INT, numOfProcs - 1, numOfProcs, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else if(i <= (ITERATIONS - 1) and myId != 0)
        {
            MPI_Recv(&incomingCar, 1, MPI_INT, myId - 1, myId, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        display(myId, currentRoundaboutPart);

        outgoingCar = currentRoundaboutPart[ROUNDABOUT_PART_SIZE-1];

        //Let a new car into a roundabout if possible
        if(incomingCar == NONE and carsAtEntrance.size() > 0)
        {
            incomingCar = carsAtEntrance.back();
            carsAtEntrance.pop();
        }
        //cars moving
        moveCars(currentRoundaboutPart, incomingCar);
        //last car in the part of the roundabout moves or leaves the roundabout
        if (outgoingCar <= (ROUNDABOUT_PART_SIZE-1) + (myId * ROUNDABOUT_PART_SIZE))
            outgoingCar = NONE;

        // HANDLE THE LAST PROCESS (LAST PART OF THE ROUNDABOUT)
        if (myId == (numOfProcs - 1) and i <= (ITERATIONS - 2))
        {
            MPI_Send(&outgoingCar, 1, MPI_INT, 0, myId+1, MPI_COMM_WORLD);
            roundaboutOverlapped = 1;
        }
        else if(myId != (numOfProcs - 1) and i <= (ITERATIONS - 1))
        {
            MPI_Send(&outgoingCar, 1, MPI_INT, myId + 1, myId + 1, MPI_COMM_WORLD);
        }

        if(i == 0 ){
            MPI_Bcast(&roundaboutOverlapped, 1, MPI_INT, numOfProcs-1, MPI_COMM_WORLD);
        }
    }
    MPI_Finalize();
    if(myId == 0)
    {
        printf("Amount Of Iterations: %d\n", ITERATIONS);
        printf("Amount Of Entrances/Exits: %d\n", numOfProcs);
        printf("Probability Of An Incoming Car: %d%\n", static_cast<int>(CAR_PROBABILITY*100));
    }
    printf("ENTRANCE %d: %d AWAITING CARS\n", myId, carsAtEntrance.size());
}
