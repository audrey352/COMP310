#ifndef SCHEDULER_MT_H
#define SCHEDULER_MT_H

#include "pcb.h"

void scheduler_start();  // create worker threads
void scheduler_enqueue(PCB *p);  // add a job
void scheduler_stop();  // called by quit

#endif