/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/TMVAInterface/Method.h>
#include <analysis/TMVAInterface/Teacher.h>

#include <framework/logging/LogSystem.h>

#include <boost/program_options.hpp>
#include <iostream>

using namespace Belle2;
namespace po = boost::program_options;


int main(int argc, char* argv[])
{

  po::options_description options("Options");
  options.add_options()
  ("help", "print this message")
  ("methodName", po::value<std::string>()->required(), "name of method")
  ("methodType", po::value<std::string>()->required(), "type of method")
  ("methodConfig", po::value<std::string>()->required(), "config of method")
  ("variables", po::value<std::vector<std::string>>()->required()->multitoken(), "variables")
  ("target", po::value<std::string>(), "target variable used to distinguish between signal and background, isSignal is used as default.")
  ("prefix", po::value<std::string>(), "Prefix which is used by the TMVAInterface to store its configfile $prefix.config and by TMVA itself to write the files weights/$prefix_$method.class.C and weights/$prefix_$method.weights.xml with additional information")
  ("workingDirectory", po::value<std::string>(), "Working directory in which the config file and the weight file directory is created")
  ("factoryOption", po::value<std::string>(), "Option passed to TMVA::Factory")
  ("prepareOption", po::value<std::string>(), "Option passed to TMVA::Factory::PrepareTrainingAndTestTree")
  ("createMVAPDFs", "Creates the MVA PDFs for signal and background. This is needed to transform the output of the trained method to a probability.")
  ("maxEventsPerClass", po::value<int>(),  "Maximum number of events per class")
  ;

  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, options), vm);

    if (vm.count("help")) {
      std::cout << options << "\n";
      return 1;
    }

    //check for required arguments etc. after parsing --help
    po::notify(vm);
  } catch (po::error& err) {
    std::cerr << "Error: " << err.what() << "\n";
    return 1;
  }

  std::string methodName = vm["methodName"].as<std::string>();
  std::string methodType = vm["methodType"].as<std::string>();
  std::string methodConfig = vm["methodConfig"].as<std::string>();

  if (vm.count("createMVAPDFs")) {
    methodConfig = std::string("CreateMVAPdfs:") + methodConfig;
  }

  std::string target = "isSignal";
  if (vm.count("target") == 1) {
    target = vm["target"].as<std::string>();
  }

  std::string prefix = "TMVA";
  if (vm.count("prefix") == 1) {
    prefix = vm["prefix"].as<std::string>();
  }

  std::string workingDirectory = ".";
  if (vm.count("workingDirectory") == 1) {
    workingDirectory = vm["workingDirectory"].as<std::string>();
  }

  std::string factoryOption = "!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification";
  if (vm.count("factoryOption") == 1) {
    factoryOption = vm["factoryOption"].as<std::string>();
  }

  std::string prepareOption = "SplitMode=random:!V";
  if (vm.count("prepareOption") == 1) {
    prepareOption = vm["prepareOption"].as<std::string>();
  }

  std::vector<std::string> variables = vm["variables"].as< std::vector<std::string> >();

  int maxEventsPerClass = 0;
  if (vm.count("maxEventsPerClass") == 1) {
    maxEventsPerClass = vm["maxEventsPerClass"].as<int>();
  }

  std::cout << "Received options" << std::endl;
  std::cout << "\t methodName \t" << methodName << std::endl;
  std::cout << "\t methodType \t" << methodType << std::endl;
  std::cout << "\t methodConfig \t" << methodConfig << std::endl;
  std::cout << "\t prefix \t" << prefix << std::endl;
  std::cout << "\t workingDirectory \t" << workingDirectory << std::endl;
  std::cout << "\t target \t" << target << std::endl;
  std::cout << "\t factoryOption \t" << factoryOption << std::endl;
  std::cout << "\t prepareOption \t" << prepareOption << std::endl;
  std::cout << "\t maxEventsPerClass \t" << maxEventsPerClass << std::endl;
  std::cout << "\t variables " << std::endl;

  for (auto & var : variables) {
    std::cout << "\t\t " << var << std::endl;
  }

  std::vector<TMVAInterface::Method> methods;
  methods.push_back(TMVAInterface::Method(methodName, methodType, methodConfig, variables));
  // weight == constant(1), the weights will be taken from the defined root file, the constant(1) variable will be never used!

  //check this didn't result in errors...
  if (LogSystem::Instance().getMessageCounter(LogConfig::c_Error) > 0) {
    std::cerr << "Errors encountered during method initialisation, aborting.\n";
    return 1;
  }

  TMVAInterface::Teacher* teacher = new TMVAInterface::Teacher(prefix, workingDirectory, target, "constant(1)", methods, true);
  teacher->train(factoryOption, prepareOption, maxEventsPerClass);

  delete teacher;
  return 0;

}
