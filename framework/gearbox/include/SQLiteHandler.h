/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEARFILEHANDLER_H
#define GEARFILEHANDLER_H

#include <framework/gearbox/InputHandler.h>
#include <boost/iostreams/filtering_stream.hpp>

struct sqlite3;
struct sqlite3_stmt;

namespace Belle2 {
  namespace gearbox {
    /** InputContext implementation for sqlite3 database backend */
    class SQLiteContext: public InputContext {
    public:
      /**
       * Create a new context from an (already executed) sqlite3 statement
       * Statement is owned by SQLiteHandler and will only be used*/
      SQLiteContext(sqlite3_stmt* m_statement);
      /** Reset the statement to be ready for execution again */
      virtual ~SQLiteContext();
      /** Read a block of data */
      virtual int readXmlData(char *buffer, int buffsize) {
        m_stream.read(buffer, buffsize);
        return m_stream.gcount();
      }
    protected:
      /** pointer to the executed sqlite3 statement */
      sqlite3_stmt* m_statement;
      /** stream to provide easy access to the column pointer offered by SQLite */
      boost::iostreams::filtering_istream m_stream;
    };

    /**
     * InputHandler using SQLite3 as backend.
     * This handler is mainly meant as an example to show how relational databases could be interfaced
     */
    class SQLiteHandler: public InputHandler {
    public:
      /**
       * Create a new SQLiteHandler opening a given database file
       * @param uri name of the database file
       */
      SQLiteHandler(const std::string& uri);
      /** Close database connection */
      ~SQLiteHandler();
      /**
       * Return InputContext corresponding to a requested XML resource
       * @return SQLiteContext pointer on success, 0 on failure
       */
      virtual InputContext* open(const std::string &path);

    protected:
      /** string containing the query to be executed */
      static const std::string m_query;
      /** database object representing the SQLite3 database */
      sqlite3* m_database;
      /** prepared statement object used for querying the database */
      sqlite3_stmt* m_statement;
    };
  }
}

#endif
