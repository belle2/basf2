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

#include <boost/program_options.hpp>
#include <iostream>

using namespace Belle2;
namespace po = boost::program_options;


int main(int argc, char* argv[])
{

  po::options_description options("Options");
  options.add_options()
  ("help", "produce help message")
  ("methodName", po::value<std::string>()->required(), "name of method")
  ("methodType", po::value<std::string>()->required(), "type of method")
  ("methodConfig", po::value<std::string>()->required(), "config of method")
  ("variables", po::value<std::vector<std::string>>()->multitoken(), "variables")
  ("target", po::value<std::string>()->required(), "target")
  ("prefix", po::value<std::string>(), "Prefix which is used by the TMVAInterface to store its configfile $prefix.config and by TMVA itself to write the files weights/$prefix_$method.class.C and weights/$prefix_$method.weights.xml with additional information")
  ("workingDirectory", po::value<std::string>(), "Working directory in which the config file and the weight file directory is created")
  ("factoryOption", po::value<std::string>(), "Option passed to TMVA::Factory")
  ("prepareOption", po::value<std::string>(), "Option passed to TMVA::Factory::PrepareTrainingAndTestTree")
  ("createMVAPDFs", "Creates the MVA PDFs for signal and background. This is needed to transform the output of the trained method to a probability.")
  ("maxEventsPerClass", po::value<int>(),  "Maximum number of events per class")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, options), vm);

  if (vm.count("help")) {
    std::cout << options << "\n";
    return 1;
  }
  //check for required arguments etc. after parsing --help
  po::notify(vm);

  std::string methodName = "";
  if (vm.count("methodName") == 1) {
    methodName = vm["methodName"].as<std::string>();
  } else {
    std::cerr << "Please input one methodName" << std::endl;
    return 1;
  }

  std::string methodType = "";
  if (vm.count("methodType") == 1) {
    methodType = vm["methodType"].as<std::string>();
  } else {
    std::cerr << "Please input one methodType" << std::endl;
    return 1;
  }

  std::string methodConfig = "";
  if (vm.count("methodConfig") == 1) {
    methodConfig = vm["methodConfig"].as<std::string>();
  } else {
    std::cerr << "Please input one methodConfig" << std::endl;
    return 1;
  }

  if (vm.count("createMVAPDFs")) {
    methodConfig = std::string("CreateMVAPdfs:") + methodConfig;
  }

  std::string target = "";
  if (vm.count("target") == 1) {
    target = vm["target"].as<std::string>();
  } else {
    std::cerr << "Please input one target" << std::endl;
    return 1;
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

  std::vector<std::string> variables;
  if (not vm["variables"].empty()) {
    variables = vm["variables"].as< std::vector<std::string> >();
  }

  if (variables.empty()) {
    std::cerr << "Please input some variables" << std::endl;
    return 1;
  }

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
  std::cout << "\t Variables " << std::endl;

  for (auto & var : variables) {
    std::cout << "\t\t " << var << std::endl;
  }

  std::vector<TMVAInterface::Method> methods;
  methods.push_back(TMVAInterface::Method(methodName, methodType, methodConfig, variables));
  TMVAInterface::Teacher* teacher = new TMVAInterface::Teacher(prefix, workingDirectory, target, methods, true);
  teacher->train(factoryOption, prepareOption, maxEventsPerClass);

  delete teacher;
  return 0;

}
