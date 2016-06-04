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
#include <analysis/TMVAInterface/Expert.h>
#include <analysis/TMVAInterface/Config.h>
#include <analysis/TMVAInterface/SPlot.h>

#include <analysis/VariableManager/Utility.h>

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
  ("methodName", po::value<std::string>(), "name of method")
  ("methodType", po::value<std::string>(), "type of method")
  ("methodConfig", po::value<std::string>(), "config of method")
  ("variables", po::value<std::vector<std::string>>()->required()->multitoken(), "variables")
  ("spectators", po::value<std::vector<std::string>>()->multitoken(), "spectators")
  ("target", po::value<std::string>(),
   "target variable used to distinguish between signal and background, isSignal is used as default.")
  ("weight", po::value<std::string>(), "weight variable")
  ("prefix", po::value<std::string>(),
   "Prefix which is used by the TMVAInterface to store its configfile $prefix.config and by TMVA itself to write the files weights/$prefix_$method.class.C and weights/$prefix_$method.weights.xml with additional information")
  ("treeName", po::value<std::string>(), "name of tree in output file")
  ("workingDirectory", po::value<std::string>(),
   "Working directory in which the config file and the weight file directory is created")
  ("factoryOption", po::value<std::string>(), "Option passed to TMVA::Factory")
  ("prepareOption", po::value<std::string>(), "Option passed to TMVA::Factory::PrepareTrainingAndTestTree")
  ("modelFileName", po::value<std::string>(), "sPlot model filename")
  ("discriminatingVariables", po::value<std::vector<std::string>>()->multitoken(), "discriminating variables used for sPlot training")
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


  std::string methodName = "FastBDT";
  if (vm.count("methodName") == 1) {
    methodName = vm["methodName"].as<std::string>();
  }

  std::string methodType = "Plugin";
  if (vm.count("methodType") == 1) {
    methodType = vm["methodType"].as<std::string>();
  }

  std::string methodConfig = "!H:!V:CreateMVAPdfs:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3";
  if (vm.count("methodConfig") == 1) {
    methodConfig = vm["methodConfig"].as<std::string>();
  }

  std::string target = "";
  if (vm.count("target") == 1) {
    target = vm["target"].as<std::string>();
  }

  std::string weight = "";
  if (vm.count("weight") == 1) {
    weight = vm["weight"].as<std::string>();
  }

  std::string prefix = "TMVA";
  if (vm.count("prefix") == 1) {
    prefix = vm["prefix"].as<std::string>();
  }

  std::string treeName = "TMVA_tree";
  if (vm.count("treeName") == 1) {
    treeName = vm["treeName"].as<std::string>();
  }


  std::string workingDirectory = ".";
  if (vm.count("workingDirectory") == 1) {
    workingDirectory = vm["workingDirectory"].as<std::string>();
  }

  std::string factoryOption = "!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification";
  if (vm.count("factoryOption") == 1) {
    factoryOption = vm["factoryOption"].as<std::string>();
  }

  std::vector<std::string> variables = vm["variables"].as< std::vector<std::string> >();

  std::vector<std::string> spectators;
  if (vm.count("spectators") >= 1) {
    spectators = vm["spectators"].as< std::vector<std::string> >();
  }

  std::vector<std::string> discriminatingVariables;
  if (vm.count("discriminatingVariables") >= 1) {
    discriminatingVariables = vm["discriminatingVariables"].as< std::vector<std::string> >();
  }

  std::string modelFileName = "";
  if (vm.count("modelFileName") == 1) {
    modelFileName = vm["modelFileName"].as<std::string>();
  }

  std::string prepareOption = "SplitMode=random:!V";
  // If sPlot mode
  if (modelFileName != "") {
    prepareOption = "!V:SplitMode=alternate:MixMode=Block:NormMode=None";
  }

  if (vm.count("prepareOption") == 1) {
    prepareOption = vm["prepareOption"].as<std::string>();
  }


  std::cout << "Received options" << std::endl;
  std::cout << "\t methodName \t" << methodName << std::endl;
  std::cout << "\t methodType \t" << methodType << std::endl;
  std::cout << "\t methodConfig \t" << methodConfig << std::endl;
  std::cout << "\t prefix \t" << prefix << std::endl;
  std::cout << "\t workingDirectory \t" << workingDirectory << std::endl;
  std::cout << "\t target \t" << target << std::endl;
  std::cout << "\t weight \t" << weight << std::endl;
  std::cout << "\t factoryOption \t" << factoryOption << std::endl;
  std::cout << "\t prepareOption \t" << prepareOption << std::endl;

  std::cout << "\t Discriminating variables" << std::endl;
  for (auto& var : discriminatingVariables) {
    std::cout << "\t\t" << var << std::endl;
  }
  std::cout << "\t Spectator variables" << std::endl;
  for (auto& var : spectators) {
    std::cout << "\t\t" << var << std::endl;
  }
  std::cout << "\t variables " << std::endl;
  for (auto& var : variables) {
    std::cout << "\t\t " << var << std::endl;
  }

  if (weight != "" and std::find(spectators.begin(), spectators.end(), weight) == spectators.end()) {
    spectators.push_back(weight);
  }

  if (target != "" and std::find(spectators.begin(), spectators.end(), target) == spectators.end()) {
    spectators.push_back(target);
  }

  for (auto& var : discriminatingVariables) {
    if (std::find(spectators.begin(), spectators.end(), var) == spectators.end()) {
      spectators.push_back(var);
    }
  }

  std::vector<TMVAInterface::Method> methods;
  methods.push_back(TMVAInterface::Method(methodName, methodType, methodConfig));

  //check this didn't result in errors...
  if (LogSystem::Instance().getMessageCounter(LogConfig::c_Error) > 0) {
    std::cerr << "Errors encountered during method initialisation, aborting.\n";
    return 1;
  }

  TMVAInterface::TeacherConfig config(prefix, treeName, workingDirectory, variables, spectators, methods);
  auto teacher = std::unique_ptr<TMVAInterface::Teacher>(new TMVAInterface::Teacher(config, true));

  target = makeROOTCompatible(target);
  weight = makeROOTCompatible(weight);

  if (target == "") {
    teacher->trainSPlot(modelFileName, discriminatingVariables, weight);
  } else {
    teacher->trainClassification(factoryOption, prepareOption, target, weight);
  }

  return 0;

}
