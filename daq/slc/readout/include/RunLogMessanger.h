#ifndef _Belle2_RunLogMessanger_h
#define _Belle2_RunLogMessanger_h

#include "daq/slc/readout/SharedMemory.h"
#include "daq/slc/readout/MMutex.h"
#include "daq/slc/readout/MCond.h"

#include <string>

namespace Belle2 {

  class RunLogMessanger {

  public:
    static const int MAX_MESSAGE;

  public:
    struct run_log_message {
      int priority;
      char message[60];
    };

  public:
    static const int DEBUG;
    static const int NOTICE;
    static const int ERROR;
    static const int FATAL;

  public:
    RunLogMessanger() {}
    ~RunLogMessanger() {}

  public:
    size_t size() throw();
    bool open(const std::string& path);
    bool create(const std::string& path);
    void close();
    void unlink(const std::string& path = "");
    std::string recieve(int& priority, int timeout = -1);
    bool send(int priority, const std::string& message);

  private:
    std::string _path;
    SharedMemory _memory;
    MMutex _mutex;
    MCond _cond;
    int* _windex;
    int* _rindex;
    run_log_message* _msg_v;

  };

}

#endif
