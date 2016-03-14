/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEARBOX_H
#define GEARBOX_H

#include <libxml/xpath.h>

#include <framework/gearbox/Interface.h>
#include <framework/gearbox/InputHandler.h>

#include "framework/gearbox/types.h"
#include "framework/gearbox/Backend.h"

#include <map>
#include <list>
#include <vector>
#include <string>
#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>

namespace boost {
  namespace python {
    class dict;
  }
}

namespace Belle2 {
  namespace xmldb {
    class Tree;
  }

  namespace gearbox {
    void* openXmlUri(const char*);
    class GBResult;
  }
  template <class KEY, class VALUE> class MRUCache;

  /** Singleton class responsible for loading detector parameters from an XML file.
   *
   * \sa GearboxModule
   */
  class Gearbox: public gearbox::Interface {
  public:
    /** Default cache size (in entries) for the Gearbox */
    enum { c_DefaultCacheSize = 200 };

    /** Struct for caching results from the xml file */
    struct PathValue {
      PathValue(): numNodes(0), value(""), unit("") {}
      /** number of nodes corresponding to the path */
      int numNodes;
      /** value of the first node if present, otherwise "" */
      std::string value;
      /** unit attribute of the first node if present, otherwise "" */
      std::string unit;
    };

    /** Struct to override a path in the XML file with a custom value */
    struct PathOverride {
      /** XPath expression of the path to override */
      std::string path {""};
      /** New value */
      std::string value {""};
      /** new Unit */
      std::string unit {""};
      /** if true, override all nodes when more than one node matches the XPath
       * expression, bail otherwise */
      bool multiple {false};
    };

    /** Free structures on destruction */
    ~Gearbox();

    /** Return reference to the Gearbox instance */
    static Gearbox& getInstance();

    /**
     * Select the backends to use to find resources
     * @param backends list of URIs identifying the backends starting with the
     *                 handler prefix followed by a colon, the rest is handler
     *                 specific
     */
    void setBackends(const std::vector<std::string>& backends);

    /** Clear list of backends */
    void clearBackends();

    /** Add an override for a given XPath expression */
    void addOverride(const PathOverride& poverride)
    {
      m_overrides.push_back(poverride);
    }

    /** Clear all existing overrides */
    void clearOverrides()
    {
      m_overrides.clear();
    }

    /**
     * Open connection to backend and parse tree
     * @param name Name of the tree to parse
     * @param database Load from the XML database
     * @param cacheSize maximum cache size in entries
     */
    void open(const std::string& name = "Belle2.xml", bool database = false, size_t cacheSize = c_DefaultCacheSize);

    /** Free internal structures of previously parsed tree and clear cache */
    void close();

    /**
     * Return the state of the Gearbox.
     * @return True if the Gearbox has an initialized backend and opened a document for reading.
     */
    bool isOpen() const { return m_xmlDocument != 0; }

    /**
     * Return the number of nodes a given path will expand to
     * @return number of nodes, 0 if path does not exist
     */
    virtual int getNumberNodes(const std::string& path = "") const
    {
      return getPathValue(path).numNodes;
    }

    /**
     * Get the parameter path as a string.
     * @exception gearbox::PathEmptyError if path is empty or does not exist
     * @param path Path of the parameter to get
     * @return value of the parameter
     */
    virtual std::string getString(const std::string& path = "") const throw(gearbox::PathEmptyError)
    {
      PathValue p = getPathValue(path);
      if (p.numNodes == 0) throw gearbox::PathEmptyError() << path;
      return p.value;
    }

    /**
     * Get the parameter path as a string.
     * if the parameter is empty or does not exist, defaultValue will be
     * returned.
     * @param path path of the parameter to get
     * @param defaultValue value to return if the path es empty or does not exist
     * @return value of the parameter
     */
    std::string getString(const std::string& path, const std::string& defaultValue) const
    {
      return gearbox::Interface::getString(path, defaultValue);
    }

