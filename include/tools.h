#ifndef TOOLS_H
#define TOOLS_H

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include<algorithm>

#include <alchemy/task.h>

using std::string;

struct rtTaskInfos
{
    char name[64];
    string path;

    bool isHardRealTime;
    int periodicity;
    int deadline;
    int affinity;
} ;

#endif
