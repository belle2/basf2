/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MODULEMANAGER_H_
#define MODULEMANAGER_H_

#include <framework/fwcore/Module.h>
#include <framework/fwcore/FwExceptions.h>

#include <string>
#include <map>
#include <set>
#include <list>


namespace Belle2 {

  //! The ModuleManager Class
  /*! This class manages the dynamically loaded modules.
      It provides methods to load modules from a shared file (module library) and
      to create instances of the found modules.

      This class is designed as a singleton.
  */

  class ModuleManager {

  public:

    //-----------------------------------------------------------
    //! Class for registering modules
    template<class AModule> class Registrator {
    public:

      //! Constructor
      Registrator() {
        m_module = new AModule;
        ModuleManager::Instance().registerModule(m_module);
      }

      //! Destructor
      ~Registrator() {
        delete m_module;
      }

    private:
      Module* m_module; /*!< Stores the registered module. */
    };
    //-----------------------------------------------------------

    //! Static method to get a reference to the ModuleManager instance.
    /*!
      \return A reference to an instance of this class.
    */
    static ModuleManager& Instance();

    //! Searches for module libraries in the given path and dynamically loads them.
    /*!
     \param path The path to the folder or directly to the shared file, which contains module libraries or modules.
    */
    void loadModuleLibs(const std::string& path);

    //!  Registers a module.
    /*
     Each module registers itself to the Module Manager using this method.
     \param module Pointer to the module which should be registered.
     */
    void registerModule(Module* module);

    //! Returns a reference to a module given by its type (string).
    /*!
     The returned module is taken from the list of self registered
     modules and therefore not meant to be used in paths.

     If a module with the given type is not found, an exception of
     type FwExcModuleTypeNotFound is thrown.

     \param type The type string of the module which should be returned.
     \return A reference to the module given by its type.
    */
    const Module& getModuleByType(const std::string& type) const throw(FwExcModuleTypeNotFound);

    //!  Returns a list of all available modules.
    /*
     \return A list of all available modules.
     */
    std::list<ModulePtr> getAvailableModules() const;

    //! Creates a new module of a given type.
    /*!
     The ModuleManager does not take ownership of the returned Module.

     If the module could not be created, an exception of type FwExcModuleNotCreated is thrown.

     \param type The type string of the module which should be created.
     \return A pointer to the newly created module.
    */
    ModulePtr createModule(const std::string& type) const throw(FwExcModuleNotCreated);

    //! Adds a new file extension to the list of supported file extension for module libraries.
    /*!
     The module manager searches for module library files having specific file extensions. This
     method allows to add additional supported file extensions.

     \param fileExt The new file extension.
    */
    void addLibFileExtension(const std::string& fileExt);


  private:

    std::map<const std::string, void*>   m_nameLibraryMap; /*!< Maps the name of a library to its pointer. */
    std::map<const std::string, Module*> m_typeModuleMap;  /*!< Maps the type of a module to its pointer. */
    std::set<std::string> m_libFileExtensions;             /*!< List of all supported file extensions for module library files. */

    //! Loads the library shared file given by the library path.
    /*!
     \param libName The name of the library which will be loaded.
     \param libPath The path to the library shared file.
    */
    void loadLibrary(const std::string& libName, const std::string& libPath);

    //! Closes all open libraries.
    void closeOpenLibraries();

    //! Returns true if the given file extension is supported by the framework.
    /*!
     \param fileExt The file extension which should be tested.
     \return True if the given file extension is supported.
    */
    bool isExtensionSupported(const std::string& fileExt) const;

    //! The constructor is hidden to avoid that someone creates an instance of this class.
    ModuleManager();

    //! Disable/Hide the copy constructor
    ModuleManager(const ModuleManager&);

    //! Disable/Hide the copy assignment operator
    ModuleManager& operator=(const ModuleManager&);

    //! The ModuleManager destructor
    /*!
      Deletes the ModuleManager.
    */
    ~ModuleManager();

    static ModuleManager* m_instance; /*!< Pointer that saves the instance of this class. */

    //! Destroyer class to delete the instance of the ModuleManager class when the program terminates.
    class SingletonDestroyer {
    public: ~SingletonDestroyer() {
        if (ModuleManager::m_instance != NULL) delete ModuleManager::m_instance;
      }
    };
    friend class SingletonDestroyer;

  };

} //end of namespace Belle2

#endif /* MODULEMANAGER_H_ */
