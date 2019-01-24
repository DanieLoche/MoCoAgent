#include "tools.h"

class MacroTask
{
  private :
    rtTaskInfosStruct properties;

    int before();
    void proceed();
    int after();

  public :
    MacroTask();

};
