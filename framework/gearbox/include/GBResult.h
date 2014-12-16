#ifndef GBRESULT_H_
#define GBRESULT_H_

#include <framework/gearbox/types.h>

#include <boost/variant/get.hpp>
#include <boost/shared_ptr.hpp>

#include <map>


namespace boost {
  namespace property_tree {

    /**
     * This specializes the translators in boost::property_tree for translation between a QryResultSet-Node and its (still templated)
     * value interfacing using std::pair and boost::variant
     */
    template <typename variantType>
    struct VariantTranslator  {

      // necessary for boost to recognize this struct as a valid translator (see boost MPL)
      typedef Belle2::gearbox::QryResultNode internal_type;
      typedef variantType external_type;

      /**
       * Converts a QryResultNode to its value
       * @param value The QryResultNode, whose value is to be fetched
       * @return the value of the QryResultNode
       */
      boost::optional<external_type> get_value(const internal_type& value) {

        boost::optional<external_type> res;

        try {
          res = boost::get<external_type>(value.second);
        } catch (bad_get& e) {
          //TODO: handle boost::variant exceptions properly
        }

        // boost::optional is returned, so it is OK to return empty (e.g. in case of boost::variant exception)
        return res;
      }

      /**
       * Converts a value to a valid QryResultNode
       * @param value the value that is to be converted
       * @return the QryResultNode, that has been produced using that value
       */
      internal_type put_value(const external_type& value) {
        return internal_type(Belle2::gearbox::nodeId_t(Belle2::gearbox::genRandomString(16)), Belle2::gearbox::nodeValue_t(value));
      }

    };

    /*
     * unfortunately, partial specialization does not seem to work here, possibly because the original boost
     * header is using the same type in both template arguments
     */

    /**
     * Translator specialization for QryResultSet <> int
     */
    template<>
    struct translator_between<Belle2::gearbox::QryResultNode, int> {
      typedef VariantTranslator<int> type;
    };

    /**
     * Translator specialization for QryResultSet <> double
     */
    template<>
    struct translator_between<Belle2::gearbox::QryResultNode, double> {
      typedef VariantTranslator<double> type;
    };

    /**
     * Translator specialization for QryResultSet <> std:string
     */
    template<typename Ch, typename Traits, typename Alloc>
    struct translator_between<Belle2::gearbox::QryResultNode, std::basic_string< Ch, Traits, Alloc >> {
      typedef VariantTranslator<std::string> type;
    };


  } // namespace property_tree
} // namespace boost

namespace Belle2 {
  namespace gearbox {

    /**
     * This translator is used by the boost::property_tree to extract the unique node ID from a node
     */
    struct UniqueIdTranslator {

    public:

      /**
       * Extracts the unique node ID from a node
       * @param The node, whose unique ID should be extracted
       * @return the unique node ID
       */
      boost::optional<std::string> get_value(const QryResultNode& value) {
        return boost::optional<std::string>(value.first);
      }

    };


    /**
     * Get the unique node ID of a node
     * @param the node, whose unique node ID is to be fetched
     * @return the unique node ID of the submitted node
     */
    std::string getUniqueNodeId(QryResultSet& res);

    /**
     * Print a result set using B2INFO in a human readable form
     * @param res the result set (its top node), that is to be printed
     * @param prefix a prefix, that is to be added to the path
     * @param header whether a header should be printed before the top-level nodes
     */
    void printResultSet(QryResultSet& res, const std::string& prefix = "", bool header = true);

    /**
     * This class wraps a query result set and provides several methods to work on it
     */
    class GBResult {

    private:

      /**
       * a reference to the root node of this result set
       */
      boost::shared_ptr<QryResultSet> resultSet;

      /**
       * a map (using a red-black-tree) for finding node references by their node IDs
       */
      std::map<nodeId_t, QryResultSet*> nodeIdMap;

      /**
       * Submit a node to this result set using its value as boost::variant type
       * @param parent the parent node, of which the submitted node is a child
       * @param existState the existenceState of the submitted node (e.g. add or modify)
       * @param label the name of the node and as such the last part of its path
       * @param value if present, the value of the node wrapped in boost::variant
       * @param uniqueId the unique ID of the node, which is automatically generated if not present
       * @param refNodeId for node existStates, that are related to another node (e.g. modify), the unique ID of it
       * @param isAttrib whether the submitted not is an attribute or not
       */
      void submitNodeVariant(QryResultSet& parent, nodeExistenceState existState, const std::string label, const nodeValue_t value, const nodeId_t uniqueId, const nodeId_t refNodeId, const bool isAttrib);


