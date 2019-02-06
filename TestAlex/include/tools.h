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
using std::cout;
using std::endl;
using std::cin;

struct structInfo
{
    string name;
    string path;
    string task_args;

    int  periodicity;
    int  deadline;
    int  affinity;
} ;

#endif
