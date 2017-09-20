/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/interface/Interface.h>
#include <mva/interface/Options.h>
#include <mva/utility/Utility.h>

#include <iostream>
#include <string>
#include <cerrno>

namespace po = boost::program_options;
using namespace Belle2::MVA;

int main(int argc, char* argv[])
{

  AbstractInterface::initSupportedInterfaces();

  GeneralOptions general_options;
  po::options_description general_description(general_options.getDescription());

  MetaOptions meta_options;
  po::options_description meta_description(meta_options.getDescription());

  // Order of additional options
  // Loop over all classes from multi-class
  // Loop over all hyperparameters -> needs also apply functionality from expert
  // Do sPlot boost

  std::map<std::string, std::unique_ptr<SpecificOptions>> specific_options;

  for (auto& interface : AbstractInterface::getSupportedInterfaces()) {
    specific_options.emplace(interface.second->getName(), interface.second->getOptions());
  }

  po::variables_map vm;

  try {
    po::options_description cmdline_description;
    cmdline_description.add(general_description);
    cmdline_description.add(meta_description);

    po::parsed_options parsed = po::command_line_parser(argc, argv).options(cmdline_description).allow_unregistered().run();
    po::store(parsed, vm);

    if (vm.count("help")) {
      if (vm.count("method")) {
        std::string method = vm["method"].as<std::string>();
        if (specific_options.find(method) != specific_options.end()) {
          std::cout << specific_options[method]->getDescription() << std::endl;
        } else {
          std::cerr << "Provided method is unkown" << std::endl;
        }
      } else {
        std::cout << general_description << std::endl;
        std::cout << meta_description << std::endl;
      }
      return 1;
    }
    po::notify(vm);

    if (vm.count("method")) {
      std::string method = vm["method"].as<std::string>();
      if (specific_options.find(method) != specific_options.end()) {
        cmdline_description.add(specific_options[method]->getDescription());
        po::parsed_options specific_parsed = po::command_line_parser(argc, argv).options(cmdline_description).run();
        po::store(specific_parsed, vm);
        po::notify(vm);
      } else {
        std::cerr << "Provided method is unkown" << std::endl;
        return 1;
      }
    } else {
      std::cerr << "You must provide a method" << std::endl;
      return 1;
    }
  } catch (po::error& err) {
    std::cerr << "Error: " << err.what() << "\n";
    return 1;
  }

  //Check if method is available
  if (specific_options.find(general_options.m_method) == specific_options.end()) {
    std::cerr << "Unkown method " << general_options.m_method << std::endl;
  }

  Belle2::MVA::teacher(general_options, *specific_options[general_options.m_method], meta_options);

  return 0;

}

