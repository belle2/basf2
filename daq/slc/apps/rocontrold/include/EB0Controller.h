#ifndef _Belle2_EB0Controller_h
#define _Belle2_EB0Controller_h

#include "daq/slc/apps/rocontrold/ROController.h"

namespace Belle2 {

  class EB0Controller : public ROController {

  public:
    virtual ~EB0Controller() throw();

  protected:
    virtual void initArguments(const DBObject& obj);
    virtual bool loadArguments(const DBObject& obj);

  private:
    void readDB(const DBObject& obj, int& port,
                std::string& executable, bool& used, int& nsenders);

  };

}

#endif
