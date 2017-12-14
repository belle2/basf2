#ifndef _Belle2_Stream0Controller_h
#define _Belle2_Stream0Controller_h

#include "daq/slc/apps/rocontrold/ROController.h"

namespace Belle2 {

  class Stream0Controller : public ROController {

  public:
    virtual ~Stream0Controller() throw();

  public:
    virtual void check();

  protected:
    virtual void initArguments(const DBObject& obj);
    virtual bool loadArguments(const DBObject& obj);

  public:
    const std::string& getHost() const throw() { return m_host; }


  private:
    std::string m_host;

  };

}

#endif
