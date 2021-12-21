/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/*
 * There are two ways to work with the framework. Either
 * by executing "basf2" and providing a python steering
 * file as an argument or by using the framework within
 * python itself.
 *
 * This file implements the main executable "basf2".
 */

#include <boost/python.hpp> //Has to be the first include (restriction due to python)

#include <framework/core/Environment.h>
#include <framework/core/DataFlowVisualization.h>
#include <framework/core/RandomNumbers.h>
#include <framework/logging/Logger.h>
#include <framework/logging/LogConfig.h>
#include <framework/logging/LogSystem.h>
#include <framework/utilities/FileSystem.h>
#include <framework/core/MetadataService.h>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp> //for iequals()

#include <csignal>
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

namespace {
  void executePythonFile(const string& pythonFile)
  {
    // temporarily disable users' rootlogon
    // FIXME: remove this line when ROOT-10468 is resolved
    import("ROOT").attr("PyConfig").attr("DisableRootLogon") =  true;

    object main_module = import("__main__");
    object main_namespace = main_module.attr("__dict__");
    if (pythonFile.empty()) {
      // No steering file given, start an interactive ipython session
      object interactive = import("interactive");
      main_namespace["__b2shell_config"] = interactive.attr("basf2_shell_config")();
      exec("import IPython; "
           " from basf2 import *; "
           "IPython.embed(config=__b2shell_config, header=f\"Welcome to {basf2label}\"); ",
           main_namespace, main_namespace);
      return;
    }
    // otherwise execute the steering file
    auto fullPath = boost::filesystem::system_complete(boost::filesystem::path(pythonFile));
    if ((!(boost::filesystem::is_directory(fullPath))) && (boost::filesystem::exists(fullPath))) {

      std::ifstream file(fullPath.string().c_str());
      std::stringstream buffer;
      buffer << file.rdbuf();
      Environment::Instance().setSteering(buffer.str());
      exec_file(boost::python::str(fullPath.string()), main_namespace, main_namespace);
    } else {
      B2FATAL("The given filename and/or path is not valid: " + pythonFile);
    }
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

  //Initialize metadata service
  MetadataService::Instance();

  //Check for Belle2 environment variables (during environment initialisation)
  Environment::Instance();

  //Get the lib path (checked for NULL in Environment)
  const char* belle2SubDir = getenv("BELLE2_SUBDIR");
  boost::filesystem::path libPath = "lib";
  libPath /= belle2SubDir;

  string runModuleIOVisualization(""); //nothing done if empty
  vector<string> arguments;
  string pythonFile;

  try {
    //---------------------------------------------------
    //          Handle command line options
    //---------------------------------------------------

    prog::options_description generic("Generic options (to be used instead of steering file)");
    generic.add_options()
    ("help,h", "Print this help")
    ("version,v", "Print long and verbose version string")
    ("version-short", "Print short version string")
    ("info", "Print information about basf2")
    ("license", "Print the short version of the basf2 license")
    ("modules,m", prog::value<string>()->implicit_value(""),
     "Print a list of all available modules (can be limited to a given package), or give detailed information on a specific module given as an argument (case sensitive).")
    ;

    prog::options_description config("Configuration");
    config.add_options()
    ("steering", prog::value<string>(), "The python steering file to run.")
    ("arg", prog::value<vector<string> >(&arguments), "Additional arguments to be passed to the steering file")
    ("log_level,l", prog::value<string>(),
     "Set global log level (one of DEBUG, INFO, RESULT, WARNING, or ERROR). Takes precedence over set_log_level() in steering file.")
    ("random-seed", prog::value<string>(),
     "Set the default initial seed for the random number generator. "
     "This does not take precedence over calls to set_random_seed() in the steering file, but just changes the default. "
     "If no seed is set via either of these mechanisms, the initial seed will be taken from the system's entropy pool.")
    ("debug_level,d", prog::value<unsigned int>(), "Set default debug level. Also sets the log level to DEBUG.")
    ("events,n", prog::value<unsigned int>(), "Override number of events for EventInfoSetter; otherwise set maximum number of events.")
    ("run", prog::value<int>(), "Override run for EventInfoSetter, must be used with -n and --experiment")
    ("experiment", prog::value<int>(), "Override experiment for EventInfoSetter, must be used with -n and --run")
    ("skip-events", prog::value<unsigned int>(),
     "Override skipNEvents for EventInfoSetter and RootInput. Skips this many events before starting.")
    ("input,i", prog::value<vector<string> >(),
     "Override name of input file for (Seq)RootInput. Can be specified multiple times to use more than one file. For RootInput, wildcards (as in *.root or [1-3].root) can be used, but need to be escaped with \\  or by quoting the argument to avoid expansion by the shell.")
    ("sequence,S", prog::value<vector<string> >(),
     "Override the number sequence (e.g. 23:42,101) defining the entries (starting from 0) which are processed by RootInput."
     "Must be specified exactly once for each file to be opened."
     "This means one sequence per input file AFTER wildcard expansion."
     "The first event has the number 0.")
    ("output,o", prog::value<string>(),
     "Override name of output file for (Seq)RootOutput. In case multiple modules are present in the path, only the first will be affected.")
    ("processes,p", prog::value<int>(), "Override number of worker processes (>=1 enables, 0 disables parallel processing)");

    prog::options_description advanced("Advanced Options");
    advanced.add_options()
    ("module-io", prog::value<string>(),
     "Create diagram of inputs and outputs for a single module, saved as ModuleName.dot. To create a PostScript file, use e.g. 'dot ModuleName.dot -Tps -o out.ps'.")
    ("visualize-dataflow", "Generate data flow diagram (dataflow.dot) for the executed steering file.")
    ("no-stats",
     "Disable collection of statistics during event processing. Useful for very high-rate applications, but produces empty table with 'print(statistics)'.")
    ("dry-run",
     "Read steering file, but do not start any event processing when process(path) is called. Prints information on input/output files that would be used during normal execution.")
    ("dump-path", prog::value<string>(),
     "Read steering file, but do not actually start any event processing. The module path the steering file would execute is instead pickled (serialized) into the given file.")
    ("execute-path", prog::value<string>(),
     "Do not read any provided steering file, instead execute the pickled (serialized) path from the given file.")
    ("zmq",
     "Use ZMQ for multiprocessing instead of a RingBuffer. This has many implications and should only be used by experts.")
    ("job-information", prog::value<string>(),
     "Create json file with metadata of output files and basf2 execution status.")
    ("realm", prog::value<string>(),
     "Set the realm of the basf2 execution (online or production).")
#ifdef HAS_CALLGRIND
    ("profile", prog::value<string>(),
     "Name of a module to profile using callgrind. If more than one module of that name is registered only the first one will be profiled.")
#endif
    ;

    prog::options_description cmdlineOptions;
    cmdlineOptions.add(generic).add(config).add(advanced);

    prog::positional_options_description posOptDesc;
    posOptDesc.add("steering", 1);
    posOptDesc.add("arg", -1);

    prog::variables_map varMap;
    prog::store(prog::command_line_parser(argc, argv).
                options(cmdlineOptions).positional(posOptDesc).run(), varMap);
    prog::notify(varMap);

    //Check for non-steering file options
    if (varMap.count("help")) {
      cout << "Usage: " << argv[0] << " [OPTIONS] [STEERING_FILE] [-- [STEERING_FILE_OPTIONS]]\n";
      cout << cmdlineOptions << endl;
      return 0;
    } else if (varMap.count("version")) {
      pythonFile = "basf2/version.py";
    } else if (varMap.count("version-short")) {
      pythonFile = "basf2/version_short.py";
    } else if (varMap.count("info")) {
      pythonFile = "basf2_cli/print_info.py";
    } else if (varMap.count("license")) {
      pythonFile = "basf2_cli/print_license.py";
    } else if (varMap.count("modules")) {
      string modArgs = varMap["modules"].as<string>();
      if (!modArgs.empty()) {
        arguments.insert(arguments.begin(), modArgs);
      }
      // recent boost program_options will not consume extra tokens for
      // implicit options. In this case the module/package name gets consumed
      // in the steering file so we just use that.
      if (varMap.count("steering")) {
        arguments.insert(arguments.begin(), varMap["steering"].as<string>());
      }
      pythonFile = "basf2_cli/modules.py";
    } else if (varMap.count("module-io")) {
      runModuleIOVisualization = varMap["module-io"].as<string>();
      pythonFile = "basf2/core.py"; //make module maps available, visualization will happen later
    } else if (varMap.count("execute-path")) {
      Environment::Instance().setPicklePath(varMap["execute-path"].as<string>());
      pythonFile = "basf2_cli/execute_pickled_path.py";
    } else if (varMap.count("steering")) {
      // steering file not misused as module name, so print it's name :D
      pythonFile = varMap["steering"].as<string>();
      B2INFO("Steering file: " << pythonFile);
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

    // --zmq
    if (varMap.count("zmq")) {
      Environment::Instance().setUseZMQ(true);
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
        cmd.reserve(valgrind_argv.size());
        for (const auto& arg : valgrind_argv) { cmd.push_back(strdup(arg.c_str())); }
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

    // -n
    if (varMap.count("events")) {
      unsigned int nevents = varMap["events"].as<unsigned int>();
      if (nevents == 0 or nevents == std::numeric_limits<unsigned int>::max()) {
        B2FATAL("Invalid number of events (valid range: 1.." << std::numeric_limits<unsigned int>::max() - 1 << ")!");
      }
      Environment::Instance().setNumberEventsOverride(nevents);
    }
    // --run & --experiment
    if (varMap.count("experiment") or varMap.count("run")) {
      if (!varMap.count("events"))
        B2FATAL("--experiment and --run must be used with --events/-n!");
      if (!(varMap.count("run") and varMap.count("experiment")))
        B2FATAL("Both --experiment and --run must be specified!");

      int run = varMap["run"].as<int>();
      int experiment = varMap["experiment"].as<int>();
      B2ASSERT("run must be >= 0!", run >= 0);
      B2ASSERT("experiment must be >= 0!", experiment >= 0);
      Environment::Instance().setRunExperimentOverride(run, experiment);
    }

    // --skip-events
    if (varMap.count("skip-events")) {
      unsigned int skipevents = varMap["skip-events"].as<unsigned int>();
      Environment::Instance().setSkipEventsOverride(skipevents);
    }

    // -i
    if (varMap.count("input")) {
      const auto& names = varMap["input"].as<vector<string>>();
      Environment::Instance().setInputFilesOverride(names);
    }

    // -S
    if (varMap.count("sequence")) {
      const auto& sequences = varMap["sequence"].as<vector<string>>();
      Environment::Instance().setEntrySequencesOverride(sequences);
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

    // -d
    if (varMap.count("debug_level")) {
      unsigned int level = varMap["debug_level"].as<unsigned int>();
      LogSystem::Instance().getLogConfig()->setDebugLevel(level);
      LogSystem::Instance().getLogConfig()->setLogLevel(LogConfig::c_Debug);
    }

    if (varMap.count("visualize-dataflow")) {
      Environment::Instance().setVisualizeDataFlow(true);
      if (Environment::Instance().getNumberProcesses() > 0) {
        B2WARNING("--visualize-dataflow cannot be used with parallel processing, no graphs will be saved!");
      }
    }

    if (varMap.count("no-stats")) {
      Environment::Instance().setNoStats(true);
    }

    if (varMap.count("dry-run")) {
      Environment::Instance().setDryRun(true);
    }

    if (varMap.count("dump-path")) {
      Environment::Instance().setPicklePath(varMap["dump-path"].as<string>());
    }

    if (varMap.count("random-seed")) {
      RandomNumbers::initialize(varMap["random-seed"].as<string>());
    }

    if (varMap.count("job-information")) {
      string jobInfoFile = varMap["job-information"].as<string>();
      MetadataService::Instance().setJsonFileName(jobInfoFile);
      B2INFO("Job information file: " << jobInfoFile);
    }

    if (varMap.count("realm")) {
      std::string realmParam = varMap["realm"].as<string>();
      int realm = -1;
      for (int i = LogConfig::c_Online; i <= LogConfig::c_Production; i++) {
        std::string thisRealm = LogConfig::logRealmToString((LogConfig::ELogRealm)i);
        if (boost::iequals(realmParam, thisRealm)) { //case-insensitive
          realm = i;
          break;
        }
      }
      if (realm < 0) {
        B2FATAL("Invalid realm! Needs to be one of online or production.");
      }
      Environment::Instance().setRealm((LogConfig::ELogRealm)realm);
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
    //Search in local or central lib/ if this isn't a direct path
    if (!boost::filesystem::exists(pythonFile)) {
      std::string libFile = FileSystem::findFile((libPath / pythonFile).string(), true);
      if (!libFile.empty())
        pythonFile = libFile;
    }
  }

  try {
    //Init Python interpreter
    Py_InitializeEx(0);

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::vector<wstring> pyArgvString(arguments.size() + 1);
    // Set argument 0 to either script name or the basf2 exectuable
    if (!pythonFile.empty()) {
      pyArgvString[0] = converter.from_bytes(pythonFile);
    } else {
      pyArgvString[0] = converter.from_bytes(argv[0]);
    }
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

    //Report completion in json metadata
    MetadataService::Instance().addBasf2Status("finished successfully");
    MetadataService::Instance().finishBasf2();
  } catch (error_already_set&) {
    //Apparently an exception occured which wasn't handled. So print the traceback
    PyErr_Print();
    //And in rare cases, i.e. when redirecting output, the buffers are not
    //flushed unless we finalize python. So do it now
    Py_Finalize();
    return 1;
  }

  return 0;
}
