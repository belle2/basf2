/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/Particle.h>

#include <iostream>
#include <string>
#include <memory>

using namespace Belle2;

int main()
{
  std::string condition;
  std::cout <<
            "This program allows you to test variable conditions (see the VariableManager section at https://software.belle2.org/).\n";
  std::cout << "Please input a condition " << std::flush;
  std::getline(std::cin, condition);

  std::unique_ptr<Variable::Cut> cut = Variable::Cut::compile(condition);

  cut->print();

  TLorentzVector momentum(1, 2, 3, 4);
  Particle p(momentum, 421);
  std::cout << "This condition is: " << (cut->check(&p) ? "True" : "False") << std::endl;

  return 0;
}
