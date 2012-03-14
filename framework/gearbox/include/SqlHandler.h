/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2012  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Rok Pestotnik                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEARSQLHANDLER_H
#define GEARSQLHANDLER_H

#include <framework/gearbox/InputHandler.h>
#include <boost/iostreams/filtering_stream.hpp>


class TSQLStatement;
class TSQLServer;
class TUrl;


namespace Belle2 {
  namespace gearbox {
    /** InputContext implementation for sql database backend */
    class SqlContext: public InputContext {
    public:
      /**
       * Create a new context from an (already executed) mysql statement
       * Statement is owned by SqlHandler and will only be used*/
      SqlContext(TSQLStatement* statement);
      /** Reset the statement to be ready for execution again */
      virtual ~SqlContext();
      /** Read a block of data */
      virtual int readXmlData(char* buffer, int buffsize) {
        B2DEBUG(100, "readXmlData:  buffsize=" <<  buffsize);
        m_stream.read(buffer, buffsize);
        B2DEBUG(100, "readXmlData: Bytes read from m_stream " << m_stream.gcount());
        return m_stream.gcount();
      }

    protected:
      /** stream to provide easy access to the column pointer offered by Sql */
      boost::iostreams::filtering_istream m_stream;
    };

    /**
     * InputHandler using Sql as backend.
     */
    class SqlHandler: public InputHandler {
    public:
      /**
       * Create a new SqlHandler opening a given database file
       * @param uri name of the database file
       */
      SqlHandler(const std::string& uri);
      /** Close database connection */
      ~SqlHandler();
      /**
       * Return InputContext corresponding to a requested XML resource
       * @return SqlContext pointer on success, 0 on failure
       */
      virtual InputContext* open(const std::string& path);

    protected:
      /** string containing the query to be executed */
      std::string GetQuery(TSQLServer*, TUrl*);
      static std::string m_data;
      /** database object representing the TSQLServer database */
      TSQLServer* m_database;
      /** prepared statement object used for querying the database */
      TSQLStatement* m_statement;

    };
  }
}

#endif
