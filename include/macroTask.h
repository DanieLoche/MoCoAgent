#include "tools.h"

class MacroTask
{
  private :
    rtTaskInfos properties;

    int before();
    void proceed();
    int after();

  public :
    MacroTask();

};
