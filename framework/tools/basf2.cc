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
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
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
#include <framework/core/Environment.h>
#include <framework/logging/Logger.h>
#include <framework/logging/LogConfig.h>
#include <framework/logging/LogSystem.h>
#include <framework/core/DataFlowVisualization.h>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <signal.h>
#include <cstdlib>
#include <sys/utsname.h>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include <streambuf>

using namespace std;
using namespace Belle2;
using namespace boost::python;

namespace prog = boost::program_options;

void executePythonFile(const string& pythonFile);

void executePythonFile(const string& pythonFile)
{
  boost::filesystem::path fullPath(boost::filesystem::initial_path<boost::filesystem::path>());

  fullPath = boost::filesystem::system_complete(boost::filesystem::path(pythonFile));
  if ((!(boost::filesystem::is_directory(fullPath))) && (boost::filesystem::exists(fullPath))) {

    std::ifstream file(fullPath.string().c_str());
    std::stringstream buffer;
    buffer << file.rdbuf();
    Environment::Instance().setSteering(buffer.str());

    object main_module = import("__main__");
    object main_namespace = main_module.attr("__dict__");

    exec_file(boost::python::str(fullPath.string()), main_namespace, main_namespace);
  } else {
    B2ERROR("The given filename and/or path is not valid: " + pythonFile);
  }
}


