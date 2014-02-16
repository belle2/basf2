#ifndef GEARBOX_BACKEND_H_
#define GEARBOX_BACKEND_H_


#include "framework/gearbox/types.h"
#include "framework/gearbox/GBResult.h"

#include <framework/logging/Logger.h>

#include <boost/spirit/include/qi.hpp>
#include <string>
#include <map>
#include <list>

#define GEARBOX_BACKEND_UNIQUEHASH_AUTOLENGTH 8

namespace Belle2 {
  namespace gearbox {

    /**
     * key->value map for Backend Query Parameters
     */
    typedef std::map<std::string, std::string> queryParams_t;

    /**
     * Parser for boost spirit in order to decode Backend parameters
     * !! still TODO !!
     */
    namespace qi = boost::spirit::qi;
    template <typename Iterator>
    struct QueryParamsParser : qi::grammar<Iterator, queryParams_t()> {

      QueryParamsParser() : QueryParamsParser::base_type(query) {
      query =  pair >> *(qi::lit(';') >> pair);
      pair  =  key >> '=' >> value;
      key   =  qi::char_("a-zA-Z_") >> *qi::char_("a-zA-Z_0-9");
      value = +qi::char_("a-zA-Z_0-9");
    }

    qi::rule<Iterator, queryParams_t()> query;
    qi::rule<Iterator, std::pair<std::string, std::string>()> pair;
    qi::rule<Iterator, std::string()> key, value;

    };

    /**
     * Abstract Definition of a Gearbox Backend
     */
    class Backend {

    public:

      /**
       * Constructor
       * @param params Configuration Parameters for the specific Backend
       */
      Backend(const BackendConfigParamSet& params);

      /**
       * Default Destructor
       */
      virtual ~Backend();

      /**
       * Query Backend for Data
       * @param xPath The XPath query concatenated with query parameters
       * @param connectNode the result Node to which the resulting nodes are to be connected
       * @param internalUpToPath the path within the backend that should be associated with the connectNode
       * @param theGBResult the query result set to add the newly fetched nodes to
       */
      virtual void query(const std::string& xPath, QryResultSet& connectNode, std::string internalUpToPath, GBResult& theGBResult) = 0;

    protected:

      /**
       * The decoded backend configuration parameters
       */
      BackendConfigParamSet params;

      /**
       * the individual backend's unique Hash, that will be part of the unique node IDs of this backend's nodes
       */
      std::string uniqueHash;

      /**
       * Parses QueryParameters, that have been extracted from a query
       * @param paramString The string containing the parameters
       * @return the decoded parameters
       */
      queryParams_t parseQueryParams(std::string paramString);

      /**
       * The Types of MultiOPs which exist
       */
      enum class multiOp_opType_t : std::uint8_t { ADD_NODES, LINK_NODES };

      /**
       * This class represents a MultiOP, which is something like a data collector for write operations
       * in order to separate write method calls and actual write queries
       */
      class MultiOp {

        // DEPENDS ON WRITE BRANCH
        // TODO

      public:

        typedef std::map<std::string, std::string> IData_t;
        typedef std::list<std::string> Data_t;

        MultiOp(Backend* owner, multiOp_opType_t type, IData_t idata, int autoCommit = 0);
        virtual ~MultiOp();

        void addData(Data_t& data);
        bool commit();
        bool hasData();
        multiOp_opType_t getType();
        void flushData();

      private:

        //Backend* owner;
        multiOp_opType_t opType;
        IData_t idata;
        unsigned int autoCommit;

        std::list<Data_t> data;

      };


      /**
       * The list of the currently existing multiOPs for a Backend
       */
      std::list<MultiOp> multiOpInfos;

      /**
       * The type used to reference to the multiOPs.
       * We can use iterators here, because for std::list, they stay intact for
       * write operations to the list (except of the deletion of the element of course)
       */
      typedef std::list<MultiOp>::iterator MultiOpRef;

      /**
       * Create a new multiOP (factory)
       * @param opType the type of the multiOP
       * @param idata initial general data for this new multiOP
       * @param autocommit The number
       * @return a list to the newly created MultiOP
       */
      MultiOpRef multiOp_begin(multiOp_opType_t opType, MultiOp::IData_t idata, int autocommit = 50);

      /**
       * remove a MultiOP
       * @param the MultiOP, that is to be removed
       */
      void multiOp_remove(MultiOpRef it);

      /**
       * Perform a manual commit of the given MultiOP
       * @param it the MultiOP, that is to be commited
       * @return whether the commit operation has been successful or not
       */
      bool multiOp_commit(MultiOpRef it);

      /**
       * Checks, whether a path exists
       * @param path the path within (relative) to the Backend, that is to be checked
       * @return the result of the check
       */
      virtual bool checkPathExists(std::string path) = 0;

    }; // class Backend

  } // namespace gearbox
} // namespace Belle2

#endif // GEARBOX_BACKEND_H_
