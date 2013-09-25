#ifndef _B2DAQ_RunStatus_hh
#define _B2DAQ_RunStatus_hh

#include "NSMData.hh"

#include <string>

namespace B2DAQ {

  class NSMNode;

  typedef unsigned long long ExpNumber_t;
  typedef unsigned long long RunNumber_t;
  typedef unsigned long long RunTime_t;

  class RunStatus : public NSMData {

  public:
    RunStatus(const std::string& data_name) throw();
    virtual ~RunStatus() throw() {}

  public:
    virtual void read(NSMNode*) throw(NSMHandlerException);
    virtual void write(NSMNode*) throw(NSMHandlerException);

  public:
    unsigned long long getSerial() const throw() { return _serial; }
    ExpNumber_t getExpNumber() const throw() { return _exp_no; }
    RunNumber_t getRunNumber() const throw() { return _run_no; }
    RunTime_t getStartTime() const throw() { return _start_time; }
    RunTime_t getEndTime() const throw() { return _end_time; }
    unsigned long long getTotalTriggers() const throw() {
      return _total_triggers;
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
    void setTotalTriggers(unsigned long long total_triggers) throw() {
      _total_triggers = total_triggers;
    }

  private:
    unsigned long long _serial;
    ExpNumber_t _exp_no;
    RunNumber_t _run_no;
    RunTime_t _start_time;
    RunTime_t _end_time;
    unsigned long long _total_triggers;
    
  };

}

#endif
