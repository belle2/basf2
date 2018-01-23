/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <libxml/xpath.h>

#include <framework/gearbox/Interface.h>
#include <framework/gearbox/InputHandler.h>

#include <map>
#include <vector>
#include <string>

namespace Belle2 {
  namespace gearbox {
    void* openXmlUri(const char*);
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
     * @param cacheSize maximum cache size in entries
     */
    void open(const std::string& name = "Belle2.xml", size_t cacheSize = c_DefaultCacheSize);

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
    virtual std::string getString(const std::string& path = "") const noexcept(false)
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
    virtual std::pair<std::string, std::string> getStringWithUnit(const std::string& path = "") const noexcept(false)
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
    virtual const TObject* getTObject(const std::string& path) const noexcept(false);

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
    static void registerInputHandler(const std::string& prefix, gearbox::InputHandler::Factory* factory)
    {
      getInstance().m_registeredHandlers[prefix] = factory;
    }

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

    /** the existing overrides */
    std::vector<PathOverride> m_overrides;

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
