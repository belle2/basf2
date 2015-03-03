#ifndef _Belle2_HVControlDaemon_h
#define _Belle2_HVControlDaemon_h

#include <string>

namespace Belle2 {

  class HVControlCallback;

  class HVControlDaemon {

  public:
    HVControlDaemon(HVControlCallback* callback,
                    const std::string& filename)
      : m_callback(callback),
        m_filename(filename) {}
    ~HVControlDaemon() throw() {}

  public:
    void run();

  private:
    HVControlCallback* m_callback;
    std::string m_filename;

  };

}

#endif
