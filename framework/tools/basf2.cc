/**************************************************************************
 *                             BASF2                                      *
 *                                                                        *
 *                  The Belle Analysis Framework 2                        *
 *                                                                        *
 *                                                                        *
 * There are two ways to work with the framework. Either                  *
 * by executing "basf2" and providing a python steering                   *
 * file as an argument or by using the framework within                   *
 * python itself.                                                         *
 *                                                                        *
 * This file implements the main executable "basf2".                      *
 *                                                                        *
 *                                                                        *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Contributing authors :                                                 *
 * (main framework)                                                       *
 *                        Andreas Moll                                    *
 *                        Martin Heck                                     *
 *                        Nobu Katayama                                   *
 *                        Ryosuke Itoh                                    *
 *                        Thomas Kuhr                                     *
 *                        Kolja Prothmann                                 *
 *                        Martin Ritter                                   *
 *                        Zbynek Drasal                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/python.hpp> //Has to be the first include (restriction due to python)

#include <framework/pybasf2/PyBasf2.h>
#include <framework/core/Framework.h>
#include <framework/logging/Logger.h>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <cstdlib>
#include <sys/utsname.h>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <string>
#include <vector>

using namespace std;
using namespace Belle2;
using namespace boost::python;

namespace prog = boost::program_options;


void executePythonFile(const string& pythonFile)
{
  boost::filesystem::path fullPath(boost::filesystem::initial_path<boost::filesystem::path>());

  try {
    fullPath = boost::filesystem::system_complete(boost::filesystem::path(pythonFile));
    if ((!(boost::filesystem::is_directory(fullPath))) && (boost::filesystem::exists(fullPath))) {
      try {
        object main_module = import("__main__");
        object main_namespace = main_module.attr("__dict__");

        exec_file(boost::python::str(fullPath.string()), main_namespace, main_namespace);
      } catch (error_already_set const &) {
        ERROR("An error occurred during the execution of the python file: " + pythonFile);
      }
    } else {
      ERROR("The given filename and/or path is not valid: " + pythonFile);
    }
  } catch (...) {
    ERROR("An error occurred during the execution of the python file: " + pythonFile);
  }
}


int main(int argc, char* argv[])
{
  //Variable declarations
  string pythonFile;

  //Check for the Belle2 environment variable
  char* belle2LocalDir = getenv("BELLE2_LOCAL_DIR");
  if (belle2LocalDir == NULL) {
    ERROR("The environment variable BELLE2_LOCAL_DIR is not set. Please execute the 'setuprel' script first.")
    return 1;
  }

  char* belle2SubDir = getenv("BELLE2_SUBDIR");
  if (belle2LocalDir == NULL) {
    ERROR("The environment variable BELLE2_SUBDIR is not set. Please execute the 'setuprel' script first.")
    return 1;
  }

  //Get the lib path
  boost::filesystem::path libPath(belle2LocalDir);
  libPath /= "lib";
  libPath /=  belle2SubDir;

  bool runInteractiveMode = true;

  try {
    //---------------------------------------------------
    //          Handle command line options
    //---------------------------------------------------

    // Declare a group of options that will be
    // allowed only on command line
    prog::options_description generic("Generic options");
    generic.add_options()
    ("help,h", "print all available options")
    ("version,v", "print version string")
    ("info,i", "print information about basf2")
    ("modules,m", "print a list of all available modules")
    ;

    // Declare a group of options that will be
    // allowed both on command line and in
    // config file
    prog::options_description config("Configuration");
    config.add_options()
    ("steering", prog::value<string>(), "the python steering file")
    ;

    prog::options_description cmdlineOptions;
    cmdlineOptions.add(generic).add(config);

    prog::positional_options_description posOptDesc;
    posOptDesc.add("steering", 1);

    prog::variables_map varMap;
    prog::store(prog::command_line_parser(argc, argv).
                options(cmdlineOptions).positional(posOptDesc).run(), varMap);
    prog::notify(varMap);

    //Check for help option
    if (varMap.count("help")) {
      cout << cmdlineOptions << endl;
      return 1;
    }

    //Check for steering option
    if (varMap.count("steering")) {
      pythonFile = varMap["steering"].as<string>();
      cout << "Steering file: " << pythonFile << endl;
      runInteractiveMode = false;
    }

    //Check for version option
    if (varMap.count("version")) {
      pythonFile = (libPath / "version.py").string();
      runInteractiveMode = false;
    }

    //Check for modules option
    if (varMap.count("modules")) {
      pythonFile = (libPath / "modules.py").string();
      runInteractiveMode = false;
    }

    //Check for info option
    if (varMap.count("info")) {
      pythonFile = (libPath / "info.py").string();
      runInteractiveMode = false;
    }

  } catch (exception& e) {
    cerr << "error: " << e.what() << endl;
    return 1;
  } catch (...) {
    cerr << "Exception of unknown type!" << endl;
    runInteractiveMode = false;
  }

  //---------------------------------------------------
  //  If the python file is set, execute it
  //---------------------------------------------------
  if (!pythonFile.empty()) {
    runInteractiveMode = false;

    try {
      //Init Python interpreter
      Py_Initialize();

      //Embedd Python modules
      PyBasf2::embedPythonModule();

      //Execute Python file
      executePythonFile(pythonFile);

      //Finish Python interpreter
      Py_Finalize();
    } catch (PythonModuleNotEmbeddedError& exc) {
      ERROR(exc.what());
    } catch (error_already_set) {
      PyErr_Print();
    }

  }

  //---------------------------------------------------
  //  If no command line parameter was given, run the
  //  Python interpreter in interactive mode.
  //---------------------------------------------------
  if (runInteractiveMode) {
    string extCommand("python -i " + (libPath / "basf2.py").string());
    system(extCommand.c_str());
  }

  return 0;
}

