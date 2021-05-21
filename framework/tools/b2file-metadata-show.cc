/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Belle 2 headers
#include <framework/core/FileCatalog.h>
#include <framework/dataobjects/FileMetaData.h>
#include <framework/io/RootFileInfo.h>
#include <framework/logging/Logger.h>

// ROOT headers
#include <TError.h>

// C++ headers
#include <csignal>
#include <iostream>
#include <string>

// Boost headers
#include <boost/program_options.hpp>

using namespace Belle2;
namespace prog = boost::program_options;

int main(int argc, char* argv[])
{
  //remove SIGPIPE handler set by ROOT which sometimes caused infinite loops
  //See https://savannah.cern.ch/bugs/?97991
  //default action is to abort
  if (std::signal(SIGPIPE, SIG_DFL) == SIG_ERR)
    B2FATAL("Cannot remove SIGPIPE signal handler");

  // Define command line options
  prog::options_description options("Options");
  options.add_options()
  ("help,h", "print all available options")
  ("file,f", prog::value<std::string>(), "local file name")
  ("lfn,l", prog::value<std::string>(), "logical file name")
  ("all,a", "print all information")
  ("json", "print machine-readable information in JSON format. Implies --all and --steering.")
  ("steering,s", "print steering file contents")
  ;

  prog::positional_options_description posOptDesc;
  posOptDesc.add("file", -1);

  prog::variables_map varMap;
  try {
    prog::store(prog::command_line_parser(argc, argv).
                options(options).positional(posOptDesc).run(), varMap);
    prog::notify(varMap);
  } catch (std::exception& e) {
    std::cout << "Problem parsing command line: " << e.what() << std::endl;
    std::cout << "Usage: " << argv[0] << " [OPTIONS] [FILE]\n";
    std::cout << options << std::endl;
    return 1;
  }

  //Check for help option
  if (varMap.count("help") or argc == 1) {
    std::cout << "Usage: " << argv[0] << " [OPTIONS] [FILE]\n";
    std::cout << options << std::endl;
    return 0;
  }

  FileMetaData metaData{};

  //Check for file and lfn options
  if (varMap.count("file")) {
    gErrorIgnoreLevel = kError;
    std::string fileName = varMap["file"].as<std::string>();
    try {
      RootIOUtilities::RootFileInfo fileInfo{fileName};
      metaData = fileInfo.getFileMetaData();
    } catch (const std::invalid_argument&) {
      B2FATAL("The input file can not be opened"
              << LogVar("File name", fileName));
    } catch (const std::runtime_error& e) {
      B2FATAL("Something went wrong with the input file"
              << LogVar("File name", fileName)
              << LogVar("Issue", e.what()));
    }
  } else if (varMap.count("lfn")) {
    std::string lfn = varMap["lfn"].as<std::string>();
    if (!FileCatalog::Instance().getMetaData(lfn, metaData))
      B2FATAL("No FileMetaData found in FileCatalog"
              << LogVar("LFN", varMap["lfn"].as<int>()));
  } else
    B2FATAL("Please specify either a file name or a LFN.");

  const char* option = "";
  if (varMap.count("json"))
    option = "json";
  else if (varMap.count("all"))
    option = "all";
  metaData.Print(option);
  if (std::string(option) != "json" and varMap.count("steering"))
    metaData.Print("steering");

  return 0;
}
