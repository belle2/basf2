/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/FrameworkExceptions.h>

#include <memory>

#include <string>
#include <map>
#include <list>

namespace boost {
  namespace filesystem {
    class directory_entry;
  }
}

namespace Belle2 {

  class Module;
  class ModuleProxyBase;

  /**
   * The ModuleManager Class.
   *
   * This class is responsible for creating and managing new module instances.
   * The module manager can only find those modules, whose shared library files are located in one of the
   * directories known to the module manager. Directories can be added to the directory list with
   * the method addModuleSearchPath(). By adding a new directory, the directory is searched for *.map
   * files which specify the kind of modules that can be found in the shared library files. The map files
   * are parsed and the found modules are added to an internal list.
   *
   * If a shared library is loaded by the module manager, all modules inside the library automatically
   * register themselves to the module manager. This is done by a so-called proxy class. Each module has a proxy
   * class attached to it, which does the registration with the module manager and is later used to create
   * the actual instance of the module. By using a proxy class, the module itself is only instantiated
   * if the user requests it.
   *
   * The creation of a new module instance works as following:
   * 1) The user requests a new module by calling the method registerModule() with the module name given as a parameter.<br>
   * 2) The ModuleManager checks if a proxy class was already registered for the given module name. If yes, the proxy creates
   * a new module instance. Otherwise the modules known to the manager are searched for the given module name. If the
   * module is known to the module manager the associated shared library is dynamically loaded.
   *
   * This class is designed as a singleton.
   */
  class ModuleManager {

  public:

    //Define exceptions
    /** Exception is thrown if the requested module could not be created by the ModuleManager. */
    BELLE2_DEFINE_EXCEPTION(ModuleNotCreatedError, "Cannot create module %1%: %2%")

    /**
     * Static method to get a reference to the ModuleManager instance.
     *
     * @return A reference to an instance of this class.
     */
    static ModuleManager& Instance();

    /**
     * Registers a module proxy.
     *
     * Each module has a proxy assigned to it, which registers itself to the ModuleManager.
     * The proxy is then responsible to create an instance of a module.
     * @param moduleProxy Pointer to the module proxy which should be registered.
     */
    void registerModuleProxy(ModuleProxyBase* moduleProxy);

    /**
     * Adds a new filepath to the list of filepaths which are searched for a requested module.
     *
     * If a module is requested by the method registerModule(), all module search paths
     * are scanned for the first occurrence of a shared library carrying the module
     * name in its filename.
     * E.g modules in the path added with the first call to addModuleSearchPath() will take precedence over those added later on.
     *
     * @param path The module search path which should be added to the list of paths.
     */
    void addModuleSearchPath(const std::string& path);

    /**
     * Returns a reference to the list of the modules search filepaths.
     *
     * @return A reference to the list of the modules search filepaths.
     */
    const std::list<std::string>& getModuleSearchPaths() const;

    /**
     * Returns a map of all modules that were found in the module search paths.
     *
     * The key of the map represents the module name and the value the shared library file in which the module is defined.
     *
     * @return A map of all modules that were found in the module search paths.
     */
    const std::map<std::string, std::string>& getAvailableModules() const;

    /**
     * Creates an instance of a module and registers it to the ModuleManager.
     *
     * Each module carries an unique name, which is used to find the corresponding shared
     * library file. The found shared library is dynamically loaded and the module proxy registered.
     * Using the proxy an instance of the module is created and returned.
     *
     * The search order is as following:<br>
     * 1) First the method checks if a proxy associated with the module name was already registered<br>
     * 2) If not, the method checks if a shared library path was given and tries to load the module from that shared library<br>
     * 3) If not, the method searches for the module name in the map of known modules due to the given search paths<br>
     *
     * @param moduleName The unique name of the Module which should be created.
     * @param sharedLibPath Optional: The shared library from which the module should be registered (not a map file!).
     * @return A shared pointer to the created module instance.
     */
    std::shared_ptr<Module> registerModule(const std::string& moduleName,
                                           std::string sharedLibPath = "") noexcept(false);

    /**
     * Returns a reference to the list of created modules.
     *
     * @return A reference to the list of created modules.
     */
    const std::list< std::shared_ptr<Module> >& getCreatedModules() const;

    /**
     * Returns a list of those modules which carry property flags matching the specified ones.
     *
     * Loops over all module instances specified in a list
     * and adds those to the returned list whose property flags match the given property flags.
     *
     * @param modulePathList The list containing all module instances added to a path.
     * @param propertyFlags The flags for the module properties.
     * @return A list containing those modules which carry property flags matching the specified ones.
     */
    static std::list< std::shared_ptr<Module> > getModulesByProperties(const std::list< std::shared_ptr<Module> >& modulePathList,
        unsigned int propertyFlags);

    /** Returns true if and only if all modules in list have the given flag (or list is empty). */
    static bool allModulesHaveFlag(const std::list<std::shared_ptr<Module>>& list, unsigned int flag);

    /** Delete all created modules. */
    void reset();


  private:
    /** List of all checked and validated filepaths that are searched for map files. */
    std::list<std::string> m_moduleSearchPathList;

    /** Maps the module name to the filename of the shared library which containes the module. */
    std::map<std::string, std::string> m_moduleNameLibMap;

    std::map<std::string, ModuleProxyBase*> m_registeredProxyMap; /**< Maps the module name to a pointer of its proxy. */
    std::list< std::shared_ptr<Module> > m_createdModulesList;  /**< List of all created modules. */

    /**
     * Adds the module names defined in the map file to the list of known module names.
     *
     * If the given map already contains this module, an error is raised
     *
     * @param moduleNameLibMap map to be filled.
     * @param mapPath The filename (path+name) of the map file which should be parsed for module names.
     */
    static void fillModuleNameLibMap(std::map<std::string, std::string>& moduleNameLibMap,
                                     const boost::filesystem::directory_entry& mapPath);

    /**
     * The constructor is hidden to avoid that someone creates an instance of this class.
     */
    ModuleManager();

    /**
     * Disable/Hide the copy constructor.
     */
    ModuleManager(const ModuleManager&);

    /**
     * Disable/Hide the copy assignment operator.
     */
    ModuleManager& operator=(const ModuleManager&);

    /**
     * The ModuleManager destructor.
     * Deletes the ModuleManager.
     */
    ~ModuleManager();
  };

} //end of namespace Belle2