int main(int argc, char* argv[])
{
  //remove SIGPIPE handler set by ROOT which sometimes caused infinite loops
  //See https://savannah.cern.ch/bugs/?97991
  //default action is to abort
  if (signal(SIGPIPE, SIG_DFL) == SIG_ERR) {
    B2FATAL("Cannot remove SIGPIPE signal handler");
  }

  //Check for the Belle2 environment variable
  char* belle2LocalDir = getenv("BELLE2_LOCAL_DIR");
  char* belle2ReleaseDir = getenv("BELLE2_RELEASE_DIR");
  if (!belle2LocalDir && !belle2ReleaseDir) {
    B2ERROR("The basf2 environment is not set up. Please execute the 'setuprel' script first.")
    return 1;
  }

  char* belle2SubDir = getenv("BELLE2_SUBDIR");
  if (!belle2SubDir) {
    B2ERROR("The environment variable BELLE2_SUBDIR is not set. Please execute the 'setuprel' script first.")
    return 1;
  }

  //Get the lib path
  boost::filesystem::path libPath = "";
  boost::filesystem::path libPath2 = "";
  if (belle2LocalDir) {
    libPath = belle2LocalDir;
    if (belle2ReleaseDir) {
      libPath2 = belle2ReleaseDir;
      libPath2 /= "lib";
      libPath2 /=  belle2SubDir;
    }
  } else {
    libPath = belle2ReleaseDir;
  }
  libPath /= "lib";
  libPath /=  belle2SubDir;

  bool runInteractiveMode = true;
  string runModuleIOVisualization(""); //nothing done if empty
  vector<string> arguments;
  string pythonFile;

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
    ("info", "print information about basf2")
    ("modules,m", prog::value<string>()->implicit_value(""), "print a list of all available modules; or give detailed information on a specific module given as an argument (case sensitive)")
    ;

    // Declare a group of options that will be
    // allowed both on command line and in
    // config file
    prog::options_description config("Configuration");
    config.add_options()
    ("steering", prog::value<string>(), "the python steering file")
    ("arg", prog::value<vector<string> >(&arguments), "additional arguments to be passed to the steering file")
    ("log_level,l", prog::value<string>(), "set log level (one of DEBUG, INFO, WARNING, or ERROR)")
    ("events,n", prog::value<int>(), "override number of events in first run for EvtMetaGen; otherwise set maximum number of events ")
    ("input,i", prog::value<string>(), "override name of input file for (Seq)RootInput")
    ("output,o", prog::value<string>(), "override name of output file for (Seq)RootOutput")
    ("processes,p", prog::value<int>(), "override number of parallel processes (0 to disable parallel processing)")
    ("visualize-dataflow", "Generate data flow diagram (dataflow.dot) for the executed steering file.")
    ("module-io", prog::value<string>(), "Create diagram of inputs and outputs for a single module, saved as ModuleName.dot. To create a PostScript file, use e.g. 'dot ModuleName.dot -Tps -o out.ps'.")
    ;

    prog::options_description cmdlineOptions;
    cmdlineOptions.add(generic).add(config);

    prog::positional_options_description posOptDesc;
    posOptDesc.add("steering", 1);
    posOptDesc.add("arg", -1);

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
      B2INFO("Steering file: " << pythonFile);
    }

    //Check for version option
    if (varMap.count("version")) {
      pythonFile = "version.py";
    }

    //Check for modules option
    if (varMap.count("modules")) {
      string modArgs = varMap["modules"].as<string>();
      if (!modArgs.empty()) arguments.push_back(modArgs);
      pythonFile = "modules.py";
    }

    if (varMap.count("events")) {
      int nevents = varMap["events"].as<int>();
      if (nevents <= 0) {
        B2FATAL("Invalid number of events!");
        return 1;
      }
      Environment::Instance().setNumberEventsOverride(nevents);
    }

    if (varMap.count("input")) {
      std::string name = varMap["input"].as<string>();
      Environment::Instance().setInputFileOverride(name);
    }

    if (varMap.count("output")) {
      std::string name = varMap["output"].as<string>();
      Environment::Instance().setOutputFileOverride(name);
    }

    if (varMap.count("processes")) {
      int nprocesses = varMap["processes"].as<int>();
      if (nprocesses < 0) {
        B2FATAL("Invalid number of processes!");
        return 1;
      }
      Environment::Instance().setNumberProcessesOverride(nprocesses);
    }

    if (varMap.count("log_level")) {
      std::string levelParam = varMap["log_level"].as<string>();
      int level = -1;
      for (int i = LogConfig::c_Debug; i < LogConfig::c_Fatal; i++) {
        std::string thisLevel = LogConfig::logLevelToString((LogConfig::ELogLevel)i);
        if (boost::iequals(levelParam, thisLevel)) { //case-insensitive
          level = i;
          break;
        }
      }
      if (level < 0) {
        B2FATAL("Invalid log level! Needs to be one of DEBUG, INFO, WARNING, or ERROR.");
        return 1;
      }

      LogSystem::Instance().getLogConfig()->setLogLevel((LogConfig::ELogLevel)level);
    }

    if (varMap.count("visualize-dataflow")) {
      Environment::Instance().setVisualizeDataFlow(true);
    }

    if (varMap.count("module-io")) {
      runModuleIOVisualization = varMap["module-io"].as<string>();
      pythonFile = "basf2.py"; //make module maps available, visualization will happen later
    }

    //Check for info option
    if (varMap.count("info")) {
      pythonFile = "info.py";
    }

  } catch (exception& e) {
    cerr << "error: " << e.what() << endl;
    return 1;
  } catch (...) {
    cerr << "Exception of unknown type!" << endl;
    return 1;
  }

  //---------------------------------------------------
  //  If the python file is set, execute it
  //---------------------------------------------------
  if (!pythonFile.empty()) {
    runInteractiveMode = false;

    //Get the lib path
    if (!boost::filesystem::exists(pythonFile)) {
      if (boost::filesystem::exists(libPath / pythonFile)) {
        pythonFile = (libPath / pythonFile).string();
      } else {
        pythonFile = (libPath2 / pythonFile).string();
      }
    }

    try {
      //Init Python interpreter
      Py_InitializeEx(0);

      //Pass python filename and additional arguments to python
      const char* pyargs[arguments.size() + 1];
      pyargs[0] = pythonFile.c_str();
      for (size_t i = 0; i < arguments.size(); i++) {
        pyargs[i + 1] = arguments[i].c_str();
      }
      PySys_SetArgv(arguments.size() + 1, const_cast<char**>(pyargs));

      //Embedd Python modules
      PyBasf2::embedPythonModule();

      //Execute Python file
      executePythonFile(pythonFile);

      //Finish Python interpreter
      Py_Finalize();

      //basf2.py was loaded, now do module I/O visualization
      if (!runModuleIOVisualization.empty()) {
        DataFlowVisualization::executeModuleAndCreateIOPlot(runModuleIOVisualization);
      }
    } catch (PythonModuleNotEmbeddedError& exc) {
      B2ERROR(exc.what());
    } catch (error_already_set) {
      PyErr_Print();
      return 1;
    }

  }

  //---------------------------------------------------
  //  If no command line parameter was given, run the
  //  Python interpreter in interactive mode.
  //---------------------------------------------------
  if (runInteractiveMode) {
    pythonFile = (libPath / "basf2.py").string();
    if (!boost::filesystem::exists(pythonFile)) {
      pythonFile = (libPath2 / "basf2.py").string();
    }
    string extCommand("python -i " + pythonFile);
    system(extCommand.c_str());
  }

  return 0;
}

