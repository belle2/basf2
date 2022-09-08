/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <mva/utility/Utility.h>
#include <iostream>

namespace po = boost::program_options;

int main(int argc, char* argv[])
{

  std::string identifier;
  std::string db_identifier;

  int event = 0;
  int run = 0;
  int experiment = 0;

  po::options_description description("Options");
  description.add_options()
  ("help", "print this message")
  ("identifier", po::value<std::string>(&identifier)->required(), "Identifier produced by basf2_mva_teacher")
  ("db_identifier", po::value<std::string>(&db_identifier)->required(), "Database identifier")
  ("experiment", po::value<int>(&experiment), "Experiment for which the weightfile should be valid")
  ("run", po::value<int>(&run), "Run for which the weightfile should be valid")
  ("event", po::value<int>(&event), "Experiment for which the weightfile should be valid");
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

  Belle2::MVA::Utility::download(db_identifier, identifier, experiment, run, event);

}
