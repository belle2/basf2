#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/core/MRUCache.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/Stream.h>

#include <libxml/parser.h>
#include <libxml/xinclude.h>
#include <libxml/xmlIO.h>
#include <cstring>
#include <boost/algorithm/string.hpp>
#include <regex>
#include <list>

#include <TObject.h>

using namespace std;

namespace Belle2 {

  namespace gearbox {
    /** Callback function to let libxml2 know that we handle all input files */
    static int matchXmlUri(const char* uri)
    {
      //Ignore uris including file protocol. They are only used if loading of a
      //resource failed, then libxml will try to look in /etc/xml/catalog if it
      //can find the file there.
      if (boost::starts_with(uri, "file:/")) return 0;
      return 1;
    }

    /** Callback function to create a GearInputContext for every "file" processed by libxml2 */
    void* openXmlUri(const char* uri)
    {
      B2DEBUG(200, "Request to open " << uri);
      InputContext* context = Gearbox::getInstance().openXmlUri(uri);
      return (void*) context;
    }

    /** Callback function to read data from GearInputContext when requested by libxml2 */
    static int readXmlData(void* context, char* buffer, int buffsize)
    {
      //B2DEBUG(200,"Calling read to get " << buffsize << " bytes");
      InputContext* gearContext = static_cast<InputContext*>(context);
      return gearContext->readXmlData(buffer, buffsize);
    }

    /** Callback function to close GearInputContext when requested by libxml2 */
    static int closeXmlContext(void* context)
    {
      B2DEBUG(200, "Closing context");
      InputContext* gearContext = static_cast<InputContext*>(context);
      delete gearContext;
      return 0;
    }
  }

  Gearbox::Gearbox(): m_xmlDocument(0), m_xpathContext(0), m_parameterCache(new MRUCache<std::string, PathValue>(c_DefaultCacheSize))
  {
    xmlInitParser();
    LIBXML_TEST_VERSION;
  }

  Gearbox::~Gearbox()
  {
    close();
    clearBackends();
    delete m_parameterCache;
  }

  Gearbox& Gearbox::getInstance()
  {
    static Gearbox instance;
    return instance;
  }

  gearbox::InputContext* Gearbox::openXmlUri(const string& uri) const
  {
    //Check input handlers one by one
    for (gearbox::InputHandler* handler : m_handlers) {
      //try to create context for uri, return if success
      gearbox::InputContext* context = handler->open(uri);
      if (context) return context;
    }
    B2ERROR("Could not find data for uri '" << uri << "'");
    return 0;
  }

  void Gearbox::setBackends(const vector<string>& backends)
  {
    clearBackends();
    for (const string& backend : backends) {
      B2DEBUG(300, "Adding InputHandler for '" << backend << "'");
      //Find correct InputHandler, assuming file backend by default if there is no colon in the
      //uri
      string prefix("file");
      string accessinfo(backend);
      size_t colon = backend.find(":");
      if (colon != string::npos) {
        prefix = backend.substr(0, colon);
        accessinfo = backend.substr(colon + 1);
      }
      map<string, gearbox::InputHandler::Factory*>::const_iterator it = m_registeredHandlers.find(prefix);
      if (it == m_registeredHandlers.end()) {
        B2ERROR("Could not find input handler to handle '" << backend << "', ignoring");
        continue;
      }
      gearbox::InputHandler* handler = it->second(accessinfo);
      if (handler) {
        m_handlers.push_back(handler);
      } else {
        B2ERROR("Problem creating input handler to handle '" << backend << "', ignoring");
      }
    }
  }

  void Gearbox::clearBackends()
  {
    for (gearbox::InputHandler* handler : m_handlers) delete handler;
    m_handlers.clear();
  }

  void Gearbox::open(const std::string& name, size_t cacheSize)
  {
    //Check if we have an open connection and close first if so
    if (m_xmlDocument) close();
    //Check if we have at least one backend
    if (m_handlers.empty())
      B2FATAL("No backends defined, please use Gearbox::setBackends() first to specify how to access XML files.");

    // register input callbacks for opening the files
    xmlRegisterInputCallbacks(gearbox::matchXmlUri, gearbox::openXmlUri,
                              gearbox::readXmlData, gearbox::closeXmlContext);

    //Open document
    m_xmlDocument = xmlParseFile(name.c_str());
    //libxml >= 2.7.0 introduced some limits on node size etc. which breaks reading VXDTF files
    xmlXIncludeProcessFlags(m_xmlDocument, XML_PARSE_HUGE);

    // reset input callbacks
    xmlPopInputCallbacks();

    if (!m_xmlDocument) B2FATAL("Could not connect gearbox to " << name);
    m_xpathContext = xmlXPathNewContext(m_xmlDocument);
    if (!m_xpathContext) B2FATAL("Could not create XPath context");

    //Apply overrides
    for (const auto& poverride : m_overrides) {
      overridePathValue(poverride);
    }

    //Speeds up XPath computation on static documents.
    xmlXPathOrderDocElems(m_xmlDocument);

    //Set cachesize
    m_parameterCache->setMaxSize(cacheSize);
  }

  void Gearbox::close()
  {
    if (m_xpathContext) xmlXPathFreeContext(m_xpathContext);
    if (m_xmlDocument) xmlFreeDoc(m_xmlDocument);
    m_xpathContext = 0;
    m_xmlDocument = 0;

    for (auto& entry : m_ownedObjects) {
      delete entry.second;
    }
    m_ownedObjects.clear();

    m_parameterCache->clear();
  }

