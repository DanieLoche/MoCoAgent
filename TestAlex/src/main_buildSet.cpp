#include "buildSet.h"
#include "tools.h"
#include <string>
#include <iostream>
#include <vector>
#include <iterator>

int main()
{
  string input_file = "tasks.txt";
  buildSet bS;

  std::vector<string> long_task;
  std::vector<string> short_task;

  for (int i = 0; i<10; i++) {
    std::string l = "exe" + std::to_string(i+1) + "L";
    std::string s = "exe" + std::to_string(i+1) + "S";
    long_task.push_back(l);
    short_task.push_back(s);
  }

  // Définition des listes comportant les tâches longue et courte
  std::vector<string> all_crit_tasks = bS.distributionCrit(long_task, short_task, 50, 25);

  // Affichage des tâches critiques choisies
  std::cout << "Toutes les tâches critiques choisies : " << '\n';
  for (int i = 0; i<all_crit_tasks.size() ; i++){

      std::cout << all_crit_tasks[i] << " - ";
  }
  std::cout << "" << '\n';

  // Affichage des tâches non critiques choisies
  std::vector<string> uncrit_tasks = bS.get_uncrit_tasks();
  std::cout << "Toutes les tâches non critiques choisies : " << '\n';
  for (int i = 0; i<uncrit_tasks.size() ; i++){

      std::cout << uncrit_tasks[i] << " - ";
  }
  std::cout << "" << '\n';

  // Récupération infos tâches
  std::vector<structInfo> info_task = bS.get_infos_tasks(input_file);

  // Edition du fichier input.txt
  bS.buildInput();

  return 0;
}
