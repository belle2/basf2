#ifndef _Belle2_ROMessageMonitor_h
#define _Belle2_ROMessageMonitor_h

namespace Belle2 {

  class ROController;

  class ROMessageMonitor {

  public:
    ROMessageMonitor(ROController* con) : _con(con) {}

  public:
    void run();

  private:
    ROController* _con;

  };

}

#endif
