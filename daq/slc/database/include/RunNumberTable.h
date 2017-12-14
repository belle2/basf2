#ifndef _Belle2_RunNumberTable_h
#define _Belle2_RunNumberTable_h

#include "daq/slc/database/RunNumber.h"

namespace Belle2 {

  class DBInterface;

  class RunNumberTable {

  public:
    RunNumberTable(DBInterface& db) : m_db(db) {}
    ~RunNumberTable() throw() {}

  public:
    RunNumber add(const std::string& config, int expno,
                  int runno, int subno, bool istart = true);
    RunNumber add(const RunNumber& info);
    int getRunNumber(int expno);
    int getExpNumber(const std::string& config = "");
    void create();

  public:
    RunNumberList get(int expno, bool started, int runno_min = 0, int runno_max = -1);
    RunNumberList get(int expno, int runno_min = 0, int runno_max = -1);
    RunNumberList get(const std::string& type, int expno, int runno_min = 0, int runno_max = -1);

  private:
    DBInterface& m_db;

  };

}

#endif
