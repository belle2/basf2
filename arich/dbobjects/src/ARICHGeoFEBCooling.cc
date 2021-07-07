/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//basf2
#include <arich/dbobjects/ARICHGeoFEBCooling.h>
#include <arich/dbobjects/tessellatedSolidStr.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <math.h>
#include <iostream>
#include <iomanip>

//root
#include <TVector3.h>

using namespace std;
using namespace Belle2;

void ARICHGeoFEBCooling::print(const std::string& title) const
{

  ARICHGeoBase::print(title);

  cout << " Size of the small square in mm              : " << getSmallSquareSize() << endl
       << " Thickness of the small square in mm         : " << getSmallSquareThickness() << endl
       << " Size of the big square in mm                : " << getBigSquareSize() << endl
       << " Thickness of the big square in mm           : " << getBigSquareThickness() << endl;
  cout << " Length of the rectangle in mm               : " << getRectangleL() << endl
       << " Width of the rectangle in mm                : " << getRectangleW() << endl
       << " Thickness of the rectangle in mm            : " << getRectangleThickness() << endl
       << " Distance from center of the rectangle in mm : " << getRectangleDistanceFromCenter() << endl;

  cout << setw(20) << "hapdID"
       << setw(25) << "FEB cooling GeometryID" << endl;
  for (unsigned i = 0; i < getFebcoolingv2GeometryID().size(); i++) {
    cout << setw(20) << i + 1
         << setw(25) << getFebcoolingv2GeometryID().at(i) << endl;
  }
}

void ARICHGeoFEBCooling::checkCoolingSystemV2DataConsistency() const
{
  B2ASSERT("Data of the cooling system V2 is inconsistent : getFebcoolingv2GeometryID().size() != 420",
           getFebcoolingv2GeometryID().size() == 420);
}
