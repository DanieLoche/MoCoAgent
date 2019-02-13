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

std::vector<string> buildSet::distributionCrit (double nbr_long, double nbr_short, int crit_percent){
    // Vecteur des tâches longues et courtes
    std::vector<string> long_task;
    std::vector<string> short_task;

    // Calcul de la moyenne des temps moyens
    double sum = 0;
    for (int i = 0; i<ordered_time.size(); i++) {
      sum += ordered_time[i];
    }
    double moy = sum/ordered_time.size();

    // Répartition dans les deux listes de tâches longues et courtes
    for (int i = 0; i<ordered_time.size(); i++) {
      // Si le tps est en dessous de la moyenne, direction les courtes
      if (ordered_time[i] < moy) {
        short_task.push_back(ordered_tasks[i]);
      }
      // Sinon les longues
      else {
        long_task.push_back(ordered_tasks[i]);
      }
    }

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

  int var_test_in; // Variable de test tâche dans les critiques ou non

  for (int i = 0; i<all_crit_tasks.size(); i++) {

    // Parcours de l'ensemble des tâches
    var_test_in = 0;

    for (int j = 0; j<long_choosen_task.size(); j++) {

      // Parcours de l'ensemble des tâches critques
      if (long_choosen_task[i] == all_crit_tasks[j]) {
        // Si tâche est dans liste tache critique
        break;
        var_test_in = 1;
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
