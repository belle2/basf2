#ifndef GEARBOX_BACKEND_POSTGRES1_H_
#define GEARBOX_BACKEND_POSTGRES1_H_

#include "framework/gearbox/Backend.h"
#include "framework/gearbox/types.h"

#include <boost/property_tree/xml_parser.hpp>
#include <cstdint>
#include <vector>
#include <string>
#include <pqxx/pqxx>

namespace Belle2 {
  namespace gearbox {

    /**
     * Backend class, that uses PostgreSQL and its ltree plugin to store data
     */
    class Backend_Postgres1 : public Backend {

    public:

      /**
       * Constructor
       * @param params Configuration Parameters for the specific Backend
       */
      Backend_Postgres1(const BackendConfigParamSet& params);

      /**
       * Default Destructor
       */
      virtual ~Backend_Postgres1();

      /**
       * Query Backend for Data
       * @param xPath The XPath query concatenated with query parameters
       * @param connectNode the result Node to which the resulting nodes are to be connected
       * @param internalUpToPath the path within the backend that should be associated with the connectNode
       * @param theGBResult the query result set to add the newly fetched nodes to
       */
      void query(const std::string& xPath, QryResultSet& connectNode, std::string internalUpToPath, GBResult& theGBResult);

      /**
       * Commit a pending write operation
       * @param Comment describing the write operation
       */
      void writeCommit(std::string comment);

      /**
       * Checks, whether a path exists
       * @param path the path within (relative) to the Backend, that is to be checked
       * @return the result of the check
       */
      bool checkPathExists(std::string path);

    private:

      /**
       * The Database connection handler
       */
      pqxx::connection* DBConnection;

      /**
       * Converts a XPath string to a lquery for ltree
       * @param xPath the XPath, that is to be converted
       * @return the resulting lquery
       */
      std::string convertXPath2LQuery(std::string xPath);

      /**
       * Ensures, that a specific path exists
       * @param nodePathVec path labels of the path that is to be ensured
       * @param nodeIDPathVec node IDs the nodes, that correspond to the labels of the previous argument
       * @param connectPathVec labels of the path, that is parent of the ensurePath and to which the ensured path should be connected
       * @param upperNode the QryResult node-object of the path described in the previous argument
       * @param theGBResult the GBResult-Object, which holds the nodes
       * @return the node, which corresponds to the ensured path
       */
      QryResultSet& ensurePath(std::vector<std::string>& nodePathVec, std::vector<std::string>& nodeIDPathVec, const std::vector<std::string>& connectPathVec, QryResultSet& upperNode, GBResult& theGBResult);

    }; // class Backend_Postgres1

  } // namespace gearbox
} // namespace Belle2

#endif // GEARBOX_BACKEND_POSTGRES1_H_

