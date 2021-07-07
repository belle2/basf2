/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

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
    static void addModuleSearchPath(const std::string& path);

    /**
     * Sets the path in which the externals of the framework are located.
     *
     * @param path The path in which the externals of the framework are located.
    */
    static void setExternalsPath(const std::string& path);

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
    static ModulePtr registerModule(const std::string& moduleName);

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
    static ModulePtr registerModule(const std::string& moduleName, const std::string& sharedLibPath);

    /**
     * Processes up to maxEvent events by starting with the first module in the specified path.
     *
     * This method starts processing events only if there is a module in the path
     * which is capable of specifying the end of the data flow.
     *
     * @param startPath The processing starts with the first module of this path.
     * @param maxEvent The maximum number of events that will be processed. If the number is smaller than 1, all events will be processed (default).
     */
    static void process(PathPtr startPath, long maxEvent = 0);

    /**
     * Function to set number of worker processes for parallel processing.
    */
    static void setNumberProcesses(int numProcesses);

    /**
     * Function to get number of worker processes for parallel processing.
    */
    static int getNumberProcesses();

    /**
     * Function to set the path to the file where the pickled path is stored
     *
     * @param path path to file where the pickled path is stored
    */
    static void setPicklePath(const std::string& path);

    /**
     * Function to get the path to the file where the pickled path is stored
     *
     * @return path to file where the pickled path is stored
    */
    static std::string getPicklePath();

    /**
     * Function to set streaming objects for Tx module
     *
     * @param streamingObjects objects to be streamed
    */
    static void setStreamingObjects(const boost::python::list& streamingObjects);

    /**
     * Function to set the execution realm
     *
     * @param realm basf2 execution realm
    */
    static void setRealm(const std::string& realm);


    /** Find a file. This is a wrapper around FileSystem::findFile() to be able
     * to call it nicely from python and create a `FileNotFoundError` if the
     * file cannot be found.
     *
     * Known types:
     *
     * - empty string: exactly like `FileSystem::findFile`, look in $BELLE2_LOCAL_DIR
     *   and then $BELLE2_RELEASE_DIR and then relative to local dir
     * - 'examples': look for example data in $BELLE2_EXAMPLES_DATA_DIR,
     *   then relative to the local directory
     * - 'validation': look for validation data in $BELLE2_VALIDATION_DATA_DIR,
     *   then relative to the local directory
     *
     * @param filename relative filename to look for, either in a central place
     *        or in the current working directory
     * @param type if set, specifies where to look if the file cannot be found
     *        locally
     * @param ignore_errors if true don't print any errors and silently return
     *        None. Otherwise this function will raise a FileNotFoundError
     *        exception if the file cannot be found.
     * @return relative or absolute filename if found, empty string if not
     *        found and ignore_errors is true
     */
    static std::string findFile(const std::string& filename, const std::string& type, bool ignore_errors = false);

    //--------------------------------------------------
    //                   Python API
    //--------------------------------------------------

    /**
     * Returns a list of all module search paths known to the framework.
     *
     * @return A python list containing all module search paths known to the framework.
     */
    static boost::python::list getModuleSearchPathsPython();

    /**
     * Returns a dictionary containing the found modules and the filenames of the shared libraries in which they can be found.
     *
     * @return A python dictionary dictionary containing the found modules.
     */
    static boost::python::dict getAvailableModulesPython();

    /**
     * Returns a list of all registered modules.
     *
     * @return A python list containing all registered modules.
     */
    static boost::python::list getRegisteredModulesPython();

    /**
     * Exposes methods of the Framework class to Python.
     */
    static void exposePythonAPI();
  };

} //end of namespace Belle2
