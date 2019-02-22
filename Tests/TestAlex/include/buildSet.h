#ifndef SET_H
#define SET_H

#include <vector>
#include <string>
#include <sys/sysinfo.h>
#include "tools.h"

using std::string;

class buildSet
{
  private:
    std::vector<string> all_tasks;
    std::vector<string> all_crit_tasks;
    std::vector<string> uncrit_tasks;
    std::vector<string> ordered_tasks;
    std::vector<double> ordered_time;

  public:

    buildSet();

    std::vector<rtTaskInfosStruct> list_info_task;

    // Fonctions pour l'automatisation des sets de tâches
    std::vector<string> distributionCrit(int crit_percent);
    // Choix des tâches critiques parmis les tâches longues et courtes choisies ^

    // Récupération des tâches non critiques
    std::vector<string> get_uncrit_tasks(); // Toujours lancer après distributionCrit

    // Récupération des informations de chaque tâches
    std::vector<rtTaskInfosStruct> get_infos_tasks(string input_file);

    // Création fichier input.txt
    void buildInput();     // Toujours lancer après distributionCrit
};

#endif
