/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MODULECHAIN_H_
#define MODULECHAIN_H_

#include <framework/fwcore/Module.h>
#include <framework/fwcore/ModuleList.h>
#include <framework/fwcore/Path.h>
#include <framework/fwcore/PathList.h>
#include <framework/fwcore/FwExceptions.h>

#include <string>
#include <list>
#include <vector>
#include <set>

namespace Belle2 {

  //! The ModuleChain Class
  /*!
    This class provides a common interface for the creation of
    modules and paths.
  */
  class ModuleChain {

  public:

    //! Constructor
    ModuleChain();

    //! Destructor
    virtual ~ModuleChain();

    //! Registers a new module to the system and returns a shared pointer.
    /*!
        This method creates a new Module based on the given type identifier string
        and registers it to the system.

        If the module could not be registered, an exception of type FwExcModuleNotRegistered is thrown.

        \param type The unique type identifier of the module which should be created.
        \return A shared pointer of the newly created and registered module.
    */
    ModulePtr registerModule(const std::string& type) throw(FwExcModuleNotRegistered);

    //! Adds a new path to the system and returns a reference.
    /*!
        This method adds a new path and registers it to the system.

        If the path could not be added, an exception of type FwExcPathNotAdded is thrown.

        \return A shared pointer of the added and registered path.
    */
    PathPtr addPath() throw(FwExcPathNotAdded);

    //! Returns a list of all modules which could be executed during the data processing.
    /*!
        The method starts with the given path, iterates over the modules in the path and
        follows recursively module conditions to make sure the final list contains all
        modules which could be executed while preserving their correct order.
        Special care is taken to avoid that a module is added more than once to the list.

        \param startPath The path with which the search should start.
        \return A list containing all modules which could be executed during the data processing.
    */
    ModulePtrList getModulePathList(PathPtr startPath) const;

    //! Returns a list of those modules which carry property flags matching the specified ones.
    /*!
         Loops over all module instances specified in a list
         and adds those to the returned list whose property flags match the given property flags.

         \param modulePathList The list containing all module instances added to a path.
         \param propertyFlags The flags for the module properties.
         \return A list containing those modules which carry property flags matching the specified ones.
    */
    ModulePtrList getModulesByProperties(const ModulePtrList& modulePathList, unsigned int propertyFlags) const;

    //! Initializes the modules.
    /*!
         Loops over all module instances specified in a list and calls their initialize() method.

         \param modulePathList A list of all modules which could be executed during the data processing.
    */
    void processInitialize(const ModulePtrList& modulePathList);

    //! Processes the full module chain, starting with the first module in the given path.
    /*!
        \param startPath The processing starts with the first module of this path.
        \param modulePathList A list of all modules which could be executed during the data processing (used for calling the beginRun() and endRun() method).
        \param maxEvent If this number is greater or equal than 0, only this number of events will be processed.
    */
    void processChain(PathPtr startPath, const ModulePtrList& modulePathList, long maxEvent = -1);

    //! Terminates the modules.
    /*!
         Loops over all module instances in reverse order specified in a list and calls their terminate() method.

         \param modulePathList A list of all modules which could be executed during the data processing.
    */
    void processTerminate(const ModulePtrList& modulePathList);

    //! Calls the begin run methods of all modules.
    /*!
         Loops over all module instances specified in a list
         and calls their beginRun() method. Please note: the
         beginRun() method of the module which triggered
         the beginRun() loop will also be called.

         \param modulePathList The list containing all module instances added to a path.
    */
    void processBeginRun(const ModulePtrList& modulePathList);

    //! Calls the end run methods of all modules.
    /*!
         Loops over all module instances specified in a list
         and calls their endRun() method. Please note: the
         endRun() method of the module which triggered
         the endRun() loop will also be called.

         \param modulePathList The list containing all module instances added to a path.
    */
    void processEndRun(const ModulePtrList& modulePathList);


  protected:


  private:

    ModuleList* m_moduleList; /*!< Instance of the list which manages the created modules. */
    PathList*   m_pathList;   /*!< Instance of the list which manages the created paths. */

  };

} // end namespace Belle2

#endif /* MODULECHAIN_H_ */
