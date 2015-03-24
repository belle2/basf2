#ifndef _Belle2_Stream1Controller_h
#define _Belle2_Stream1Controller_h

#include "daq/slc/apps/rocontrold/Stream0Controller.h"

namespace Belle2 {

  class Stream1Controller : public Stream0Controller {

  public:
    virtual ~Stream1Controller() throw() {}

  protected:
    virtual void initArguments(const DBObject& obj);
    virtual bool loadArguments(const DBObject& obj);

  private:
    void readDB(const DBObject& obj, int& port,
                std::string& script);

  };

}

#endif
