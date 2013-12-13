#ifndef _Belle2_StdOutListener_hh
#define _Belle2_StdOutListener_hh

namespace Belle2 {

  class ProcessController;

  class StdOutListener {

  public:
    StdOutListener(ProcessController* con, int pipe[2])
      : _con(con) {
      _pipe[0] = pipe[0];
      _pipe[1] = pipe[1];
    }
    ~StdOutListener() {}

  public:
    void run();

  private:
    ProcessController* _con;
    int _pipe[2];

  };

}

#endif

