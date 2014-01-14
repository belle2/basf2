#ifndef GEARBOX_BACKEND_POSTGRES2_H_
#define GEARBOX_BACKEND_POSTGRES2_H_

#include "framework/gearbox/Backend.h"
#include "framework/gearbox/types.h"

#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <pqxx/pqxx>


namespace Belle2 {
  namespace gearbox {

    /**
     * High-Speed backend for relational data using PostgreSQL
     */
    class Backend_Postgres2 : public Backend {

    public:

      /**
       * Constructor
       * @param params Configuration Parameters for the specific Backend
       */
      Backend_Postgres2(const BackendConfigParamSet& params);

      /**
       * Default Destructor
       */
      virtual ~Backend_Postgres2();

      /**
       * Query Backend for Data
       * @param xPath The XPath query concatenated with query parameters
       * @param connectNode the result Node to which the resulting nodes are to be connected
       * @param internalUpToPath the path within the backend that should be associated with the connectNode
       * @param theGBResult the query result set to add the newly fetched nodes to
       */
      void query(const std::string& xPath, QryResultSet& connectNode, std::string internalUpToPath, GBResult& theGBResult);

      /**
       * Checks, whether a path exists
       * @param path the path within (relative) to the Backend, that is to be checked
       * @return the result of the check
       */
      bool checkPathExists(std::string path);

      /**
       * Commit a pending write operation
       * @param Comment describing the write operation
       */
      void writeCommit(std::string comment);


    private:

      /**
       * The Database connection handler
       */
      pqxx::connection* DBConnection;

      /**
       * Aliases, which map nodenames to specific tablenames
       */
      std::map<std::string, std::string> aliasTablename;

      /**
       * Aliases for the value-Column of a specific table
       * !! Key is the nodeName, not the tableName !!
       */
      std::map<std::string, std::string> aliasValuecolumn;

      /**
       * The uniqueIdent string group-part for nodes coming from a specific table
       * !! Key is the nodeName, not the tableName !!
       */
      std::map<std::string, std::string> uniqueIdentGroup;

      /**
       * The name of the column to extract the last (individual) part of the uniqueIdent string from
       * !! Key is the nodeName, not the tableName !!
       */
      std::map<std::string, std::string> uniqueIdentColumn;


      void buildAndAddNode1(pqxx::tuple row, GBResult& theGBResult);
      std::string generateQueryRead1(std::string nodename, std::string conditions, queryParams_t queryParameters);


    }; // class Backend_Postgres2

  } // namespace gearbox
} // namespace Belle2

#endif // GEARBOX_BACKEND_POSTGRES2_H_

