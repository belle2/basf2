#ifndef _Belle2_ProcessListener_hh
#define _Belle2_ProcessListener_hh

#include "daq/slc/system/Fork.h"
#include "daq/slc/system/Mutex.h"

#include <string>

namespace Belle2 {

  class ProcessController;

  class ProcessListener {

  public:
    ProcessListener(ProcessController* con)
      : _con(con) {}
    ~ProcessListener() {}

  public:
    void run();

  private:
    ProcessController* _con;

  };

}

#endif

