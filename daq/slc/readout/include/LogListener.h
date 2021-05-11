#ifndef _Belle2_LogListener_hh
#define _Belle2_LogListener_hh

namespace Belle2 {

  class ProcessController;

  class LogListener {

  public:
    LogListener(ProcessController* con, int pipe[2], bool enableUnescapeNewlines = true)
      : m_con(con), m_enableUnescapeNewlines(enableUnescapeNewlines)
    {
      m_pipe[0] = pipe[0];
      m_pipe[1] = pipe[1];
    }
    ~LogListener() {}

  public:
    void run();

  private:
    ProcessController* m_con;
    int m_pipe[2];

    bool m_enableUnescapeNewlines;

  };

}

#endif

