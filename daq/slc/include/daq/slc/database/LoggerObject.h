#ifndef _Belle2_LoggerObject_hh
#define _Belle2_LoggerObject_hh

#include <daq/slc/database/DBObject.h>

namespace Belle2 {

  class LoggerObject : public DBObject {

  public:
    LoggerObject();
    LoggerObject(const LoggerObject& obj);
    virtual ~LoggerObject() throw();

  public:

  };

}

#endif
