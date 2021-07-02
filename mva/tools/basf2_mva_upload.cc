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
using namespace Belle2::MVA;

int main(int argc, char* argv[])
{

  std::string identifier;
  std::string db_identifier;

  int exp1 = 0;
  int exp2 = -1;
  int run1 = 0;
  int run2 = -1;

  po::options_description description("Options");
  description.add_options()
  ("help", "print this message")
  ("identifier", po::value<std::string>(&identifier)->required(), "Identifier produced by basf2_mva_teacher")
  ("db_identifier", po::value<std::string>(&db_identifier)->required(), "New database identifier for the method")
  ("begin_experiment", po::value<int>(&exp1), "First experiment for which the weightfile is valid")
  ("end_experiment", po::value<int>(&exp2), "Last experiment for which the weightfile is valid")
  ("begin_run", po::value<int>(&run1), "First run for which the weightfile is valid")
  ("end_run", po::value<int>(&run2), "Last run for which the weightfile is valid");
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

  Belle2::MVA::upload(identifier, db_identifier, exp1, run1, exp2, run2);


}
