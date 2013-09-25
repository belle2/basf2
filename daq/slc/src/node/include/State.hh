#ifndef _B2DAQ_State_hh
#define _B2DAQ_State_hh

#include "Enum.hh"

namespace B2DAQ {

  class State : public Enum {

  public:
    State() throw();
    State(const Enum& e) throw();
    State(const State& st) throw();
    ~State()  throw();

  public:
    bool isStable() const throw() { return _id > 100 && _id < 200; }
    bool isTransaction() const throw() { return _id > 200 && _id < 300; }
    bool isError() const throw() { return _id > 300 && _id < 400; }
    bool isRecovering() const throw() { return _id > 400 && _id < 500; }

  protected:
    State(int id, const char* label, const char* alias)  throw();
 
  };

}

#endif
