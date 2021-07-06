/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_RunNumberTable_h
#define _Belle2_RunNumberTable_h

#include "daq/slc/database/RunNumber.h"

namespace Belle2 {

  class DBInterface;

  class RunNumberTable {

  public:
    RunNumberTable(DBInterface& db) : m_db(db) {}
    ~RunNumberTable() {}

  public:
    RunNumber add(const std::string& node,
                  const std::string& runtype,
                  int expno, int runno);
    RunNumber add(const RunNumber& info);
    int getRunNumber(const std::string& node, int expno);
    int getExpNumber(const std::string& node);
    void create();

  public:
    RunNumberList get(const std::string& node, int expno, int runno_min = 0, int runno_max = -1);
    RunNumberList get(int expno, int runno_min = 0, int runno_max = -1);

  private:
    DBInterface& m_db;

  };

}

#endif
