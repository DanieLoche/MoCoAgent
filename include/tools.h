#ifndef TOOLS_H
#define TOOLS_H

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>

#include <sched.h>
#include <alchemy/task.h>

using std::string;

struct rtTaskInfosStruct
{
    RT_TASK* task;
    char   name[64];
    string path;
    string task_args;

    bool isHardRealTime;
    int  periodicity;
    int  deadline;
    int  affinity;
} ;

#endif
