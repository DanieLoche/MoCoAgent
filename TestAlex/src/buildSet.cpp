#include <vector>
#include <string>
#include <iostream>
#include <iterator>
#include <fstream>
#include "buildSet.h"

using namespace std;

std::vector<string> buildSet::distributionCrit (std::vector<string> long_task, std::vector<string> short_task, int long_percent, int crit_percent){

    int short_percent = 100 - long_percent;

    // Choix des tâches longues parmis toutes les tâches
    distributionLong(long_task, long_percent);

    // Affichage des tâches longues choisies
    std::cout << "Longues tâches choisies : " << '\n';
    for (int i = 0; i<long_choosen_task.size(); i++){

      std::cout << long_choosen_task[i] << " - ";
    }
    std::cout << "" << '\n';

    // Choix des tâches courtes parmis toutes les tâches
    distributionShort(short_task, short_percent);

    // Affichage des tâches courtes choisies
    std::cout << "Courtes tâchse choisies : " << '\n';
    for (int i = 0; i<short_choosen_task.size(); i++){

      std::cout << short_choosen_task[i] << " - ";
    }
    std::cout << "" << '\n';

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

void buildSet::distributionLong(std::vector<string> long_task, int long_percent){


  int size = long_task.size();
  int nbr_tach = (int) size*long_percent/100;
  std::vector<int> choix_tache_mem;
  int i = 0;            // Incrémentation
  int var_test_choix;   // Variable pour savoir si la tâche a dejà été choisie

  // Choix des tâches parmis toutes les tâches longues
  while (i < nbr_tach) {

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

void buildSet::distributionShort(std::vector<string> short_task, int short_percent){


  int size = short_task.size();
  int nbr_tach = (int) size*short_percent/100;
  std::vector<int> choix_tache_mem;
  int i = 0;            // Incrémentation
  int var_test_choix;   // Variable pour savoir si la tâche a dejà été choisie comme critique

  // Repartition tache critique longue dans deux listes
  while (i < nbr_tach) {

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

std::vector<structInfo> buildSet::get_infos_tasks(string input_file)
{
  std::ifstream myFile(input_file);
  if (!myFile.is_open())
  {
      std::cout << "Merde, ouveture fichier tasks.txt à chier" << '\n';
      exit(EXIT_FAILURE);
  }

  string str;
  std::getline(myFile, str); // skip the first line
  while (std::getline(myFile, str))
  {
      structInfo taskInfo;
      std::istringstream iss(str);
      cout << "Ligne traitée : " << str << endl;
      if (!(iss >> taskInfo.name
                >> taskInfo.path
                >> taskInfo.periodicity
                >> taskInfo.deadline) )
      { cout << "FAIL !" << endl; break; } // error

      list_info_task.push_back(taskInfo);
  }

  return list_info_task;

}

void buildSet::buildInput() {

  std::ofstream myFile("input.txt", ios::out | ios::trunc);

  if (myFile)
  {
      std::cout << "Fichier ouvert" << '\n';
      std::cout << '\n';

      // Ecriture de la premiere ligne
      myFile << "name path isHRT periodicity deadline" << endl;

      // Ecriture des infos de chaque tâches critiques selectionnées
      for (int i = 0; i<all_crit_tasks.size(); i++) {
        // Parcours des tâches critiques choisies
        for (auto taskInfo = list_info_task.begin(); taskInfo != list_info_task.end(); ++taskInfo) {
          // Parcours de toutes les tâches lues dans tasks.txt
          if (all_crit_tasks[i] == taskInfo->name) {
            std::cout << "Eciture tâche critiqe : " << taskInfo->name << '\n';
            myFile << taskInfo->name << " " << taskInfo->path << " 1 " << taskInfo->periodicity << " " << taskInfo->deadline << endl;
          }
        }
      }

      // Ecriture des infos de chaque tâches non critiqes selectionnées
      for (int i = 0; i<uncrit_tasks.size(); i++) {
        // Parcours des tâches critiques choisies
        for (auto taskInfo = list_info_task.begin(); taskInfo != list_info_task.end(); ++taskInfo) {
          // Parcours de toutes les tâches lues dans tasks.txt
          if (uncrit_tasks[i] == taskInfo->name) {
            std::cout << "Eciture tâche non critiqe : " << taskInfo->name << '\n';
            myFile << taskInfo->name << " " << taskInfo->path << " 0 " << taskInfo->periodicity << " " << taskInfo->deadline << endl;
          }
        }
      }

      std::cout << '\n';
      myFile.close();
      std::cout << "Fichier fermé" << '\n';
  }

  else
  {
    std::cout << "Merde, création fichier input.txt à chier" << '\n';
    exit(EXIT_FAILURE);
  }

}
