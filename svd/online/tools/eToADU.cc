/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/*
 * eToADU.cc
 *
 *  Created on: Jan 10, 2014
 *      Author: kvasnicka
 */

#include <iostream>
#include <math.h>

/** A small helper function to convert between electons and ADU */
double eToADU(double charge)
{
  double minADC = -96000;
  double maxADC = 288000;
  double unitADC = (maxADC - minADC) / 1024.0;
  return round(std::min(maxADC, std::max(minADC, charge)) / unitADC);
}

int main(int argc, const char* argv[])
{
  if (argc) {
    int input = atof(argv[1]);
    std::cout << "Electrons: " << input << " ADU: " << eToADU(input) << std::endl;
  } else {
    std::cout << "Give charge in electrons as command-line parameter" << std::endl;
  }
  return 0;
}




