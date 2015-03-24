#ifndef _Belle2_Stream0Controller_h
#define _Belle2_Stream0Controller_h

#include "daq/slc/apps/rocontrold/ROController.h"

namespace Belle2 {

  class Stream0Controller : public ROController {

  public:
    virtual ~Stream0Controller() throw();

  public:
    void check() throw();

  protected:
    virtual void initArguments(const DBObject& obj);
    virtual bool loadArguments(const DBObject& obj);

  private:
    void readDB(const DBObject& obj, int& port,
                std::string& host, std::string& script);

  };

}

#endif
