#ifndef _Belle2_ProcessListener_hh
#define _Belle2_ProcessListener_hh

namespace Belle2 {

  class ProcessController;

  class ProcessListener {

  public:
    ProcessListener(ProcessController* con)
      : m_con(con) {}
    ~ProcessListener() {}

  public:
    void run();

  private:
    ProcessController* m_con;

  };

}

#endif

