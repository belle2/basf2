/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2012  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr                                *
 *               R.Itoh, addition of parallel processing function         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <boost/python/object.hpp>

#include <boost/python/list.hpp>
#include <boost/python/dict.hpp>

#include <framework/core/Module.h>
#include <framework/core/Path.h>

#include <string>

namespace Belle2 {
  /**
   * This class combines all subsystems of the framework, and exports the main interface to Python.
   */
  class Framework {
  public:
    /**
     * Constructor.
     */
    Framework();

    /**
     * Destructor.
     */
    virtual ~Framework();

    /**
     * Adds a new filepath to the list of filepaths which are searched for modules.
     *
     * If a module is requested by the method registerModule(), all module search paths
     * are scanned for shared libraries or map files which contain the requested module.
     *
     * @param path The module search path which should be added to the list of paths.
    */
    void addModuleSearchPath(const std::string& path);

    /**
     * Sets the path in which the externals of the framework are located.
     *
     * @param path The path in which the externals of the framework are located.
    */
    void setExternalsPath(const std::string& path);

    /**
     * Registers a new module to the framework and returns a shared pointer.
     *
     * This method creates a new Module based on the given module name. If the shared library file
     * containing the module was already loaded, the module is directly created. Otherwise the
     * shared library file is dynamically loaded first.
     *
     * If the module could not be registered, an exception of type ModuleNotCreatedError is thrown.
     *
     * @param moduleName The unique name of the module which should be created.
     * @return A shared pointer of the newly created and registered module.
     */
    ModulePtr registerModule(const std::string& moduleName);

    /**
     * Registers a new module to the framework and returns a shared pointer.
     *
     * This method creates a new Module based on the given module name. If the shared library file
     * containing the module was already loaded, the module is directly created. Otherwise the
     * shared library file is dynamically loaded first.
     *
     * If the module could not be registered, an exception of type ModuleNotCreatedError is thrown.
     *
     * @param moduleName The unique name of the module which should be created.
     * @param sharedLibPath Optional: The shared library from which the module should be registered (not a map file !).
     * @return A shared pointer of the newly created and registered module.
     */
    ModulePtr registerModule(const std::string& moduleName, const std::string& sharedLibPath);

    /**
     * Processes up to maxEvent events by starting with the first module in the specified path.
     *
     * This method starts processing events only if there is a module in the path
     * which is capable of specifying the end of the data flow.
     *
     * @param startPath The processing starts with the first module of this path.
     * @param maxEvent The maximum number of events that will be processed. If the number is smaller than 1, all events will be processed (default).
     */
    void process(PathPtr startPath, long maxEvent = 0);

    /**
     * Function to set number of worker processes for parallel processing.
    */
    void setNumberProcesses(int numProcesses);

    /**
     * Function to get number of worker processes for parallel processing.
    */
    int getNumberProcesses() const;

    /**
     * Function to set the path to the file where the pickled path is stored
     *
     * @param path path to file where the pickled path is stored
    */
    void setPicklePath(std::string path);

    /**
     * Function to get the path to the file where the pickled path is stored
     *
     * @return path to file where the pickled path is stored
    */
    std::string getPicklePath() const;

    /**
     * Function to set streaming objects for Tx module
     *
     * @param streamingObjects objects to be streamed
    */
    void setStreamingObjects(boost::python::list streamingObjects);


    //--------------------------------------------------
    //                   Python API
    //--------------------------------------------------

    /**
     * Returns a list of all module search paths known to the framework.
     *
     * @return A python list containing all module search paths known to the framework.
     */
    boost::python::list getModuleSearchPathsPython() const;

    /**
     * Returns a dictionary containing the found modules and the filenames of the shared libraries in which they can be found.
     *
     * @return A python dictionary dictionary containing the found modules.
     */
    boost::python::dict getAvailableModulesPython() const;

    /**
     * Returns a list of all registered modules.
     *
     * @return A python list containing all registered modules.
     */
    boost::python::list getRegisteredModulesPython() const;

    /**
     * Exposes methods of the Framework class to Python.
     */
    static void exposePythonAPI();


  protected:

  };

} //end of namespace Belle2
