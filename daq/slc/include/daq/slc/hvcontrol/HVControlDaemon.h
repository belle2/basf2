#ifndef _Belle2_HVControlDaemon_h
#define _Belle2_HVControlDaemon_h

namespace Belle2 {

  class HVControlCallback;

  class HVControlDaemon {

  public:
    HVControlDaemon(HVControlCallback* callback)
      : m_callback(callback) {}
    ~HVControlDaemon() throw() {}

  public:
    void run();

  private:
    HVControlCallback* m_callback;

  };

}

#endif
