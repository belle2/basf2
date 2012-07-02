#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <libxml/parser.h>
#include <libxml/xinclude.h>
#include <libxml/xmlIO.h>
#include <memory>
#include <cstring>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

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
      InputContext* gearContext = (InputContext*) context;
      return gearContext->readXmlData(buffer, buffsize);
    }

    /** Callback function to close GearInputContext when requested by libxml2 */
    static int closeXmlContext(void* context)
    {
      B2DEBUG(200, "Closing context");
      InputContext* gearContext = (InputContext*) context;
      delete gearContext;
      return 0;
    }
  }

  Gearbox::Gearbox(): m_xmlDocument(0), m_xpathContext(0), m_parameterCache(DefaultCacheSize)
  {
    xmlInitParser();
    LIBXML_TEST_VERSION;
    xmlRegisterInputCallbacks(gearbox::matchXmlUri, gearbox::openXmlUri,
                              gearbox::readXmlData, gearbox::closeXmlContext);
  }

  Gearbox& Gearbox::getInstance()
  {
    static auto_ptr<Gearbox> instance(new Gearbox());
    return *instance;
  }

  gearbox::InputContext* Gearbox::openXmlUri(const string& uri) const
  {
    //Check input handlers one by one
    BOOST_FOREACH(gearbox::InputHandler * handler, m_handlers) {
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
    BOOST_FOREACH(const string & backend, backends) {
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
    BOOST_FOREACH(gearbox::InputHandler * handler, m_handlers) delete handler;
    m_handlers.clear();
  }

  void Gearbox::open(const std::string& name, size_t cacheSize)
  {
    //Check if we have an open connection and close first if so
    if (m_xmlDocument) close();
    //Check if we have at least one backend
    if (m_handlers.empty())
      B2FATAL("No backends defined, please use Geabox::setBackends() first to specify how to access XML files");
    //Open document
    m_xmlDocument = xmlParseFile(name.c_str());

    xmlXIncludeProcess(m_xmlDocument);
    //Speeds up XPath computation on static documents.
    xmlXPathOrderDocElems(m_xmlDocument);

    if (!m_xmlDocument) B2FATAL("Could not connect gearbox to " << name);
    m_xpathContext = xmlXPathNewContext(m_xmlDocument);
    if (!m_xpathContext) B2FATAL("Could not create XPath context");

    //Set cachesize
    m_parameterCache.setMaxSize(cacheSize);
  }

  void Gearbox::close()
  {
    if (m_xpathContext) xmlXPathFreeContext(m_xpathContext);
    if (m_xmlDocument) xmlFreeDoc(m_xmlDocument);
    m_xpathContext = 0;
    m_xmlDocument = 0;
    m_parameterCache.clear();
  }

  Gearbox::PathValue Gearbox::getPathValue(const std::string& path) const
  {
    PathValue value;
    if (m_xpathContext == NULL) B2FATAL("Gearbox is not connected");
    //Get from cache if possible
    if (m_parameterCache.retrieve(path, value)) {
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
        B2DEBUG(100, "Checking attribute " << attribute->name);
        if (!strcmp((char*)attribute->name, "unit")) {
          B2DEBUG(100, "found Unit " << attribute->children->content);
          value.unit = (char*)attribute->children->content;
          break;
        }
        attribute = attribute->next;
      }
      //Remove leading and trailing whitespaces
      boost::trim(value.value);
      boost::trim(value.unit);
    }
    //Add to cache, empty or not: results wont change
    m_parameterCache.insert(path, value);

    xmlXPathFreeObject(result);
    return value;
  }

  GearDir Gearbox::getDetectorComponent(const string& component)
  {
    return GearDir("/Detector/DetectorComponent[@name='" + component + "']/Content");
  }
}
