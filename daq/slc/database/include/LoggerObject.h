#ifndef _Belle2_LoggerObject_hh
#define _Belle2_LoggerObject_hh

#include <daq/slc/database/ConfigObject.h>

namespace Belle2 {

  class LoggerObject : public ConfigObject {

  public:
    LoggerObject();
    LoggerObject(const LoggerObject& obj);
    virtual ~LoggerObject() throw();

  public:

  };

}

#endif
