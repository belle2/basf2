#ifndef _Belle2_DB2NSM_h
#define _Belle2_DB2NSM_h

#include <daq/slc/database/DBInterface.h>

#include <daq/slc/nsm/NSMMessage.h>

namespace Belle2 {

  class DB2NSM {

  public:
    DB2NSM(DBInterface* db = NULL) : m_db(db) {}
    ~DB2NSM() throw() {}

  public:
    bool set(const NSMMessage& msg_in, NSMMessage& msg_out);
    bool get(const NSMMessage& msg_in);

  private:
    DBInterface* m_db;

  };

};

#endif
