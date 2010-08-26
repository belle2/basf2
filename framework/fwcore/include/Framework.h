/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FRAMEWORK_H_
#define FRAMEWORK_H_

#include <boost/python.hpp>
#include <boost/python/list.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/exception_translator.hpp>

#include <framework/fwcore/Path.h>
#include <framework/fwcore/Module.h>
#include <framework/fwcore/ModuleChain.h>
#include <framework/fwcore/FwExceptions.h>

#include <string>
#include <list>

namespace Belle2 {

  //! The main Framework class
  /*! This class combines all subsystems of the framework.
  */
  class Framework {

  public:

    //! Constructor
    Framework();

    //! Destructor
    virtual ~Framework();

    //! Adds a new file extension to the list of supported file extension for module libraries.
    /*!
     The framework searches for module library files having specific file extensions. This
     method allows to add additional supported file extensions.

     \param fileExt The new file extension.
    */
    void addLibFileExtension(const std::string& fileExt);

    //! Searches for module libraries in the given path and dynamically loads them.
    /*!
     \param path The path to the folder or directly to the shared file, which contains module libraries or modules.
    */
    void loadModuleLibs(const std::string& path);

    //! Registers a new module to the framework and returns a shared pointer.
    /*!
        This method creates a new Module based on the given type identifier string
        and registers it to the framework.

        If the module could not be registered, an exception of type FwExcModuleNotRegistered is thrown.

        \param type The unique type identifier of the module which should be created.
        \return A shared pointer of the newly created and registered module.
    */
    ModulePtr registerModule(const std::string& type) throw(FwExcModuleNotRegistered);

    //! Creates a new path and adds it to the framework and returns a shared pointer.
    /*!
        This method adds a new path and registers it to the framework.

        If the path could not be created, an exception of type FwExcPathNotCreated is thrown.

        \return A shared pointer of the added and registered path.
    */
    PathPtr createPath() throw(FwExcPathNotCreated);

    //! Sets the logging output to the shell (std::cout).
    void setLoggingToShell();

    //! Sets the logging output to a text file.
    /*!
      \param filename The path and name of the text file.
      \param append If this value is set to true, an existing file will not be overridden,
                    but instead the log messages will be added to the end of the file.
    */
    void setLoggingToTxtFile(const std::string& filename, bool append);

    //! Processes the full module chain, starting with the first module in the given path.
    /*!
        This method starts processing events only if there is a module in the path
        which is capable of specifying the end of the data flow.

        \param startPath The processing starts with the first module of this path.
    */
    void process(PathPtr startPath);

    //! Processes the full module chain, starting with the first module in the given path.
    /*!
        \param startPath The processing starts with the first module of this path.
        \param maxEvent The maximum number of events that will be processed. If the number is smaller than 1, all events will be processed.
    */
    void process(PathPtr startPath, long maxEvent);

    //! Processes the full module chain, starting with the first module in the given path.
    /*!
        Processes the full module chain for the given run number and for events from 0 to maxEvent.

        \param startPath The processing starts with the first module of this path.
        \param maxEvent The maximum number of events that will be processed. If the number is smaller than 1, all events will be processed.
        \param runNumber The number of the run which should be processed.
    */
    void process(PathPtr startPath, long maxEvent, unsigned long runNumber);


    //--------------------------------------------------
    //                   Python API
    //--------------------------------------------------

    //! Returns a python list consisting of all available module types.
    /*!
        \return A python list consisting of all available module types.
    */
    boost::python::list getAvailModulesPython() const;

    //! Returns a summary of the log message calls.
    /*!
        \return A python dictionary containing the log levels as (string) key and the number of calls per level as integer.
    */
    boost::python::dict getLogStatisticPython() const;

    //! Exposes methods of the Framework class to Python.
    static void exposePythonAPI();


  protected:

    ModuleChain* m_moduleChain; /*!< The module chain, which manages the created modules and paths. */

  private:

  };

} //end of namespace Belle2

#endif /* FRAMEWORK_H_ */
