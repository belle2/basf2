#ifndef _Belle2_DBObjectLoader_h
#define _Belle2_DBObjectLoader_h

#include <daq/slc/database/DBObject.h>
#include <daq/slc/database/DBInterface.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

namespace Belle2 {

  class DBObjectLoader {

  public:
    static DBObject load(const std::string& filename);
    static DBObject load(ConfigFile& config);
    static DBObject load(DBInterface& db,
                         const std::string& tablename,
                         const std::string& configname,
                         bool isfull = true);
    static bool createDB(DBInterface& db,
                         const std::string& tablename,
                         const DBObject& obj);
    static StringList getDBlist(DBInterface& db,
                                const std::string& tablename,
                                const std::string& prefix,
                                int max = -1);

  private:
    static bool setObject(DBObject& obj, StringList& str,
                          DBField::Type type, const std::string& value,
                          const std::string& table_in = "",
                          const std::string& config_in = "",
                          DBInterface* db = NULL);
    static bool add(DBObject& obj, StringList& str,
                    const std::string& name_in, const DBObject& cobj);

  };

}

#endif
