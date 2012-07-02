#ifdef HAS_SQLITE

#include <sqlite3.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <framework/gearbox/SQLiteHandler.h>
#include <framework/gearbox/Gearbox.h>

#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>

using namespace std;
namespace io = boost::iostreams;

namespace Belle2 {
  namespace gearbox {

    const string SQLiteHandler::m_query = "SELECT data,compressed FROM names "
                                          "LEFT JOIN meta USING (nameID) LEFT JOIN data USING (dataID) "
                                          "WHERE name=:name AND firstExp<=:exp AND (lastExp<0 OR lastExp>=:exp) "
                                          "AND firstRUN<=:run AND (lastRun<0 OR lastRun>=:run) ORDER BY meta.rowid ASC";

    SQLiteContext::SQLiteContext(sqlite3_stmt* statement)
    {
      m_statement = statement;
      const size_t size = sqlite3_column_bytes(m_statement, 0);
      const char* data = (const char*) sqlite3_column_text(m_statement, 0);
      const int compressed = sqlite3_column_int(m_statement, 1);
      if (compressed) m_stream.push(io::gzip_decompressor());
      m_stream.push(io::array_source(data, size));
    }

    SQLiteContext::~SQLiteContext()
    {
      sqlite3_reset(m_statement);
    }

    SQLiteHandler::SQLiteHandler(const string& uri): InputHandler(uri), m_database(0), m_statement(0)
    {
      B2DEBUG(100, "Opening SQLite database file " << uri);
      int resultCode = sqlite3_open_v2(uri.c_str(), &m_database, SQLITE_OPEN_READONLY, 0);
      if (resultCode) {
        B2ERROR("Cannot open SQLite database: " << sqlite3_errmsg(m_database));
        sqlite3_close(m_database);
        m_database = 0;
      } else {
        resultCode = sqlite3_prepare_v2(m_database, m_query.c_str(), m_query.size(), &m_statement, 0);
        if (resultCode) B2ERROR("Cannot prepare statement: " << sqlite3_errmsg(m_database));
      }
    }

    InputContext* SQLiteHandler::open(const string& path)
    {
      if (!m_statement) return 0;
      StoreObjPtr<EventMetaData> eventMetaDataPtr;

      int exp = eventMetaDataPtr->getExperiment();
      int run = eventMetaDataPtr->getRun();
      int indexName = sqlite3_bind_parameter_index(m_statement, ":name");
      int indexExp = sqlite3_bind_parameter_index(m_statement, ":exp");
      int indexRun = sqlite3_bind_parameter_index(m_statement, ":run");
      sqlite3_bind_int(m_statement, indexExp, exp);
      sqlite3_bind_int(m_statement, indexRun, run);
      sqlite3_bind_text(m_statement, indexName, path.c_str(), path.size(), SQLITE_TRANSIENT);
      int resultCode = sqlite3_step(m_statement);
      if (resultCode == SQLITE_ROW) {
        return new SQLiteContext(m_statement);
      }
      if (resultCode == SQLITE_DONE) {
        B2WARNING(m_uri << ": Could not find entry corresponding to " << path);
      } else {
        B2ERROR(m_uri << ":Problem executing statement: " << sqlite3_errmsg(m_database));
      }
      return 0;
    }

    SQLiteHandler::~SQLiteHandler()
    {
      if (m_statement) sqlite3_finalize(m_statement);
      if (m_database) sqlite3_close(m_database);
    }

    B2_GEARBOX_REGISTER_INPUTHANDLER(SQLiteHandler, "sqlite");
  }
}

#endif
