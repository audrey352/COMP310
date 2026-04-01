// policy.h
#ifndef POLICY_H
#define POLICY_H
#include <stdbool.h>
#include "readyqueue.h"

SchedulerContext* get_scheduler_context(const char* policy_name);

#endif