      /**
       * Create a new node using its value as boost::variant type
       * @param parent the parent node, to which the new node should be added as child
       * @param label the name of the new node, that is to be created
       * @param value if present, the value of the new node wrapped in boost::variant
       * @param uniqueId the unique ID of the new node, which is automatically generated if not present
       * @param isAttrib whether the new node should be marked as an attribute or not
       * @return the QryResultObject, that references to the newly created node
       */
      QryResultSet& createNodeVariant(QryResultSet& parent, std::string label, nodeValue_t value, nodeId_t uniqueId, bool isAttrib);

      /**
       * converts a path into a valid "node path"
       * @param path the path, that is to be converted
       */
      void makeNodePath(std::string& path);

    public:

      /**
       * Default constructor
       */
      GBResult();

      /**
       * Default destructor
       */
      virtual ~GBResult();

      /**
       * Get the root node of this query result
       * @return the root node of the result set
       */
      boost::shared_ptr<QryResultSet> getResultSetPtr();
      QryResultSet::size_type erasePath(std::string path);

      /**
       * Submit a node to this result set
       * @param parent the parent node, of which the submitted node is a child
       * @param existState the existenceState of the submitted node (e.g. add or modify)
       * @param label the name of the node and as such the last part of its path
       * @param value if present, the value of the node using its native type
       * @param uniqueId the unique ID of the node, which is automatically generated if not present
       * @param refNodeId for node existStates, that are related to another node (e.g. modify), the unique ID of it
       * @param isAttrib whether the submitted not is an attribute or not
       */
      template <typename valueType>
      void submitNode(QryResultSet& parent, nodeExistenceState existState, const std::string label, const valueType value, const nodeId_t uniqueId = genRandomString(16), const nodeId_t refNodeId = "", const bool isAttrib = false) {
        this->submitNodeVariant(parent, existState, std::move(label), nodeValue_t(value), uniqueId, refNodeId, isAttrib);
      }

      /**
       * Create a new node
       * @param parent the parent node, to which the new node should be added as child
       * @param label the name of the new node, that is to be created
       * @param value if present, the value of the new node
       * @param uniqueId the unique ID of the new node, which is automatically generated if not present
       * @param isAttrib whether the new node should be marked as an attribute or not
       * @return the QryResultObject, that references to the newly created node
       */
      template <typename valueType>
      QryResultSet& createNode(QryResultSet& parent, std::string label, valueType value, nodeId_t uniqueId = genRandomString(16), bool isAttrib = false) {
        return this->createNodeVariant(parent, std::move(label), nodeValue_t(value), uniqueId, isAttrib);
      }


      /**
       * Create a non-attribute node without a value
       * @param parent the parent node, to which the new node should be added as child
       * @param label label the name of the new node, that is to be created
       * @param uniqueId uniqueId the unique ID of the new node, which is automatically generated if not present
       * @return the QryResultObject, that references to the newly created node
       */
      QryResultSet& createEmptyNode(QryResultSet& parent, std::string label, std::string uniqueId = genRandomString(16));

      /**
       * Get reference to a node by providing its path (non-deterministic, because paths are not unique)
       * @param path the path, which is to be used to seek for the node
       * @return the reference to the requested node
       */
      QryResultSet& getNodeByPath(std::string path);

      /**
       * Get reference to a node by its unique ID
       * @param uniqueId the unique ID, which is to be used to seek for the node
       * @return the reference to the requested node
       */
      QryResultSet& getNodeByUniqueId(std::string uniqueId);

      /**
       * Ensure, that a path exists and get the reference to a corresponding node (non-deteministic)
       * @param path the path, whose existence is to be ensured
       * @return a reference to the last node of the submitted path
       */
      QryResultSet& ensurePath(std::string path);

      /**
       * The exception returned, if a node has not been found
       */
      struct NodeNotFoundException {

      };

    };

  } /* namespace gearbox */
} /* namespace Belle2 */

#endif /* GBRESULT_H_ */
