/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
            "This program allows you to test variable conditions (see https://confluence.desy.de/display/BI/Physics+VariableManager).\n";
  std::cout << "Please input a condition " << std::flush;
  std::getline(std::cin, condition);

  std::unique_ptr<Variable::Cut> cut = Variable::Cut::compile(condition);

  cut->print();

  TLorentzVector momentum(1, 2, 3, 4);
  Particle p(momentum, 421);
  std::cout << "This condition is: " << (cut->check(&p) ? "True" : "False") << std::endl;

  return 0;
}
