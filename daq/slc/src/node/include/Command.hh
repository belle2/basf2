#ifndef _B2DAQ_Command_hh
#define _B2DAQ_Command_hh

#include "Enum.hh"

namespace B2DAQ {

  class State;

  class Command : public Enum {

  public:
    static const int SUGGESTED = 2;
    static const int ENABLED = 1;
    static const int DISABLED = 0;

  public:
    static const Command OK;
    static const Command ERROR;

  public:
    Command() throw();
    Command(const Command& cmd) throw();
    Command(const Enum& e) throw();
    ~Command() throw();

  protected:
    Command(int id, const char* label, const char* alias) throw();
 
  };

}

#endif
