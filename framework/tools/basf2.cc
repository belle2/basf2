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
 * Copyright(C) 2010-2014  Belle II Collaboration                         *
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
 *                        Christian Pulvermacher                          *
 *                        Thomas Keck                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/python.hpp> //Has to be the first include (restriction due to python)

#include <framework/core/Environment.h>
#include <framework/core/DataFlowVisualization.h>
#include <framework/logging/Logger.h>
#include <framework/logging/LogConfig.h>
#include <framework/logging/LogSystem.h>
#include <framework/utilities/FileSystem.h>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp> //for iequals()

#include <unistd.h>
#include <signal.h>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include <locale>
#include <codecvt>

#ifdef HAS_CALLGRIND
#include <valgrind/valgrind.h>
#endif

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
    B2FATAL("The given filename and/or path is not valid: " + pythonFile);
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

  //Check for Belle2 environment variables (during environment initialisation)
  Environment::Instance();

  //Get the lib path (checked for NULL in Environment)
  const char* belle2SubDir = getenv("BELLE2_SUBDIR");
  boost::filesystem::path libPath = "lib";
  libPath /= belle2SubDir;

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
    ("modules,m", prog::value<string>()->implicit_value(""),
     "print a list of all available modules (can be limited to a given package), or give detailed information on a specific module given as an argument (case sensitive).")
    ;

    // Declare a group of options that will be
    // allowed both on command line and in
    // config file
    prog::options_description config("Configuration");
    config.add_options()
    ("steering", prog::value<string>(), "the python steering file")
    ("arg", prog::value<vector<string> >(&arguments), "additional arguments to be passed to the steering file")
    ("log_level,l", prog::value<string>(),
     "set global log level (one of DEBUG, INFO, RESULT, WARNING, or ERROR). Takes precedence over set_log_level() in steering file.")
    ("events,n", prog::value<int>(), "override number of events for EventInfoSetter; otherwise set maximum number of events.")
    ("input,i", prog::value<vector<string> >(),
     "override name of input file for (Seq)RootInput. Can be specified multiple times to use more than one file.")
    ("output,o", prog::value<string>(), "override name of output file for (Seq)RootOutput")
    ("processes,p", prog::value<int>(), "override number of worker processes (>=1 enables, 0 disables parallel processing)")
    ("visualize-dataflow", "Generate data flow diagram (dataflow.dot) for the executed steering file.")
    ("module-io", prog::value<string>(),
     "Create diagram of inputs and outputs for a single module, saved as ModuleName.dot. To create a PostScript file, use e.g. 'dot ModuleName.dot -Tps -o out.ps'.")
    ("no-stats",
     "Disable collection of statistics during event processing. Useful for very high-rate applications, but produces empty table with 'print statistics'.")
    ("dry-run",
     "Read steering file, but do not start any actually start any event processing. Prints information on input/output files that would be used during normal execution.")
    ("dump-path", prog::value<string>(),
     "Read steering file, but do not actually start any event processing. The module path the steering file would execute is instead pickled (serialized) into the given file.")
    ("execute-path", prog::value<string>(),
     "Do not read any provided steering file, instead execute the pickled (serialized) path from the given file.")
#ifdef HAS_CALLGRIND
    ("profile", prog::value<string>(),
     "Name of a module to profile using callgrind. If more than one module of that name is registered only the first one will be profiled.")
#endif
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
      cout << "Usage: " << argv[0] << " [OPTIONS] [STEERING_FILE] [-- [STEERING_FILE_OPTIONS]]\n";
      cout << cmdlineOptions << endl;
      return 0;
    }

    // -p
    // Do now so that we can override if profiling is requested
    if (varMap.count("processes")) {
      int nprocesses = varMap["processes"].as<int>();
      if (nprocesses < 0) {
        B2FATAL("Invalid number of processes!");
      }
      Environment::Instance().setNumberProcessesOverride(nprocesses);
    }

#ifdef HAS_CALLGRIND
    if (varMap.count("profile")) {
      string profileModule = varMap["profile"].as<string>();
      //We want to profile a module so check if we are running under valgrind
      if (!RUNNING_ON_VALGRIND) {
        //Apparently not. Ok,  let's call ourself using valgrind
        cout << "Profiling requested, restarting using callgrind" << endl;

        //Sadly calling processes in C++ is very annoying as we have to
        //build a command line.
        vector<char*> cmd;
        //First we add all valgrind arguments.
        const vector<string> valgrind_argv {
          "valgrind", "--tool=callgrind", "--instr-atstart=no", "--trace-children=no",
          "--callgrind-out-file=callgrind." + profileModule + ".%p",
        };
        //As execvp wants non-const char* pointers we have to copy the string contents.
        for (auto argv : valgrind_argv) { cmd.push_back(strdup(argv.c_str())); }
        //And now we add our own arguments, including the program name.
        for (int i = 0; i < argc; ++i)  { cmd.push_back(argv[i]); }
        //Finally, execvp wants a nullptr as last argument
        cmd.push_back(nullptr);
        //And call this thing. Execvp will not return if successful as the
        //current process will be replaced so we do not need to care about what
        //happens if succesful
        if (execvp(cmd[0], cmd.data()) == -1) {
          int errsv = errno;
          perror("Problem calling valgrind");
          return errsv;
        }
      }
      //Ok, running under valgrind, set module name we want to profile in
      //environment.
      Environment::Instance().setProfileModuleName(profileModule);
      //and make sure there is no multiprocessing when profiling
      Environment::Instance().setNumberProcessesOverride(0);
    }