    /**
     * Get the parameter path as string and also return the unit it was defined with.
     *
     * If no unit was defined, an empty string will be returned for the unit.
     * No parsing of the unit is performed, this funtion is primarily used by
     * getWithUnit.
     *
     * @exception gearbox::PathEmptyError if path is empty or does not exist
     * @param path Path of the parameter to get
     * @return value of the parameter
     */
    virtual std::pair<std::string, std::string> getStringWithUnit(const std::string& path = "") const throw(gearbox::PathEmptyError)
    {
      PathValue p = getPathValue(path);
      if (!p.numNodes) throw gearbox::PathEmptyError() << path;
      return make_pair(p.value, p.unit);
    }

    /**
     * Get the parameter path as a TObject
     * @exception gearbox::PathEmptyError if path is empty or does not exist
     * @exception gearbox::TObjectConversionError if the value could not be deserialized
     * @param path Path of the parameter to get
     * @return pointer to object, owned and managed by gearbox. Object will
     *         be deleted once it is no longer valid (e.g. after the current
     *         run if it belongs to this run)
     */
    virtual const TObject* getTObject(const std::string& path) const throw(gearbox::PathEmptyError, gearbox::TObjectConversionError);

    /**
     * Return GearDir representing a given DetectorComponent
     *
     * @param component Name of the DetectorComponent (e.g. IR, PXD)
     */
    GearDir getDetectorComponent(const std::string& component);

    /**
     * Register a new input handler
     * @param prefix prefix signaling which handler to use for a given
     *               backend uri. The prefix is everything up to the first
     *               colon in the backend uri
     * @param factory Pointer to the factory function which will return an
     *               instance of the handler
     */
    static void registerInputHandler(std::string prefix, gearbox::InputHandler::Factory* factory)
    {
      getInstance().m_registeredHandlers[prefix] = factory;
    }

    /**
     * Create a new Backend
     * @param backendType the type of the backend that is to be created
     * @param params configuration params passed to the backend
     * @return Pointer to the newly created backend
     */
    gearbox::BackendPtr createBackend(std::string backendType, const gearbox::BackendConfigParamSet& params);

    /** Wrapper for createBackend for python export
     * @param backendType the type of the backend that is to be created
     * @param parameters configuration params to be converted from the python dict and passed to the backend
     * @return Pointer to the newly created backend
     * */
    gearbox::BackendPtr createBackend(std::string backendType, const boost::python::dict& parameters);

    /**
     * Mount a backend to a certain path
     * @param backend the backend, that is to be mounted
     * @param mountPath the path within the backend, that is to be mounted on the mountpoint
     * @param mountPoint the path within the global hierarchy, where the Backend's content should be visible
     * @param mountMode the mount mode (overlay or merge nodes)
     * @return a handle to the specific Backend mount state
     */
    gearbox::BackendMountHandlePtr mountBackend(gearbox::BackendPtr backend, std::string mountPath, std::string mountPoint,
                                                gearbox::EMountMode mountMode);

    /**
     * Wrapper for mountBackend without return value (for python export)
     * @param backend the backend, that is to be mounted
     * @param mountPath the path within the backend, that is to be mounted on the mountpoint
     * @param mountPoint the path within the global hierarchy, where the Backend's content should be visible
     * @param mountMode the mount mode (overlay or merge nodes)
     * @param mountMode
     */
    void mountBackendAndForgetHandle(gearbox::BackendPtr backend, std::string mountPath, std::string mountPoint,
                                     gearbox::EMountMode mountMode);

    /**
     * Unmounts a backend (but does not destroy it!)
     * @param the mount handle, which was originally returned by mountBackend
     */
    void unmountBackend(gearbox::BackendMountHandlePtr);

    /**
     * Performs a query and prints its results
     * @param the XPath query, that is to be performed
     */
    void testQuery(std::string);


    /** Actually exports Python API. */
    static void exposePythonAPI();

    /**
     * Query the Interface and the matching backend
     * @param xpath the XPath query to be performed
     * @return the result set of the query
     */
    boost::shared_ptr<gearbox::QryResultSet> query(const std::string& xpath);

