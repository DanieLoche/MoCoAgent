#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <vector>
#include <chrono>

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include <alchemy/task.h>
#include <alchemy/timer.h>

#include <xenomai/init.h>


using namespace std::chrono;
using namespace std;

extern "C" {
	int basicmath_small   (int argc, char *argv[]);
	int basicmath_large   (int argc, char *argv[]);
	int bitcount_func     (int argc, char *argv[]);
	int qsort_small       (int argc, char *argv[]);
	int qsort_large       (int argc, char *argv[]);
	int susan             (int argc, char *argv[]);
	int djpeg_func        (int argc, char *argv[]);
	int cjpeg_func        (int argc, char *argv[]);
	int typeset_func      (int argc, char *argv[]);
	int dijkstra_small    (int argc, char *argv[]);
	int dijkstra_large    (int argc, char *argv[]);
	int patricia          (int argc, char *argv[]);
	int stringsearch_small(int argc, char *argv[]);
	int stringsearch_large(int argc, char *argv[]);
	int blowfish          (int argc, char *argv[]);
	int rijndael          (int argc, char *argv[]);
	int sha               (int argc, char *argv[]);
	int rawdaudio         (int argc, char *argv[]);
	int rawcaudio         (int argc, char *argv[]);
	int crc               (int argc, char *argv[]);
	int fft               (int argc, char *argv[]);
	int gsm_func          (int argc, char *argv[]);
}

/*
* make sure everybody is in the same session and that
* we have registry sharing.
*/
void do_xeno_init(void) {
   const char *args[] = {
      "program",
      "--shared-registry",
      "--session=test",
      NULL
   } ;
   const char **argv = args ;
   int argc = (sizeof args/sizeof args[0])-1 ; /* exclude NULL */

   xenomai_init(&argc,(char * const **)&argv) ;
}
#define XENO_INIT() do_xeno_init()

std::string trim(const std::string& str, const std::string& whitespace = " ")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

std::string reduce(const std::string& str,
                   const std::string& fill = " ",
                   const std::string& whitespace = " \t")
{
    // trim first
    auto result = trim(str, whitespace);

    // replace sub ranges
    auto beginSpace = result.find_first_of(whitespace);
    while (beginSpace != std::string::npos)
    {
        const auto endSpace = result.find_first_not_of(whitespace, beginSpace);
        const auto range = endSpace - beginSpace;

        result.replace(beginSpace, range, fill);

        const auto newStart = beginSpace + fill.length();
        beginSpace = result.find_first_of(whitespace, newStart);
    }

    return result;
}

int _argc = 0;
std::vector<char*> _argv;

void manageArguments(char* argv[], string arguments)
{

	arguments = reduce(arguments);
	int chr;
	for (chr = 0; arguments[chr] != '\0'; chr++)
	{
	  if (arguments[chr] == '\n' || arguments[chr] == '\r')
		 arguments[chr] = '\0';
	}

	std::istringstream iss( arguments);
	cout << "\nManaging arguments : [" << arguments << "]" << endl;
	string token;
	int nextStr = 0;
	//_argc = 1;
	while (getline(iss, token, ' '))
	{
		token = reduce(token);
		cout << "Managing token [" << token << "]." << endl;
		if (token == "<")
			nextStr = 1;
		else if (token == ">")
			nextStr = 2;
		else if (token != "")
		{
			if (nextStr == 1)
			{
				cout << "Tried to change StdIn to : " << token << endl;
			}
			else if (nextStr == 2)
			{
				cout << "Tried to change StdOut to : " << token << endl;
			}
			else
			{
				char *arg = new char[token.size() +1];  // +1
				copy(token.begin(), token.end(), arg);
				arg[token.size()]= '\0';
				cout << "token : [" << token << "] (" << token.size() << ") copied to [" << arg << "] (" << strlen(arg) << ")." << endl;

				_argv.push_back(arg);
				_argc++;
			}
			nextStr = 0;
		}
	}
	_argv.push_back(0);
}

