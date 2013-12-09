#ifndef _Belle2_State_hh
#define _Belle2_State_hh

#include "daq/slc/base/Enum.h"

namespace Belle2 {

  class State : public Enum {

  public:
    static const State INITIAL_S;
    static const State CONFIGURED_S;
    static const State READY_S;
    static const State RUNNING_S;
    static const State PAUSED_S;
    static const State BOOTING_TS;
    static const State LOADING_TS;
    static const State STARTING_TS;
    static const State STOPPING_TS;
    static const State ERROR_ES;
    static const State FATAL_ES;
    static const State ABORTING_RS;

  public:
    State() throw() : Enum(Enum::UNKNOWN) {}
    State(const Enum& num) throw() : Enum(num) {}
    State(const State& st) throw() : Enum(st) {}
    State(const char* st) throw() { *this = st; }
    State(const std::string& st) throw() { *this = st; }
    State(int id) throw() { *this = id; }
    ~State()  throw() {}

  protected:
    State(int id, const char* label, const char* alias)
    throw() : Enum(id, label, alias) {}

  public:
    bool isStable() const throw() { return _id > 100 && _id < 200; }
    bool isTransaction() const throw() { return _id > 200 && _id < 300; }
    bool isError() const throw() { return _id > 300 && _id < 400; }
    bool isRecovering() const throw() { return _id > 400 && _id < 500; }
    State next() const throw();

  public:
    const State& operator=(const std::string& msg) throw();
    const State& operator=(const char* msg) throw();
    const State& operator=(int id) throw();

  };

}

#endif
