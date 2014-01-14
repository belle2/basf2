#include "framework/gearbox/Backend_Xml.h"

#include <boost/regex.hpp>

namespace Belle2 {
  namespace gearbox {

    Backend_Xml::Backend_Xml(const BackendConfigParamSet& params) : Backend(params)
    {

      this->doc = xmlParseFile(boost::get<std::string>(this->params["filename"]).c_str());


      if (this->doc == NULL)
        throw("XML file not found");

      xmlNodePtr rootNode = xmlDocGetRootElement(this->doc);
      if (rootNode == NULL)
        throw("Root node not found - it must exist");

      if (!(this->ifctlXmlNS = xmlSearchNsByHref(this->doc, rootNode, (const xmlChar*) IFCTL_NAMESPACE_URI)))
        this->ifctlXmlNS = xmlNewNs(rootNode, (const xmlChar*) IFCTL_NAMESPACE_URI, (const xmlChar*)"ifctl");

      const char* uniqueHashPropVal = (const char*) xmlGetNsProp(rootNode, (const xmlChar*) "uniqueHash", (const xmlChar*) IFCTL_NAMESPACE_URI);
      if (uniqueHashPropVal != NULL)
        this->uniqueHash = std::string(uniqueHashPropVal);
      else {
        (xmlNewProp(rootNode, (const xmlChar*)"uniqueHash", (const xmlChar*)(this->uniqueHash = genRandomString(GEARBOX_BACKEND_UNIQUEHASH_AUTOLENGTH)).c_str()))->ns = this->ifctlXmlNS;
      }

      if (boost::get<int>(this->params["disableUniqueIdCheck"]) != 1)
        this->ensureAllNodesHaveUniqueIds(rootNode);

    } // Backend_Xml constructor


    void Backend_Xml::ensureAllNodesHaveUniqueIds(xmlNodePtr node)
    {

      while (node != NULL) {

        if (node->type == XML_ELEMENT_NODE) {

          if (xmlGetNsProp(node , (const xmlChar*) "nodeId", (const xmlChar*) IFCTL_NAMESPACE_URI) == NULL)
            (xmlNewProp(node, (const xmlChar*)"nodeId", (const xmlChar*) genRandomString(8).c_str()))->ns = this->ifctlXmlNS;

          if (node->children)
            this->ensureAllNodesHaveUniqueIds(node->children);

        }
        node = node->next;
      }

    }

    Backend_Xml::~Backend_Xml()
    {

      xmlSaveFormatFileEnc(boost::get<std::string>(this->params["filename"]).c_str(), this->doc, "UTF-8", 1);

      xmlFreeDoc(this->doc);

    } // Backend_Xml destructor



    void Backend_Xml::query(const std::string& xPath, QryResultSet& connectNode, std::string internalUpToPath, GBResult& theGBResult)
    {

      B2INFO("Gearbox::Backend_Xml::query: received xPath '" + xPath + "' and connect to '" + getUniqueNodeId(connectNode) + "' at internal path '" + internalUpToPath + "'");

      if (internalUpToPath[internalUpToPath.length() - 1] == '/' && internalUpToPath.length() > 1)
        internalUpToPath.pop_back();

      boost::smatch extraction;
      boost::regex_search(xPath, extraction, boost::regex("^(.*)(\\{.*\\})?$"));

      auto queryParams = this->parseQueryParams(extraction[2]);

      xmlXPathContextPtr xpathCtx(xmlXPathNewContext(this->doc));
      xmlXPathObjectPtr xpathObj(xmlXPathEvalExpression((unsigned char*)extraction[1].str().c_str(), xpathCtx));

      int size(xpathObj->nodesetval ? xpathObj->nodesetval->nodeNr : 0);

      for (int i(0); i < size; ++i) {

        // process element Nodes and attribute nodes only
        if (xpathObj->nodesetval->nodeTab[i]->type == XML_ELEMENT_NODE || xpathObj->nodesetval->nodeTab[i]->type == XML_ATTRIBUTE_NODE) {

          QryResultSet& parent = ensurePath(xpathObj->nodesetval->nodeTab[i]->parent, internalUpToPath, connectNode, theGBResult);

          this->buildAndAddNode(xpathObj->nodesetval->nodeTab[i], parent, theGBResult);

        }

      }

      xmlXPathFreeObject(xpathObj);
      xmlXPathFreeContext(xpathCtx);

    }

