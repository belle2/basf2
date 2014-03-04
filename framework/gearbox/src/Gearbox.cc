#include <boost/python/scope.hpp>
#include <boost/python/class.hpp>
#include <boost/python/enum.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/manage_new_object.hpp>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/Stream.h>

#include <libxml/parser.h>
#include <libxml/xinclude.h>
#include <libxml/xmlIO.h>
#include <memory>
#include <cstring>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <list>

#include <framework/gearbox/Backend_Postgres1.h>
#include <framework/gearbox/Backend_Postgres2.h>
#include <framework/gearbox/Backend_Xml.h>

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

  Gearbox::Gearbox(): m_xmlDocument(0), m_xpathContext(0), m_parameterCache(DefaultCacheSize)
  {
    xmlInitParser();
    LIBXML_TEST_VERSION;
    xmlRegisterInputCallbacks(gearbox::matchXmlUri, gearbox::openXmlUri,
                              gearbox::readXmlData, gearbox::closeXmlContext);
    // do random seed only once
    gearbox::randomSeed();

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
      B2FATAL("No backends defined, please use Gearbox::setBackends() first to specify how to access XML files.");
    //Open document
    m_xmlDocument = xmlParseFile(name.c_str());

#if LIBXML_VERSION >= 20700
    //libxml >= 2.7.0 introduced some limits on node size etc. which breaks reading VXDTF files
    xmlXIncludeProcessFlags(m_xmlDocument, XML_PARSE_HUGE);
#else
    xmlXIncludeProcess(m_xmlDocument);
#endif
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

    typedef std::pair<std::string, TObject*> EntryType;
    BOOST_FOREACH(EntryType entry, m_ownedObjects) {
      delete entry.second;
    }
    m_ownedObjects.clear();

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
    //Add to cache, empty or not: results won't change
    m_parameterCache.insert(path, value);

    xmlXPathFreeObject(result);
    return value;
  }

  const TObject* Gearbox::getTObject(const std::string& path) const throw(gearbox::PathEmptyError, gearbox::TObjectConversionError)
  {
    //do we already have an object for this path?
    std::map<std::string, TObject*>::const_iterator it = m_ownedObjects.find(path);
    if (it != m_ownedObjects.end())
      return it->second;

    const string& value = getString(path);
    TObject* object = NULL;
    if (value.find("<Object") == 0) {
      //most likely is XML-serialized object
      object = Stream::deserializeXML(value);
    } else {
      //assume base64-encoded raw data.
      object = Stream::deserializeEncodedRawData(value);
    }
    if (!object)
      throw gearbox::TObjectConversionError() << path;

    m_ownedObjects[path] = object;

    return object;
  }


  GearDir Gearbox::getDetectorComponent(const string& component)
  {
    return GearDir("/Detector/DetectorComponent[@name='" + component + "']/Content");
  }



  gearbox::BackendPtr Gearbox::createBackend(std::string backendType, const gearbox::BackendConfigParamSet& params)
  {

    gearbox::BackendPtr ret;

    try {

      if (backendType == "Postgres1") {
        ret = gearbox::BackendPtr(new gearbox::Backend_Postgres1(params));
      } else if (backendType == "Postgres2") {
        ret = gearbox::BackendPtr(new gearbox::Backend_Postgres2(params));
      } else if (backendType == "Xml") {
        ret = gearbox::BackendPtr(new gearbox::Backend_Xml(params));
      } else {
        throw (std::string("Unknown Backend Type. Valid Types are 'Postgres1', 'Postgres2' and 'Xml' (case sensitive)"));
      }

    } catch (std::string& s) {
      B2ERROR("Gearbox::create_backend: Could not create backend of type " + backendType + ". Reason: " + s);
      exit(1);
    }

    return ret;

  }



  gearbox::BackendPtr Gearbox::createBackend(std::string backendType, const boost::python::dict& parameters)
  {
    boost::python::list dictKeys = parameters.keys();
    int nKey = boost::python::len(dictKeys);
    gearbox::BackendConfigParamSet paramMap;

    //Loop over all keys in the dictionary
    for (int iKey = 0; iKey < nKey; ++iKey) {
      boost::python::object currKey = dictKeys[iKey];
      boost::python::extract<std::string> keyProxy(currKey);

      if (keyProxy.check()) {
        boost::python::object currValue = parameters[currKey];
        //I don't see why this doesn't work (proxy.check() always returns false)
        //boost::python::extract<gearbox::BackendConfigParam> valueProxy(currValue);
        boost::python::extract<int> intProxy(currValue);
        boost::python::extract<double> dblProxy(currValue);
        boost::python::extract<std::string> strProxy(currValue);

        //check types, from more strict to less strict
        gearbox::BackendConfigParam variant;
        if (intProxy.check()) {
          variant = intProxy();
        } else if (dblProxy.check()) {
          variant = dblProxy();
        } else if (strProxy.check()) {
          variant = strProxy();
        } else {
          B2ERROR("Setting the backend parameters from a python dictionary: invalid value for key '" << keyProxy() << "')!");
          throw std::exception();
        }

        //ok, add to map
        paramMap[keyProxy()] = variant;
      } else {
        B2ERROR("Setting the backend parameters from a python dictionary: invalid key in dictionary!");
        throw std::exception();
      }
    }

    return  createBackend(backendType, paramMap);
  }

  void Gearbox::exposePythonAPI()
  {
    using namespace boost::python;

    //Not creatable by user
    class_<gearbox::Backend, gearbox::BackendPtr, boost::noncopyable>("Backend", no_init);
    class_<gearbox::BackendMountHandlePtr, boost::noncopyable>("BackendMountHandle", no_init);

    gearbox::BackendPtr(Gearbox::*createBackendPython)(std::string, const boost::python::dict&) = &Gearbox::createBackend;

    enum_<gearbox::EMountMode>("EMountMode")
    .value("overlay", gearbox::EMountMode::overlay)
    .value("merge", gearbox::EMountMode::merge)
    ;

    class_<Gearbox, boost::noncopyable>("Gearbox", no_init)
    .def("create_backend", createBackendPython, return_value_policy<boost::python::manage_new_object>())
    .def("mount_backend", &Gearbox::mountBackendAndForgetHandle) // even with boost::python::iterator the BackendMountHandle (which is a list iterator) can not get exported or unsufficient documentation
    .def("testQuery", &Gearbox::testQuery)
    .def("unmount_backend", &Gearbox::unmountBackend)
    .def("printBackendUseCount", &Gearbox::printBackendUseCount)
    ;

    //export global object 'gearbox'
    scope global;
    Gearbox& instance = Gearbox::getInstance();
    global.attr("gearbox") = object(ptr(&instance));
  }

  void Gearbox::mountBackendAndForgetHandle(gearbox::BackendPtr backend, std::string mountPath, std::string mountPoint, gearbox::EMountMode mountMode)
  {
    this->mountBackend(backend, std::move(mountPath), std::move(mountPoint), mountMode);
  }


  void Gearbox::printBackendUseCount(gearbox::BackendPtr /*p*/)
  {
    // subtract by 1 in order to hide the increased use count due to this function itself
    // makes only sense if BackendPtr is a shared_ptr
    // TODO
    //B2INFO("Use count of shared_ptr is '"+ std::to_string(p.use_count()-1) +"' (python itself consumes 1)");
  }

  gearbox::BackendMountHandlePtr Gearbox::mountBackend(gearbox::BackendPtr backend, std::string mountPath, std::string mountPoint, gearbox::EMountMode mountMode)
  {

    if (mountPath.front() != '/')
      B2ERROR("Gearbox::mountBackend: Invalid mountPath '" + mountPath + "' - has to begin with '/' character");

    if (mountPoint.front() != '/')
      B2ERROR("Gearbox::mountBackend: Invalid mountPoint '" + mountPoint + "' - has to begin with '/' character");

    if (mountPath.back() != '/')
      mountPath.append("/");

    if (mountPoint.back() != '/')
      mountPoint.append("/");

    this->mountInfo.emplace_back(backend, mountPath, mountPoint, mountMode);

    return --this->mountInfo.end();

  }

  void Gearbox::unmountBackend(gearbox::BackendMountHandlePtr m)
  {
    this->mountInfo.erase(m);
  }

  void Gearbox::testQuery(std::string qry)
  {

    gearbox::printResultSet(*this->query(qry));

  }

  boost::shared_ptr<gearbox::QryResultSet> Gearbox::query(const std::string& xpath)
  {

    std::vector<std::string> xpath_parts;
    boost::split(xpath_parts, xpath, boost::is_any_of("|"));

    gearbox::GBResult res;

    for (auto & xp_single : xpath_parts) {
      this->dispatchQuery(xp_single, res);
    }

    return res.getResultSetPtr();

  }

  std::vector<std::string> Gearbox::getSimplePathParts(std::string path)
  {

    // remove xpath's square-bracket conditions (we cannot evaluate them now)
    path = boost::regex_replace(path, boost::regex("\\[[^\\]]\\]"), "");

    // remove first slash
    path.erase(0, 1);

    std::vector<std::string> path_parts;
    boost::split(path_parts, path, boost::is_any_of("/"));

    return path_parts;

  }

  bool Gearbox::matchXPath2Path(const std::string& xPath, const std::string& path, int& xPath_matchLevel)
  {

    std::vector<std::string> xPath_parts(this->getSimplePathParts(xPath));
    std::vector<std::string> path_parts(this->getSimplePathParts(path));

    xPath_matchLevel = 0;
    auto path_actPart = path_parts.begin();

    for (auto xPath_actPart = xPath_parts.cbegin(); xPath_actPart != xPath_parts.cend(); ++xPath_actPart) {

      B2DEBUG(2, "Gearbox::matchXPath2Path: matchlevel '" + std::to_string(xPath_matchLevel) + "' xPath_actPart is '" + *xPath_actPart + "' and path_actPart is '" + *path_actPart + "'");

      if ((*xPath_actPart == "" && xPath_actPart == xPath_parts.cend() - 1)
          ||      // xpath or path have an empty label at the end => is not relevant for comparison!
          (*path_actPart == "" && path_actPart == path_parts.end() - 1)
         )
        break;

      if (*xPath_actPart == "") // having xpath like "test1//test2" => "//" consumes complete path => match
        break;
      else if (*xPath_actPart == "*" || *xPath_actPart == *path_actPart) { // xpath wildcard label or label identical
        xPath_matchLevel++;
        path_actPart++;
      } else {  // we do not need to continue comparison, because path won't match anymore
        return false;
      }

    }

    return true;

  }


  void Gearbox::dispatchQuery(std::string xPath, gearbox::GBResult& res)
  {

    int matchlevel;

    // loop forwards over all mounts and check,
    // whether they match the query
    for (auto & mI : this->mountInfo) {

      if (this->matchXPath2Path(xPath, mI.mountPoint, matchlevel)) {

        boost::iterator_range<std::string::iterator> matchedRange = boost::algorithm::find_nth(xPath, "/", matchlevel);

        B2INFO("Gearbox::dispatchQuery: found Backend: mountpath '" + mI.mountPath + "', mountpoint '" + mI.mountPoint
               + "', matchlevel '" + std::to_string(matchlevel) + "', matched range '" + std::string(matchedRange.begin(), matchedRange.end())
               + "', rest '" + std::string(matchedRange.end(), xPath.end()) + "'");


        if (mI.mountMode == gearbox::EMountMode::overlay) {
          res.erasePath(mI.mountPoint);
        }

        mI.backend->query(mI.mountPath + std::string(matchedRange.end(), xPath.end()), res.ensurePath(mI.mountPoint), mI.mountPath, res);

      }

    }

  }


}
