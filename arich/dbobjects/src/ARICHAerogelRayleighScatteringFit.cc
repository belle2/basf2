/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/dbobjects/ARICHAerogelRayleighScatteringFit.h>
#include <iostream>
#include <iomanip>

using namespace Belle2;
using namespace std;

void ARICHAerogelRayleighScatteringFit::printContent(bool printHeader)
{
  if (printHeader) {
    cout << setw(10) << "version"
         << setw(10) << "serial"
         << setw(10) << "comment"
         << setw(15) << "p0"
         << setw(15) << "p1"
         << setw(15) << "p2"
         << setw(15) << "p3"
         << setw(15) << "p4"
         << setw(15) << "p5"
         << setw(15) << "p6" << endl;
  }
  cout << setw(10) << getAerogelMeasurementVersion()
       << setw(10) << getAerogelSerialNumber()
       << setw(10) << getComment();
  for (unsigned i = 0; i < getFitParameters().size(); i++)
    cout << setw(15) << getFitParameters().at(i);
  cout << endl;
}
