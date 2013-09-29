#ifndef _B2DAQ_ProcessListener_hh
#define _B2DAQ_ProcessListener_hh

#include <system/Fork.hh>
#include <system/Mutex.hh>

#include <string>

namespace B2DAQ {

  class RCCallback;

  class ProcessListener {

  public:
    ProcessListener(RCCallback* callback, Fork forkfd,
		    const std::string& process_name) 
      : _callback(callback), _forkfd(forkfd),
	_process_name(process_name) {}
    ~ProcessListener() {}

  public:
    void run();
    void setRunning(bool is_running);

  private:
    RCCallback* _callback;
    Fork _forkfd;
    std::string _process_name;
    Mutex _mutex;
    bool _is_running;

  };

}

#endif