  void Gearbox::overridePathValue(const PathOverride& poverride)
  {
    if (m_xpathContext == NULL) B2FATAL("Gearbox is not connected");
    //Make sure it ends with a slash
    string query = ensureNode(poverride.path);
    //Ok, lets search for the path
    B2INFO("Override '" << poverride.path << "' with '" << poverride.value
           << "' (unit: '" << poverride.unit << "')");
    xmlXPathObjectPtr result = xmlXPathEvalExpression((xmlChar*) query.c_str(), m_xpathContext);
    if (result != NULL && result->type == XPATH_NODESET && !xmlXPathNodeSetIsEmpty(result->nodesetval)) {
      //Found it, so let's replace the content
      int numNodes = xmlXPathNodeSetGetLength(result->nodesetval);
      if (!poverride.multiple && numNodes > 1) {
        B2ERROR("Cannot override '" << poverride.path << "': more than one node found");
        return;
      } else {
        B2DEBUG(200, "Found " << numNodes << " nodes, overriding them all");
      }
      for (int i = numNodes - 1; i >= 0; --i) {
        xmlNodePtr node = result->nodesetval->nodeTab[i];
        //Check if children are only TEXT nodes
        bool textOnly(true);
        for (xmlNodePtr child = node->children; child; child = child->next) {
          textOnly &= child->type == XML_TEXT_NODE;
          if (!textOnly) break;
        }
        if (!textOnly) {
          B2ERROR("Cannot override '" << poverride.path << "': not just text content");
          continue;
        }
        xmlNodeSetContent(node, BAD_CAST poverride.value.c_str());

        //Is the path an element? if so replace the unit, otherwise warn
        if (node->type != XML_ELEMENT_NODE) {
          if (!poverride.unit.empty())
            B2WARNING("Cannot set unit '" << poverride.unit << "' on '"
                      << poverride.path << "': not an element");
        } else {
          xmlSetProp(node, BAD_CAST "unit", BAD_CAST poverride.unit.c_str());
        }
        //From libxml example xpath2.c:
        //All the elements returned by an XPath query are pointers to
        //elements from the tree *except* namespace nodes where the XPath
        //semantic is different from the implementation in libxml2 tree.
        //As a result when a returned node set is freed when
        //xmlXPathFreeObject() is called, that routine must check the
        //element type. But node from the returned set may have been removed
        //by xmlNodeSetContent() resulting in access to freed data.
        //There is 2 ways around it:
        //  - make a copy of the pointers to the nodes from the result set
        //    then call xmlXPathFreeObject() and then modify the nodes
        //or
        //  - remove the reference to the modified nodes from the node set
        //    as they are processed, if they are not namespace nodes.
        if (node->type != XML_NAMESPACE_DECL)
          result->nodesetval->nodeTab[i] = NULL;
      }
    } else {
      B2ERROR("Cannot override '" << poverride.path << "': not found");
    }
    xmlXPathFreeObject(result);
  }

  Gearbox::PathValue Gearbox::getPathValue(const std::string& path) const
  {
    PathValue value;
    if (m_xpathContext == NULL) B2FATAL("Gearbox is not connected");
    //Get from cache if possible
    if (m_parameterCache->retrieve(path, value)) {
      return value;
    }
    //Nothing in cache, query xml
    string query = ensureNode(path);
    B2DEBUG(1000, "Gearbox XPath query: " << query);
    xmlXPathObjectPtr result = xmlXPathEvalExpression((xmlChar*) query.c_str(), m_xpathContext);
    if (result != NULL && result->type == XPATH_NODESET && !xmlXPathNodeSetIsEmpty(result->nodesetval)) {
      value.numNodes = xmlXPathNodeSetGetLength(result->nodesetval);
      xmlNodePtr node = result->nodesetval->nodeTab[0];
      //Example: <foo><bar/></foo>
      // - bar has no children, so node->children is 0
      // - foo has not text children, so node->children->content should be 0
      //   but xmlXPathOrderDocElems assigns them an index<0 to speed up XPath
      //   so we have to cast to a long integer and check if it is positive
      if (node->children && (long)node->children->content > 0) {
        xmlChar* valueString = xmlNodeListGetString(m_xmlDocument, node->children, 1);
        value.value = (char*)valueString;
        xmlFree(valueString);
      }
      //See if we have a unit attribute and add it
      xmlAttrPtr attribute = node->properties;
      while (attribute) {
        B2DEBUG(1001, "Checking attribute " << attribute->name);
        if (!strcmp((char*)attribute->name, "unit")) {
          B2DEBUG(1001, "found Unit " << attribute->children->content);
          value.unit = (char*)attribute->children->content;
          break;
        }
        attribute = attribute->next;
      }
      //Remove leading and trailing whitespaces
      boost::trim(value.value);
      boost::trim(value.unit);
    }
    //Add to cache, empty or not: results won't change
    m_parameterCache->insert(path, value);
    B2DEBUG(1000, "Gearbox XPath result: " << value.numNodes << ", " << value.value << ", " << value.unit);

    xmlXPathFreeObject(result);
    return value;
  }

  const TObject* Gearbox::getTObject(const std::string& path) const noexcept(false)
  {
    //do we already have an object for this path?
    std::map<std::string, TObject*>::const_iterator it = m_ownedObjects.find(path);
    if (it != m_ownedObjects.end())
      return it->second;

    const string& value = getString(path);
    //assume base64-encoded raw data.
    TObject* object = Stream::deserializeEncodedRawData(value);
    if (!object)
      throw gearbox::TObjectConversionError() << path;

    m_ownedObjects[path] = object;

    return object;
  }


  GearDir Gearbox::getDetectorComponent(const string& component)
  {
    return GearDir("/Detector/DetectorComponent[@name='" + component + "']/Content");
  }
}
