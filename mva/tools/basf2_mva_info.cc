/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <iostream>
#include <vector>

#include <mva/utility/Utility.h>

namespace po = boost::program_options;
using namespace Belle2::MVA;

int main(int argc, char* argv[])
{

  std::vector<std::string> filenames;

  po::options_description description("Options");
  description.add_options()
  ("help", "print this message")
  ("identifiers", po::value<std::vector<std::string>>(&filenames)->multitoken(), "Identifiers of the trained methods");

  po::variables_map vm;

  try {
    po::parsed_options parsed = po::command_line_parser(argc, argv).options(description).run();
    po::store(parsed, vm);

    if (vm.count("help")) {
      std::cout << description << std::endl;
      return 1;
    }
    po::notify(vm);
  } catch (po::error& err) {
    std::cerr << "Error: " << err.what() << "\n";
    return 1;
  }

  for (const auto& filename : filenames) {
    std::cout << "Information about " << filename << std::endl;
    std::cout << Belle2::MVA::info(filename) << std::endl;
    std::cout << std::endl;
  }
  return 0;

}

