/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <iostream>
#include <vector>

#include <mva/utility/Utility.h>

namespace po = boost::program_options;
using namespace Belle2::MVA;

int main(int argc, char* argv[])
{

  std::vector<std::string> filenames;
  std::vector<std::string> datafiles;
  std::string treename = "variables";
  std::string outputfile;
  bool copy_target = true;

  po::options_description description("Options");
  description.add_options()
  ("help", "print this message")
  ("identifiers", po::value<std::vector<std::string>>(&filenames)->multitoken(), "Identifiers of the trained methods")
  ("datafiles", po::value<std::vector<std::string>>(&datafiles)->multitoken()->required(),
   "ROOT files containing the dataset")
  ("treename", po::value<std::string>(&treename), "Name of tree in ROOT datafile")
  ("outputfile", po::value<std::string>(&outputfile)->required(), "ROOT file containing the results")
  ("copy_target", po::value<bool>(&copy_target), "defines if the target variable should be copied");

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

  Belle2::MVA::Utility::expert(filenames, datafiles, treename, outputfile, 0, 0, 0, copy_target);
  return 0;

}

