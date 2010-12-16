/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *               R.Itoh, addition of parallel processing function         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FRAMEWORK_H_
#define FRAMEWORK_H_

#include <boost/python.hpp>
#include <boost/python/list.hpp>
#include <boost/python/dict.hpp>

#include <framework/core/Module.h>
#include <framework/core/ModuleUtils.h>
#include <framework/core/ModuleManager.h>
#include <framework/core/Path.h>
#include <framework/core/PathManager.h>
#include <framework/core/EventProcessor.h>

#include <framework/pcore/pEventProcessor.h>

#include <string>
#include <map>
#include <list>

namespace Belle2 {

  /**
   * The main Framework class.
   * This class combines all subsystems of the framework.
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
    ModulePtr registerModule(const std::string moduleName) throw(ModuleManager::ModuleNotCreatedError);

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
    ModulePtr registerModule(const std::string moduleName, const std::string sharedLibPath) throw(ModuleManager::ModuleNotCreatedError);

    /**
     * Creates a new path, adds it to the framework and returns a shared pointer.
     *
     * This method creates a new path and registers it to the framework.
     * If the path could not be created, an exception of type PathNotCreatedError is thrown.
     *
     * @return A shared pointer of the added and registered path.
     */
    PathPtr createPath() throw(PathManager::PathNotCreatedError);

    /**
     * Processes all events by starting with the first module in the specified path.
     *
     * This method starts processing events only if there is a module in the path
     * which is capable of specifying the end of the data flow.
     *
     * @param startPath The processing starts with the first module of this path.
     */
    void process(PathPtr startPath);

    /**
     * Processes up to maxEvent events by starting with the first module in the specified path.
     *
     * @param startPath The processing starts with the first module of this path.
     * @param maxEvent The maximum number of events that will be processed. If the number is smaller than 1, all events will be processed.
     */
    void process(PathPtr startPath, long maxEvent);

    /**
     * Processes up to maxEvent events using the specified run number by starting with the first module in the specified path.
     *
     * @param startPath The processing starts with the first module of this path.
     * @param maxEvent The maximum number of events that will be processed. If the number is smaller than 1, all events will be processed.
     * @param runNumber The number of the run which should be processed.
     */
    void process(PathPtr startPath, long maxEvent, long runNumber);

    /**
     * Function to set number of processes for parallel processing.
     *
     * @param nproc Number of processes for parallel processing
    */
    void set_nprocess(int nproc);

    /**
     * Function to get number of processes for parallel processing.
    */
    static int nprocess(void);

    /**
     * Reads the particle information from the evtgen table file specified by its filename.
     * @param filename The path and the file which should be read and its content stored into TDatabasePDG.
     * @return True if the filename exists and the content could be read.
    */
    bool readEvtGenTableFromFile(const std::string& filename);

    /**
     * Configure the log level.
     *
     * @param logLevel The log level.
     */
    void setLogLevel(int logLevel);

    /**
     * Configure the debug messaging level.
     *
     * @param debugLevel The debug level.
     */
    void setDebugLevel(int debugLevel);

    /**
     * Configure the abort log level.
     *
     * @param abortLevel The abort log level.
     */
    void setAbortLevel(int abortLevel);

    /**
     * Configure the printed log information for the given level.
     *
     * @param logLevel The log level.
     * @param logInfo The log information that should be printed.
     */
    void setLogInfo(int logLevel, unsigned int logInfo);

    /**
     * Sets the log level and debug level for a package.
     *
     * @param package The package name.
     * @param logLevel The log level.
     * @param debugLevel The debug level.
     */
    void setPackageLogLevel(std::string package, int logLevel, int debugLevel);

    /**
     * Adds the logging output to the shell (std::cout).
     *
     * @param color Enable color output if set to true. Default is color off.
     */
    void addLoggingToShell(bool color = false);

    /**
     * Adds the logging output to a text file.
     *
     * @param filename The path and name of the text file.
     * @param append If this value is set to true, an existing file will not be overridden,
     *               but instead the log messages will be added to the end of the file.
     */
    void addLoggingToTxtFile(const std::string& filename, bool append);

    /*! Removes all logging output. */
    void resetLogging();


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
     * Returns a summary of the log message calls.
     *
     * @return A python dictionary containing the log levels as (string) key and the number of calls per level as integer.
     */
    boost::python::dict getLogStatisticPython() const;

    /**
     * Exposes methods of the Framework class to Python.
     */
    static void exposePythonAPI();


  protected:

    PathManager* m_pathManager;         /**< The path manager, which takes care of creating and handling paths. */
    EventProcessor* m_eventProcessor;   /**< The event processor, which loops over the events and calls the modules. */
    pEventProcessor* m_peventProcessor; /**< The event processor, which loops over the events and calls the modules. */


  private:

    static int m_nproc;

  };

} //end of namespace Belle2

#endif /* FRAMEWORK_H_ */
