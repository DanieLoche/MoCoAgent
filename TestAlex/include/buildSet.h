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
    std::vector<string> short_choosen_task = std::vector<string>(0);
    std::vector<string> long_choosen_task = std::vector<string>(0);
    std::vector<string> all_crit_tasks;
    std::vector<string> uncrit_tasks;

  public:

    std::vector<structInfo> list_info_task;

    // Fonctions pour l'automatisation des sets de tâches
    std::vector<string> distributionCrit(std::vector<string> long_task, std::vector<string> short_task, int long_percent, int crit_percent);
    // Choix des tâches critiques parmis les tâches longues et courtes choisies ^

    void distributionLong(std::vector<string> long_task, int long_percent); // Choix des tâches longues
    void distributionShort(std::vector<string> short_task, int short_percent); // Choix des tâches courtes

    // Récupération des tâches non critiques
    std::vector<string> get_uncrit_tasks(); // Toujours lancer après distributionCrit

    // Récupération des informations de chaque tâches
    std::vector<structInfo> get_infos_tasks(string input_file);

    // Création fichier input.txt
    void buildInput();     // Toujours lancer après distributionCrit
};

#endif
