#ifndef _Belle2_RunStatus_hh
#define _Belle2_RunStatus_hh

#include "base/DataObject.h"

namespace Belle2 {

  class RunStatus : public DataObject {

  public:
    RunStatus(const std::string& data_name, int revision) throw();
    virtual ~RunStatus() throw() {}

  public:
    int getRunNumber() const throw() { return getInt("run_number"); }
    int getExpNumber() const throw() { return getInt("exp_number"); }
    int getStartTime() const throw() { return getInt("start_time"); }
    int getEndTime() const throw() { return getInt("end_time"); }
    void setRunNumber(int run_number) throw() { setInt("run_number", run_number); }
    void setExpNumber(int exp_number) throw() { setInt("exp_number", exp_number); }
    void setStartTime(int start_time) throw() { setInt("start_time", start_time); }
    void setEndTime(int end_time) throw() { setInt("end_time", end_time); }
    int incrementExpNumber() throw() {
      int exp_number = getExpNumber();
      exp_number++;
      setExpNumber(exp_number);
      return exp_number;
    }
    int incrementRunNumber() throw() {
      int run_number = getRunNumber();
      run_number++;
      setRunNumber(run_number);
      return run_number;
    }
  };

}

#endif
