/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_MonitorDB_h
#define _Belle2_MonitorDB_h

#include <daq/slc/base/Date.h>

#include <daq/slc/nsm/NSMVar.h>

#include <daq/slc/database/DBInterface.h>

namespace Belle2 {

  class MonitorDB {

  public:
    static void add(DBInterface& db, const std::string& tablename,
                    const std::string& vname, int val);
    static void add(DBInterface& db, const std::string& tablename,
                    const std::string& vname, float val);
    static void add(DBInterface& db, const std::string& tablename,
                    const std::string& vname, const std::string& val);
    static NSMVarList get(DBInterface& db,
                          const std::string& tablename,
                          const std::string& vname);
    static NSMVarList get(DBInterface& db,
                          const std::string& tablename,
                          const std::string& vname, int max);
    static NSMVarList get(DBInterface& db,
                          const std::string& tablename,
                          const std::string& vname, const Date& start,
                          const Date& end);
    static NSMVarList get(DBInterface& db,
                          const std::string& tablename,
                          const std::string& vname, int max,
                          const Date& start, const Date& end);
  private:
    static NSMVarList readTable(DBInterface& db, const std::string& vname);
    static void createTable(DBInterface& db, const std::string& tablename);

  };

}

#endif