#endif

    //Check for version option
    if (varMap.count("version")) {
      pythonFile = "version.py";
    }

    //Check for modules option (-m)
    if (varMap.count("modules")) {
      string modArgs = varMap["modules"].as<string>();
      if (!modArgs.empty()) {
        arguments.insert(arguments.begin(), modArgs);
      }
      // recent boost program_options will not consume extra tokens for
      // implicit options. In this case the module/package name gets consumed
      // in tyhe steering file so we just use that.
      if (varMap.count("steering")) {
        arguments.insert(arguments.begin(), varMap["steering"].as<string>());
      }
      pythonFile = "modules.py";
    } else if (varMap.count("steering")) {
      // steering file not misused as module name, so print it's name :D
      pythonFile = varMap["steering"].as<string>();
      B2INFO("Steering file: " << pythonFile);
    }

    // -n
    if (varMap.count("events")) {
      int nevents = varMap["events"].as<int>();
      if (nevents <= 0) {
        B2FATAL("Invalid number of events!");
      }
      Environment::Instance().setNumberEventsOverride(nevents);
    }

    // -i
    if (varMap.count("input")) {
      const vector<string>& names = varMap["input"].as<vector<string> >();
      Environment::Instance().setInputFilesOverride(names);
    }

    // -o
    if (varMap.count("output")) {
      std::string name = varMap["output"].as<string>();
      Environment::Instance().setOutputFileOverride(name);
    }

    // -l
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
        B2FATAL("Invalid log level! Needs to be one of DEBUG, INFO, RESULT, WARNING, or ERROR.");
      }

      //set log level
      LogSystem::Instance().getLogConfig()->setLogLevel((LogConfig::ELogLevel)level);
      //and make sure it takes precedence overy anything in the steeering file
      Environment::Instance().setLogLevelOverride(level);
    }

    if (varMap.count("visualize-dataflow")) {
      Environment::Instance().setVisualizeDataFlow(true);
      if (Environment::Instance().getNumberProcesses() > 0) {
        B2WARNING("--visualize-dataflow cannot be used with parallel processing, no graphs will be saved!");
      }
    }

    if (varMap.count("module-io")) {
      runModuleIOVisualization = varMap["module-io"].as<string>();
      pythonFile = "basf2.py"; //make module maps available, visualization will happen later
    }

    if (varMap.count("no-stats")) {
      Environment::Instance().setNoStats(true);
    }

    if (varMap.count("dry-run")) {
      Environment::Instance().setDryRun(true);
    }

    //Check for version option
    if (varMap.count("execute-path")) {
      Environment::Instance().setPicklePath(varMap["execute-path"].as<string>());
      pythonFile = "execute_pickled_path.py";
    }

    //Check for version option
    if (varMap.count("dump-path")) {
      Environment::Instance().setPicklePath(varMap["dump-path"].as<string>());
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

    //Search in local or central lib/ if this isn't a direct path
    if (!boost::filesystem::exists(pythonFile)) {
      std::string libFile = FileSystem::findFile((libPath / pythonFile).string());
      if (!libFile.empty())
        pythonFile = libFile;
    }

    try {
      //Init Python interpreter
      Py_InitializeEx(0);

      std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
      std::vector<wstring> pyArgvString(arguments.size() + 1);
      pyArgvString[0] = converter.from_bytes(pythonFile);
      for (size_t i = 0; i < arguments.size(); i++) {
        pyArgvString[i + 1] = converter.from_bytes(arguments[i]);
      }
      std::vector<const wchar_t*> pyArgvArray(pyArgvString.size());
      for (size_t i = 0; i < pyArgvString.size(); ++i) {
        pyArgvArray[i] = pyArgvString[i].c_str();
      }
      //Pass python filename and additional arguments to python
      PySys_SetArgv(pyArgvArray.size(), const_cast<wchar_t**>(pyArgvArray.data()));

      //Execute Python file
      executePythonFile(pythonFile);

      //Finish Python interpreter
      Py_Finalize();

      //basf2.py was loaded, now do module I/O visualization
      if (!runModuleIOVisualization.empty()) {
        DataFlowVisualization::executeModuleAndCreateIOPlot(runModuleIOVisualization);
      }

      //--dry-run: print gathered information
      if (Environment::Instance().getDryRun()) {
        Environment::Instance().printJobInformation();
      }
    } catch (error_already_set) {
      //Apparently an exception occured which wasn't handled. So print the traceback
      PyErr_Print();
      //And in rare cases, i.e. when redirecting output, the buffers are not
      //flushed unless we finalize python. So do it now
      Py_Finalize();
      return 1;
    }

  }

  //---------------------------------------------------
  //  If no command line parameter was given, run the
  //  Python interpreter in interactive mode.
  //---------------------------------------------------
  if (runInteractiveMode) {
    pythonFile = FileSystem::findFile((libPath / "basf2.py").string());
    string extCommand("python3 -i " + pythonFile);
    if (system(extCommand.c_str()) != 0)
      return 1;
  }

  return 0;
}

