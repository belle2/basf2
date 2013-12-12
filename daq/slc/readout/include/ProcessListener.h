#ifndef _Belle2_ProcessListener_hh
#define _Belle2_ProcessListener_hh

#include "daq/slc/system/Fork.h"
#include "daq/slc/system/Mutex.h"

#include <string>

namespace Belle2 {

  class ROController;

  class ProcessListener {

  public:
    ProcessListener(ROController* con)
      : _con(con) {}
    ~ProcessListener() {}

  public:
    void run();

  private:
    ROController* _con;

  };

}

#endif

