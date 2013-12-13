#ifndef _Belle2_ProcessListener_hh
#define _Belle2_ProcessListener_hh

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

