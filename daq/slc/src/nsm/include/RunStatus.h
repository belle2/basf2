#ifndef _Belle2_RunStatus_hh
#define _Belle2_RunStatus_hh

#include "NSMData.h"

#include <string>

namespace Belle2 {

  class NSMNode;

  typedef unsigned long long ExpNumber_t;
  typedef unsigned long long RunNumber_t;
  typedef unsigned long long RunTime_t;

  class RunStatus : public NSMData {

  public:
    RunStatus(const std::string& data_name) throw();
    virtual ~RunStatus() throw() {}

  public:
    void read() throw(NSMHandlerException);
    void write() throw(NSMHandlerException);

  public:
    unsigned long long getSerial() const throw() { return _serial; }
    ExpNumber_t getExpNumber() const throw() { return _exp_no; }
    RunNumber_t getRunNumber() const throw() { return _run_no; }
    RunTime_t getStartTime() const throw() { return _start_time; }
    RunTime_t getEndTime() const throw() { return _end_time; }
    unsigned long long getEventTotal() const throw() {
      return _event_total;
    }
    unsigned long long getEventNumber() const throw() {
      return _event_number;
    }
    int getState(unsigned int index, int state) throw() {
      return _state_v[index];
    }
    void setExpNumber(ExpNumber_t exp_no) throw() { _exp_no = exp_no; }
    void setRunNumber(RunNumber_t run_no) throw() { _run_no = run_no; }
    ExpNumber_t incrementExpNumber() throw() {
      _exp_no++;
      return _exp_no;
    }
    RunNumber_t incrementRunNumber() throw() {
      _run_no++;
      return _run_no;
    }
    void setStartTime(RunTime_t start_time) throw() { _start_time = start_time; }
    void setEndTime(RunTime_t end_time) throw() { _end_time = end_time; }
    void setEventTotal(unsigned long long event_total) throw() {
      _event_total = event_total;
    }
    void setEventNumber(unsigned long long event_number) throw() {
      _event_number = event_number;
    }
    void setState(unsigned int index, int state) throw() {
      _state_v[index] = state;
    }

  private:
    unsigned long long _serial;
    ExpNumber_t _exp_no;
    RunNumber_t _run_no;
    RunTime_t _start_time;
    RunTime_t _end_time;
    unsigned long long _event_number;
    unsigned long long _event_total;
    int _state_v[1024];

  };

}

#endif
