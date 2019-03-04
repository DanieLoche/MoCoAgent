#include "tools.h"




class TaskLauncher
{
  public :
    std::vector<rtTaskInfosStruct> tasksInfosList;


    TaskLauncher(string input_file, int ChaineID);

    void runTasks( );
    std::vector<rtTaskInfosStruct> readTasksList(string, int);

    void set_affinity (RT_TASK* task, int _aff);

    void printTasksInfos (/* std::vector<rtTaskInfosStruct> _myTasksInfos*/);
    void print_affinity(pid_t _pid);

};

extern void RunmcAgentMain(void *arg);
extern void TaskMain(void* arg);
extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);


#define SCHED_DEADLINE  6
#define SCHED_FLAG_RESET_ON_FORK	0x01

/* __NR_sched_setattr number */
#ifndef __NR_sched_setattr
#ifdef __x86_64__
#define __NR_sched_setattr      314
#endif

#ifdef __i386__
#define __NR_sched_setattr      351
#endif

#ifdef __arm__
#define __NR_sched_setattr      380
#endif

#ifdef __aarch64__
#define __NR_sched_setattr      274
#endif
#endif

/* __NR_sched_getattr number */
#ifndef __NR_sched_getattr
#ifdef __x86_64__
#define __NR_sched_getattr      315
#endif

#ifdef __i386__
#define __NR_sched_getattr      352
#endif

#ifdef __arm__
#define __NR_sched_getattr      381
#endif

#ifdef __aarch64__
#define __NR_sched_getattr      275
#endif
#endif

struct sched_attr {
    __u32 size;

    __u32 sched_policy;
    __u64 sched_flags;

    /* SCHED_NORMAL, SCHED_BATCH */
    __s32 sched_nice;

    /* SCHED_FIFO, SCHED_RR */
    __u32 sched_priority;

    /* SCHED_DEADLINE */
    __u64 sched_runtime;
    __u64 sched_deadline;
    __u64 sched_period;
};

int sched_setattr(pid_t pid,
              const struct sched_attr *attr,
              unsigned int flags)
{
    return syscall(__NR_sched_setattr, pid, attr, flags);
}

int sched_getattr(pid_t pid,
              struct sched_attr *attr,
              unsigned int size,
              unsigned int flags)
{
    return syscall(__NR_sched_getattr, pid, attr, size, flags);
}
