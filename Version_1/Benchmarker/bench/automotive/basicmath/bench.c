#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

time_t get_time()
{
  time_t current_time;
  current_time = time(NULL);
  if (current_time == ((time_t)-1))
  {
    (void) fprintf(stderr, "Erreur dans l'obtention du temps.\n");
    exit(EXIT_FAILURE);
  }
  return current_time;
}

double print_time(time_t start_time, time_t end_time)
{
  char* c_time_string;
  double total_time = difftime(end_time,start_time);
  return total_time;
}

int main(int argc, char *argv[]) {
  int i;
  int target = 1000;
  char* before = "./";
  char* name = argv[1];
  char* cmd;
  cmd = malloc(strlen(before)+strlen(name));
  strcpy(cmd, before);
  strcat(cmd, name);

  double seconds;
  double sum;
  
  for (i = 0 ; i<target ; i++)
  {
    time_t start_time = get_time();
    system(cmd);
    time_t end_time = get_time();
    seconds = print_time (start_time,end_time);
    sum += seconds;
  }

  sum = sum / target;
  printf("Temps d'exécution moyen : %.f secondes\n", sum);
  return 0;
}
