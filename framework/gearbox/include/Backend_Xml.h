#ifndef GEARBOX_BACKEND_XML_H_
#define GEARBOX_BACKEND_XML_H_

#include "framework/gearbox/Backend.h"

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <vector>

#define IFCTL_NAMESPACE_URI "urn:X-belle2:interface-control"

namespace Belle2 {
  namespace gearbox {

    /**
     * Backend for working with XML files eventually providing a dedicated namespace
     * for interface control functionality
     */
    class Backend_Xml : public Backend {
    public:

      /**
       * Constructor
       * @param params Configuration Parameters for the specific Backend
       */
      Backend_Xml(const BackendConfigParamSet& params);

      /**
       * Default Destructor
       */
      virtual ~Backend_Xml();

      /**
       * Query Backend for Data
       * @param xPath The XPath query concatenated with query parameters
       * @param connectNode the result Node to which the resulting nodes are to be connected
       * @param internalUpToPath the path within the backend that should be associated with the connectNode
       * @param theGBResult the query result set to add the newly fetched nodes to
       */
      void query(const std::string& xPath, QryResultSet& connectNode, const std::string internalUpToPath, GBResult& theGBResult);

      /**
       * Checks, whether a path exists
       * @param path the path within (relative) to the Backend, that is to be checked
       * @return the result of the check
       */
      bool checkPathExists(std::string path);

    private:

      /**
       * the libxml2-representation of the XML document corresponding to this backend
       */
      xmlDocPtr doc;

      /**
       * the libxml2-representation of the database-control namespace
       */
      xmlNsPtr ifctlXmlNS;

      /**
       * ensures that the given node and all of its siblings and children have unique node IDs.
       * @param node the node to start parsing with
       */
      void ensureAllNodesHaveUniqueIds(xmlNodePtr node);

      /**
       * Ensures, that a specific path exists
       * @param actNode xml-Node, whose path is to be ensured
       * @param internalUpToPath labels of the path, that is parent of the ensurePath and to which the ensured path should be connected
       * @param upperNode the QryResult node-object of the path described in the previous argument
       * @param theGBResult the GBResult-Object, which holds the nodes
       * @return the node, which corresponds to the ensured path
       */
      QryResultSet& ensurePath(xmlNodePtr actNode, std::string& internalUpToPath, QryResultSet& mountPathNode, GBResult& theGBResult);

      /**
       * Get a certain property stored in the interface-control namespace
       * @param node the node, whose properties should be taken into account
       * @param valname the name of the property (attribute) in the interface-control-namespace
       * @return the value of the porperty
       */
      std::string getIfCtlProperty(xmlNodePtr node, std::string valname);


      /**
       * Get the unique ID of the given XML node
       * @param node the XML node whose unique ID is to be extracted
       * @return the unique ID
       */
      std::string getUniqueId(xmlNodePtr node);

      /**
       * Get the refnode-property of the given node
       * @param node the node, whose refnode-property is to be returned
       * @return the refnode-property
       */
      std::string getRefNode(xmlNodePtr node);

      /**
       * Get the existence-state of the given node
       * @param node the node, whose existence-state is to be returned
       * @return the existence-state
       */
      nodeExistenceState getExistenceState(xmlNodePtr node);

      /**
       * Build node from XML and add it to result set
       * @param node pointer to the xml node
       * @param parent pointer to the parent node in the result set
       * @param theGBResult the GBResult-Object, which holds the nodes
       */
      void buildAndAddNode(xmlNodePtr node, QryResultSet& parent, GBResult& theGBResult);

      /**
       * Gets the value of a XML node
       * @param node the node, whose value is to be fetched
       * @return the node's value
       */
      std::string getXMLNodeValue(xmlNodePtr node);


    }; // class Backend_XML

  } // namespace gearbox
} // namespace Belle2

#endif // GEARBOX_BACKEND_XML_H_