    void Backend_Xml::buildAndAddNode(xmlNodePtr node, QryResultSet& parent, GBResult& theGBResult)
    {


      B2DEBUG(4, "Gearbox::Backend_Xml::buildAndAddNode: processing '" + std::string((const char*)node->name) + "' with value '"
              + this->getXMLNodeValue(node) + "' and parent '"
              + getUniqueNodeId(parent) + "'")
      ;

      nodeExistenceState existState(this->getExistenceState(node));

      theGBResult.submitNode(parent,
                             existState,
                             std::string((const char*)node->name),
                             this->getXMLNodeValue(node),
                             this->getUniqueId(node),
                             (existState == nodeExistenceState::modify || existState == nodeExistenceState::remove) ? this->getRefNode(node) : "",
                             node->type == XML_ATTRIBUTE_NODE ? true : false
                            );


    }

    std::string Backend_Xml::getXMLNodeValue(xmlNodePtr node)
    {

      std::string ret;

      if (node->type == XML_ATTRIBUTE_NODE) {

        // ridiculus!! have to go one node up to read attrib value
        ret.assign((const char*) xmlGetProp(node->parent, node->name));


      } else if (node->type == XML_ELEMENT_NODE && xmlChildElementCount(node) == 0) {

        // TODO: Get content of first _text child node_ even of other element node children are present

        ret.assign((const char*) xmlNodeGetContent(node));

      }

      return ret;

    }

    QryResultSet& Backend_Xml::ensurePath(xmlNodePtr actNode, std::string& internalUpToPath, QryResultSet& mountPathNode, GBResult& theGBResult)
    {

      QryResultSet* ret;

      if (std::string((const char*)xmlGetNodePath(actNode)) == internalUpToPath) {
        return mountPathNode;
      }

      std::string actNodeId = this->getUniqueId(actNode);

      try {

        ret = &theGBResult.getNodeByUniqueId(actNodeId);

      } catch (GBResult::NodeNotFoundException&) {

        QryResultSet& parent = this->ensurePath(actNode->parent, internalUpToPath, mountPathNode, theGBResult);

        ret = &theGBResult.createEmptyNode(parent, std::string((const char*)actNode->name), actNodeId);
      }

      return *ret;

    }

    std::string Backend_Xml::getUniqueId(xmlNodePtr node)
    {

      std::string ret;
      std::string append;

      if (!(node->type == XML_ELEMENT_NODE || node->type == XML_ATTRIBUTE_NODE))
        throw(std::string("getUniqueId: Unique nodeID not supported for this type of node"));

      if (node->type == XML_ATTRIBUTE_NODE) {
        append = "_" + std::string((const char*) node->name);
        node = node->parent;
      }

      ret = this->getIfCtlProperty(node, "nodeId");

      return this->uniqueHash + ret + append;

    }

    std::string Backend_Xml::getRefNode(xmlNodePtr node)
    {

      std::string ret;

      if (node->type == XML_ELEMENT_NODE) {
        ret = this->getIfCtlProperty(node, "refnode");
      } else {
        throw (std::string("getRefNode: refNode not supported for this type of node"));
      }

      return ret;

    }


    nodeExistenceState Backend_Xml::getExistenceState(xmlNodePtr node)
    {

      nodeExistenceState ret;

      if (node->type == XML_ATTRIBUTE_NODE) {
        ret = nodeExistenceState::add;
      } else if (node->type == XML_ELEMENT_NODE) {


        std::string exStr = this->getIfCtlProperty(node, "existstate");

        if (exStr == "add" || exStr == "") ret = nodeExistenceState::add;
        else if (exStr == "modify") ret = nodeExistenceState::modify;
        else if (exStr == "remove") ret = nodeExistenceState::remove;
        else if (exStr == "remove_all") ret = nodeExistenceState::remove_all;
        else throw (std::string("getExistenceState: unknown existence State '" + exStr + "'"));

      } else {
        throw (std::string("getExistenceState: existenceState not supported for this type of node"));
      }

      return ret;

    }

    std::string Backend_Xml::getIfCtlProperty(xmlNodePtr node, std::string valname)
    {
      auto prop = (const char*) xmlGetNsProp(node, (const xmlChar*) valname.c_str(), (const xmlChar*) IFCTL_NAMESPACE_URI);

      if (prop == NULL)
        return std::string("");
      else
        return std::string(prop);
    }

    bool Backend_Xml::checkPathExists(std::string path)
    {
      // TODO: merge write branch
      path = "foo";
      return true;
    }

  } // namespace gearbox
} // namespace Belle2

