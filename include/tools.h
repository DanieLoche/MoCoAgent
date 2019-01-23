#ifndef TOOLS_H
#define TOOLS_H

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <string>

#include <alchemy/task.h>

using std::string;

struct rtTaskInfos
{
    RT_TASK task;
    string name;
    string path;

    bool isHardRealTime;
    int periodicity;
    int affinity[8];
} ;

#endif
