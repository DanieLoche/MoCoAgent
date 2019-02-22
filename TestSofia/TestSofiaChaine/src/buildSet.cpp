#include <vector>
#include <string>
#include <iostream>
#include <iterator>
#include <fstream>
#include "buildSet.h"

using namespace std;

buildSet::buildSet() {

  // Ouverture fichier des tâches
  std::ifstream myFile("sorted.txt");

  // Test d'ouverture
  if (!myFile.is_open())
  {
      exit(EXIT_FAILURE);
  }

  string name;
  double tpsExeMoy;
  string str;
  std::getline(myFile, str); // skip the first line

  while (std::getline(myFile, str))
  {
      std::istringstream iss(str);
      if (!(iss >> name
                >> tpsExeMoy) )
      { cout << "FAIL !" << endl; break; } // error

      // Creation liste des tâches et des temps
      ordered_tasks.push_back(name);
      ordered_time.push_back(tpsExeMoy);
  }
}

std::vector<string> buildSet::distributionCrit (int nbr_crit){

    // Choix des tâches critques parmis toutes les tâches
    int size = all_tasks.size();

    // Vecteur mémoire des choix de tâches pour éviter les redondances de choix
    std::vector<int> choix_tache_mem;

    int i = 0;
    int var_test_choix;

    // Début choix
    while (i < nbr_crit) {

      var_test_choix = 0;

      // Choix d'une tâche aléatoirement
      int choose = rand() % size;

      for (int j = 0; j<i; j++){

        // Vérification si la tâche a déjà été choisie avant
        if (choix_tache_mem[j] == choose) {
          var_test_choix = 1;
          break;
        }
      }
      if (var_test_choix == 0) {

        i++; // Si elle n'a jamais été choisi on incrmente i et on
             // ajoute la tâche i au tâche choisi
        choix_tache_mem.push_back(choose);
        std::string temp = all_tasks[choose];
        all_crit_tasks.push_back(temp);
      }
    }

    return all_crit_tasks;
}

std::vector<string> buildSet::get_uncrit_tasks() {

  int var_test_in; // Variable de test tâche dans les critiques ou non

  for (int i = 0; i<all_crit_tasks.size(); i++) {

    // Parcours de l'ensemble des tâches
    var_test_in = 0;

    for (int j = 0; j<all_tasks.size(); j++) {

      // Parcours de l'ensemble des tâches critques
      if (all_tasks[i] == all_crit_tasks[j]) {
        // Si tâche est dans liste tache critique
        var_test_in = 1;
        break;
      }

    }
    if (var_test_in == 0) {
      // Si tâche pas dans liste tâche critque
      uncrit_tasks.push_back(long_choosen_task[i]);
    }
  }
  return uncrit_tasks;
}


std::vector<rtTaskInfosStruct> buildSet::get_infos_tasks(string input_file)
{
  // Ouverture fichier tasks.txt pour récupérer ses infos
  std::ifstream myFile(input_file);

  if (!myFile.is_open())
  {
      exit(EXIT_FAILURE);
  }

  string str;
  double tpsExeMoy;
  std::getline(myFile, str); // skip the first line
  while (std::getline(myFile, str))
  {
      rtTaskInfosStruct taskInfo;
      // Attribution des infos à chaque tâches
      std::istringstream iss(str);
      if (!(iss >> taskInfo.name
                >> tpsExeMoy
                >> taskInfo.path
                >> taskInfo.periodicity
                >> taskInfo.deadline
                >> taskInfo.affinity) )
      { cout << "FAIL !" << endl; break; } // error

      list_info_task.push_back(taskInfo);
  }

  return list_info_task;

}

void buildSet::buildInput(string name_chaine) {


  std::ofstream myFile("input" + name_chaine + ".txt", ios::out | ios::trunc);

  if (myFile)
  {

      // Ecriture de la premiere ligne
      myFile << "name path isHRT periodicity deadline affinity" << endl;

      // Ecriture des infos de chaque tâches critiques selectionnées
      for (int i = 0; i<all_crit_tasks.size(); i++) {
        // Parcours des tâches critiques choisies

        for (auto taskInfo = list_info_task.begin(); taskInfo != list_info_task.end(); ++taskInfo) {
          // Parcours de toutes les tâches lues dans tasks.txt

          if (all_crit_tasks[i] == taskInfo->name) {
            myFile << taskInfo->name << " " << taskInfo->path << taskInfo->name  << " 1 " << taskInfo->periodicity << " " << taskInfo->deadline << " " << taskInfo->affinity << endl;
          }
        }
      }

      // Ecriture des infos de chaque tâches non critiqes selectionnées
      for (int i = 0; i<uncrit_tasks.size(); i++) {
        // Parcours des tâches critiques choisies

        for (auto taskInfo = list_info_task.begin(); taskInfo != list_info_task.end(); ++taskInfo) {
          // Parcours de toutes les tâches lues dans tasks.txt

          if (uncrit_tasks[i] == taskInfo->name) {
            myFile << taskInfo->name << " " << taskInfo->path << taskInfo->name << " 0 " << taskInfo->periodicity << " " << taskInfo->deadline << " " << taskInfo->affinity << endl;
          }
        }
      }

      myFile.close();
  }

  else
  {
    std::cout << "Merde, création fichier input.txt à chier" << '\n';
    exit(EXIT_FAILURE);
  }

}
