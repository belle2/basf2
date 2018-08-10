#ifndef _Belle2_DAQLogDB_h
#define _Belle2_DAQLogDB_h

#include <daq/slc/database/DAQLogMessage.h>
#include <daq/slc/database/DBInterface.h>

#include <vector>

namespace Belle2 {

  typedef std::vector<DAQLogMessage> DAQLogMessageList;

  class DAQLogDB {

  public:
    static bool createLog(DBInterface& db,
                          const std::string& tablename,
                          const DAQLogMessage& log);
    static DAQLogMessageList getLogs(DBInterface& db,
                                     const std::string& tablename,
                                     const std::string& nodename = "",
                                     int max = 0);
    static DAQLogMessageList getLogs(DBInterface& db,
                                     const std::string& tablename,
                                     const std::string& nodename,
                                     const std::string& begin_date,
                                     const std::string& end_date,
                                     int max = 0);
  };

}

#endif
