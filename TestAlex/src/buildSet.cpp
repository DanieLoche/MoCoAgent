#include <vector>
#include <string>
#include <iostream>
#include <iterator>
#include <fstream>
#include "buildSet.h"

using namespace std;

buildSet::buildSet() {
  // name path periodicity deadline affinity parameters
  rtTaskInfosStruct taskInfo;
  taskInfo.name = "exe1S"; taskInfo.path = "/null"; taskInfo.periodicity = 10; taskInfo.deadline = 50; taskInfo.affinity = 3; taskInfo.task_args = "";
  list_info_task.push_back(taskInfo);


  /*{"exe3S"; "/null/"; "30"; "70"; "3"; ""];
  ["exe4S"; "/null/"; "40"; "100"; "4"; ""];["exe5S"; "/null/"; "10"; "50"; "1"; ""];["exe6S"; "/null/"; "20"; "60"; "2"; ""];
  ["exe7S"; "/null/"; "30"; "70"; "3"; ""];["exe8S"; "/null/"; "40"; "100"; "4"; ""];["exe9S"; "/null/"; "10"; "50"; "1"; ""];
  ["exe10S"; "/null/"; "20"; "60"; "2"; ""];["exe1L"; "/null/"; "30"; "70"; "3"; ""];["exe2L"; "/null/"; "40"; "100"; "4"; ""];
  ["exe3L"; "/null/"; "10"; "50"; "1"; ""];["exe4L"; "/null/"; "20"; "60"; "2"; ""];["exe5L"; "/null/"; "30"; "700"; "3"; ""];
  ["exe6L"; "/null/"; "40"; "100"; "4"; ""];["exe7L"; "/null/"; "10"; "50"; "1"; ""];["exe8L"; "/null/"; "20"; "60"; "2"; ""];
  ["exe9L"; "/null/"; "30"; "70"; "3"; ""];["exe10L"; "/null/"; "40"; "100"; "4"; ""]];*/
}

std::vector<string> buildSet::distributionCrit (std::vector<string> long_task, std::vector<string> short_task, double nbr_long, double nbr_short, int crit_percent){

    // Choix des tâches longues parmis toutes les tâches
    distributionLong(long_task, nbr_long);

    // Choix des tâches courtes parmis toutes les tâches
    distributionShort(short_task, nbr_short);

    // Concaténation de toutes les tâches
    long_choosen_task.insert(long_choosen_task.end(), short_choosen_task.begin(), short_choosen_task.end());

    // Choix des tâches critques parmis toutes les tâches
    int size = long_choosen_task.size();
    // Définition du nombre de tâche à choisir
    int nbr_tach_crit = (int) size*crit_percent/100;
    // Vecteur mémoire des choix de tâches pour éviter les redondances de choix
    std::vector<int> choix_tache_mem;
    int i = 0;
    int var_test_choix;
    // Début choix
    while (i < nbr_tach_crit) {

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
        std::string temp = long_choosen_task[choose];
        all_crit_tasks.push_back(temp);
      }
    }

    return all_crit_tasks;
}

void buildSet::distributionLong(std::vector<string> long_task, double nbr_long){


  int size = long_task.size();
  std::vector<int> choix_tache_mem;
  int i = 0;            // Incrémentation
  int var_test_choix;   // Variable pour savoir si la tâche a dejà été choisie

  // Choix des tâches parmis toutes les tâches longues
  while (i < nbr_long) {

    // Choix des x tâches
    var_test_choix = 0;
    int choose = rand() % (size); // Choix d'une tâche aléatoirement

    for (int j = 0; j<i; j++) {
      // Vérification que la tâche n'a pas dejà été choisie
      if (choix_tache_mem[j] == choose) {
        // Si elle est choisi on recommence sans incrémenter i et on l'indique avec var_test_choix
        var_test_choix = 1;
        break;
      }
    }

    if (var_test_choix == 0) {
      i++; // Si elle n'a jamais été choisi on incrmente i et on
           // ajoute la tâche i au tâche choisi
      choix_tache_mem.push_back(choose);
      std::string temp = long_task[choose];
      long_choosen_task.push_back(temp);
    }

  }
}

void buildSet::distributionShort(std::vector<string> short_task, double nbr_short){


  int size = short_task.size();
  std::vector<int> choix_tache_mem;
  int i = 0;            // Incrémentation
  int var_test_choix;   // Variable pour savoir si la tâche a dejà été choisie comme critique

  // Repartition tache critique longue dans deux listes
  while (i < nbr_short) {

    // Choix des x tâches critque
    var_test_choix = 0;
    int choose = rand() % (size); // Choix d'une tâche aléatoirement

    for (int j = 0; j<i; j++) {
      // Vérification que la tâche n'a pas dejà été choisie

      if (choix_tache_mem[j] == choose) {
        // Si elle est choisi on recommence sans incrémenter i et on l'indique avec var_test_choix
        var_test_choix = 1;
        break;
      }
    }

    if (var_test_choix == 0) {
      i++; // Si elle n'a jamais été choisi on incrmente i et on
           // ajoute la tâche i au tâche choisi
      choix_tache_mem.push_back(choose);
      std::string temp = short_task[choose];
      short_choosen_task.push_back(temp);
    }

  }
}

std::vector<string> buildSet::get_uncrit_tasks() {

  int var_test_in;

  for (int i = 0; i<long_choosen_task.size(); i++) {
    var_test_in = 0;
    for (int j = 0; j<all_crit_tasks.size(); j++) {
      if (long_choosen_task[i] == all_crit_tasks[j]) {
        var_test_in = 1;
      }
    }
    if (var_test_in == 0) {
      uncrit_tasks.push_back(long_choosen_task[i]);
    }
  }
  return uncrit_tasks;
}

std::vector<rtTaskInfosStruct> buildSet::get_infos_tasks()
{

  return list_info_task;

}

void buildSet::buildInput() {

  std::ofstream myFile("input.txt", ios::out | ios::trunc);

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
            myFile << taskInfo->name << " " << taskInfo->path << " 1 " << taskInfo->periodicity << " " << taskInfo->deadline << " " << taskInfo->affinity << endl;
          }
        }
      }

      // Ecriture des infos de chaque tâches non critiqes selectionnées
      for (int i = 0; i<uncrit_tasks.size(); i++) {
        // Parcours des tâches critiques choisies
        for (auto taskInfo = list_info_task.begin(); taskInfo != list_info_task.end(); ++taskInfo) {
          // Parcours de toutes les tâches lues dans tasks.txt
          if (uncrit_tasks[i] == taskInfo->name) {
            myFile << taskInfo->name << " " << taskInfo->path << " 0 " << taskInfo->periodicity << " " << taskInfo->deadline << " " << taskInfo->affinity << endl;
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