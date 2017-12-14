#ifndef Belle2_DQMSocketReader_h
#define Belle2_DQMSocketReader_h

#include <string>

#define MAXBUFSIZE 160000000

namespace Belle2 {

  class DQMViewCallback;
  class DQMSocketReader {

  public:
    DQMSocketReader(int port, DQMViewCallback* callback);
    ~DQMSocketReader();

  public:
    int init();
    void run();

  private:
    int m_port;
    DQMViewCallback* m_callback;

  };
}
#endif

