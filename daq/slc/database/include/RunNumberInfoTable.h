#ifndef _Belle2_RunNumberTable_h
#define _Belle2_RunNumberTable_h

#include "daq/slc/database/RunNumberInfo.h"


namespace Belle2 {

  class DBInterface;

  class RunNumberInfoTable {

  public:
    RunNumberInfoTable(DBInterface* db) : m_db(db) {}
    ~RunNumberInfoTable() throw() {}

  public:
    void create() throw();
    RunNumberInfo add(int expno, int runno, int subno);
    RunNumberInfo add(const RunNumberInfo& info);
    RunNumberInfoList getList(int expno, int runno, int subno, int max = 0);
    int getRunNumber(int expno);
    int getExpNumber();

  private:
    DBInterface* m_db;

  };

}

#endif