int main(int argc, char *argv[])
{
	printf("Debut test\n");

	const char* taskNames[3] = {"task0", "task1", "task2"};

   setvbuf(stdout,NULL,_IOLBF,4096) ;

   for (int i = 0 ; i < 3 ; ++i)
   {
      pid_t pid = fork() ;

      if (pid < 0) {
         perror("fork") ;
         continue ;
      }
      else if (pid) // fork OK, we're parent
      {
         //sleep(1); // wait a little before going through another fork.
      } else { // child process
         RT_TASK _t;
         int ret = 0 ;

         printf("calling xenomai_init()\n") ;
         XENO_INIT() ;
         rt_printf("process for %s running, pid %lu\n",
         taskNames[i],getpid()) ;
         ret = rt_task_shadow(&_t, taskNames[i], 50, 0);
         rt_printf("shadow task %s returns %d\n", taskNames[i], ret) ;
         ret = rt_task_sleep(1e9*30) ;
         rt_printf("sleep returned status %d\n",ret) ;
         exit(EXIT_SUCCESS);
      }

   }

   // Only "main" process goes here.

   printf("launching done, sleeping 5\n") ;
   sleep(5) ;
   printf("resuming, binding...\n") ;

   printf("dumping the registry\n") ;
   system("find /run/xenomai") ; // see what the registry is looking like

   XENO_INIT() ;

   for (int i = 0 ; i < 3 ; ++i)
   {
      RT_TASK _t;
      int ret ;
      printf("binding to %s\n",taskNames[i]) ;
      ret = rt_task_bind(&_t, taskNames[i], TM_NONBLOCK); // <== HERE. TM_NONBLOCK, TM_INFINITE or a time value, changes nothing.... never goes here.
      if (ret) printf("Error : could not bind task. Error #%d\n", ret);
      else printf("Bind to task %s done.\n", taskNames[i]);
   }
   /* Do stuff here on the binded tasks */
   exit(EXIT_SUCCESS);



	  int i = 0;
	  _argv.push_back(argv[0]);
	  _argc++;
	  for (i=1; i < argc; i++)
	  {
		  std::string toPrint;
		  if (argv[i]==NULL)toPrint = "null";
		  else toPrint = argv[i];
		  std::cerr << "Arg #" << i << " = " << toPrint << " ; ";
		  manageArguments(argv, argv[i]);
	  }
	  std::cerr << " with (" << _argc << ") elements." << std::endl;
	  cerr << "Bilan paramètres donnés : \n";
	  for (i=0; i < (int)_argv.size()-1; i++)
	  {
		  std::cerr << "Arg #" << i << " = " << _argv[i] << " ; ";
	  }
	  cerr << endl;
	  cerr << "Total : " << _argv.size()-1 << " arguments." << endl;

	int ret = 0;
	// auto start = high_resolution_clock::now();
	std::cerr << "Reading timer Start." << std::endl;
	auto start = rt_timer_read();

	std::cerr << "Shadowing..." << std::endl;
	RT_TASK task;
	ret += rt_task_shadow(&task, "Testing", 50, 0);
	std::cerr << "Task shadowed." << std::endl;
	auto mid = rt_timer_read();
 ret += basicmath_small( _argv.size()-1, &_argv[0]);
// ret += basicmath_large( argc, argv);
//	ret += bitcount_func( argc, argv);
//	ret += qsort_small(  _argv.size()-1, &_argv[0]) ;
//	ret += qsort_large( argc, argv) ;
//	ret += susan( argc, argv);
//	ret += djpeg_func( argc, argv);
//	ret += cjpeg_func( _argv.size()-1, &_argv[0]);
//	ret += dijkstra_small( argc, argv) ;
//	ret += dijkstra_large( argc, argv) ;
//	ret += patricia( argc, argv);
//	ret += stringsearch_small( argc, argv);
//	ret += stringsearch_large( argc, argv);
//	ret += blowfish( _argv.size()-1, &_argv[0]);
//	ret += rijndael( argc, argv);
//	ret += sha( argc, argv);
// ret += rawdaudio( argc, argv);

// ret += rawcaudio( argc, argv);
//	ret += crc( argc, argv);
//	ret += fft( argc, argv) ;
//	ret += typeset_func( argc, argv);
//	ret += gsm_func ( argc, argv);
	auto end = rt_timer_read();
	printf("Fin test : %d.\n", ret);
	auto duration = end - start;
	auto midTime = mid-start; //duration_cast<microseconds>(mid - start);
	std::cout << "Total duration = " << duration << "ns. Miduration = " << midTime <<"ns." << std::endl;
	return ret;

}
