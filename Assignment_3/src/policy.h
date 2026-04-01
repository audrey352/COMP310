// policy.h
#ifndef POLICY_H
#define POLICY_H
#include <stdbool.h>
#include <stdlib.h> 
#include <string.h>
#include <stdio.h>
#include "readyqueue.h"
#include "scheduler.h"

SchedulerContext* get_scheduler_context(const char* policy_name);

#endif