  private:
    /** Singleton: private constructor */
    Gearbox();
    /** Singleton: private copy constructor */
    Gearbox(const Gearbox& other) = delete;

    /** Function to be called when libxml requests a new input uri to be opened */
    gearbox::InputContext* openXmlUri(const std::string& uri) const;

    /** Change the value of a given path expression */
    void overridePathValue(const PathOverride& poverride);
    /** Return the (cached) value of a given path */
    PathValue getPathValue(const std::string& path) const;

    /** The handle for the tree in the XML database, when it is in use. */
    Belle2::xmldb::Tree* m_configTree;
    /** Flag indicating whether the last load operation used the database */
    bool m_database;

    /** Pointer to the libxml Document structure */
    xmlDocPtr m_xmlDocument;
    /** Pointer to the libxml XPath context */
    xmlXPathContextPtr m_xpathContext;
    /** Cache for already queried paths */
    mutable MRUCache<std::string, PathValue>* m_parameterCache;
    /** Map of queried objects (path -> TObject*). Objects will be removed once they are no longer valid. */
    mutable std::map<std::string, TObject*> m_ownedObjects;

    /** List of input handlers which will be used to find resources. */
    std::vector<gearbox::InputHandler*> m_handlers;
    /** Map of registered InputHandlers */
    std::map<std::string, gearbox::InputHandler::Factory*> m_registeredHandlers;

    /**
     * The existing Backends
     */
    std::vector<gearbox::Backend> backends;

    /** the existing overrides */
    std::vector<PathOverride> m_overrides;

    /**
     * Information regarding the active mounts
     */
    std::list<gearbox::BackendMountHandle> mountInfo;

    /**
     * Dispatch Query among backends
     * @param xpath the query to be dispatched
     * @param the result set to which the matching nodes should be added
     */
    void dispatchQuery(std::string xpath, gearbox::GBResult&);

    /**
     * Shows by how many mounts a backend is used
     * @param The backend that is to be investigated
     */
    void printBackendUseCount(gearbox::BackendPtr p);

    /**
     * Checks, whether a XPath query could match a certain path
     * @param xPath The xPath, that is to be analyzed
     * @param path The path, against that the matching process should take place
     * @param xPath_matchLevel How many labels (parts of the paths from the beginning) are consumed by the match
     * @return whether it matched or not
     */
    bool matchXPath2Path(const std::string& xPath, const std::string& path, int& xPath_matchLevel);

    /**
     * removes conditions from xpath and return its path-like parts
     * @param path the (x)Path to work on
     * @return the individual parts without conditions
     */
    std::vector<std::string> getSimplePathParts(std::string path);


    /**
     * friend to internal c-like function to interface libxml2 callback
     *
     * This function is declared as friend since it needs access to
     * Gearbox::openXmlUri, a function which should only be called by libxml2
     * and thus is declared private.
     */
    friend void* gearbox::openXmlUri(const char*);
  };

  /** Helper class to easily register new input handlers */
  template<class T> struct InputHandlerFactory {
    /** constructor, used by B2_GEARBOX_REGISTER_INPUTHANDLER macro.*/
    explicit InputHandlerFactory(const std::string& prefix)
    {
      Gearbox::registerInputHandler(prefix, factory);
    }
    /** create a new InputHandler of type T for given URI. */
    static gearbox::InputHandler* factory(const std::string& uri)
    {
      return new T(uri);
    }
  };

  /**
   * Helper macro to easily register new input handlers. It will create a factory function for
   * the InputHandler and register it with the Gearbox instance
   * @param classname Classname to create factory for
   * @param prefix    Prefix to register the handler for, e.g. "file" to register
   *                  a handler responsible for uris starting with file:
   */
#define B2_GEARBOX_REGISTER_INPUTHANDLER(classname,prefix)\
  InputHandlerFactory<classname> Gearbox_InputHandlerFactory_##classname(prefix)
}

#endif
