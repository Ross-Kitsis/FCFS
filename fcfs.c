//
//  main.c
//  A2
//
//  Created by Ross Kitsis on 2/17/2014.
//  Copyright (c) 2014 Ross Kitsis. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "sch-helpers.h"

process allProcesses[MAX_PROCESSES + 1];
int numberOfProcesses = 0;

process_queue ready; /*A fifo queue holding processes in the ready state*/
process_queue waiting; /*A fifo queue holding processes in the waiting state*/

int nextProcess;
int processesLeft;
int nextProcess = 0; /*Current index in process array*/

int time = 0; /*Variable to hold time for the simulation*/

process *preReady[MAX_PROCESSES + 1]; /*An array holding newly created processes to be added to the ready queue*/
int preReadyIndex = 0; /*Index of location in preReady array*/

process *CPU[NUMBER_OF_PROCESSORS];

/*
 * Move processes finished their IO bursts from the waiting queue to the ready queue
 */
void moveWaitingProcess()
{
    int i;
    int size = waiting.size;
    for(i = 0; i < size; i++)
    {
        process *front = waiting.front->data;
        dequeueProcess(&waiting);
        
        if(front->bursts[front->currentBurst].step == front->bursts[front->currentBurst].length)
        {
            /*Process has finished its IO burst, move on to next CPU burst and move to ready queue*/
            
            front -> currentBurst++;
            enqueueProcess(&ready, front);
        }else
        {
            /*Process has not finished its IO burst */
            enqueueProcess(&waiting, front);
        }
    }
}

/*
 *Move proccess which have finished their CPU burst to the waiting queue or terminate them if finished
 */
void clearRunningProcess()
{
    int i;
    for(i = 0; i < NUMBER_OF_PROCESSORS; i++)
    {
        if(CPU[i] != NULL)
        {
            process active = *CPU[i];
            if(active.bursts[active.currentBurst].step == active.bursts[active.currentBurst].length)
            {
                /*Process has finished its CPU burst */
                
                /*Move the process to its next burst*/
                active.currentBurst++;
                
                /*If current burst greater than total number of bursts the process has finished, otherwise process 
                 * going into IO burst, move to waiting queue and free the CPU
                 */
                
                if(active.currentBurst > active.numberOfBursts)
                {
                    numberOfProcesses--;
                    active.endTime = time;
                }else
                {
                    enqueueProcess(&waiting, &active);
                }
                CPU[i] = NULL;
            }
        }
            
    }
}
/*
 * Move all process whos arrival time equals the current time to the ready queue
 * Processes are already sorted by arrival time and all times have already been resolved
 */
void moveIncomingProcess()
{
    while(nextProcess < numberOfProcesses && allProcesses[nextProcess].arrivalTime == time)
    {
        printf("Enqueing process %d\n", allProcesses[nextProcess].pid);
        enqueueProcess(&ready,&allProcesses[nextProcess]);
        nextProcess++;
    }
}
/*
 *Reads all processes in the data file using the readProcess function in sch-helpers.
 */
void readAllProcesses()
{
    int status;
    while((status = readProcess(&allProcesses[numberOfProcesses])))
    {
        if(status == 1)
        {
            numberOfProcesses++;
        }
    }
    
}
int main(int argc, const char * argv[])
{
    //printf("Running");
    readAllProcesses();
    //printf("Read processes");
    if(numberOfProcesses > MAX_PROCESSES)
    {
        printf("Number of processes in data files exeeds maximum number of processes");
        exit(-1);
    }else if(numberOfProcesses == 0)
    {
        printf("No proccesses specified in data file");
        exit(-2);
    }
    
    qsort(allProcesses, numberOfProcesses, sizeof(process),compareByArrival); /*Sort processes by their arrival times */
   
    /*Initialize ready and waiting queues*/
    initializeProcessQueue(&ready);
    initializeProcessQueue(&waiting);
    
    processesLeft = numberOfProcesses;
    
    //printf("able to read process file, num process: %d", numberOfProcesses);
    while(1)
    {
        moveIncomingProcess(); /*Move incoming processes to the ready queue*/
        moveWaitingProcess(); /*Move waiting process to ready queue if finished IO burst*/
        clearRunningProcess(); /*Clear CPU of processes finished their CPU burst*/
        time++;
        if(time > 7000)
        {
            break;
        }
    }
    
}

