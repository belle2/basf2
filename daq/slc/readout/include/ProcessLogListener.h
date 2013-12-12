#ifndef _Belle2_ProcessLogListener_h
#define _Belle2_ProcessLogListener_h

namespace Belle2 {

  class ProcessController;

  class ProcessLogListener {

  public:
    ProcessLogListener(ProcessController* con) : _con(con) {}

  public:
    void run();

  private:
    ProcessController* _con;

  };

}

#endif
