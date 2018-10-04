/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leonid Burmistrov                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//basf2
#include <arich/dbobjects/ARICHGeoCooling.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <math.h>
#include <iostream>
#include <iomanip>

//root
#include <TVector3.h>

using namespace std;
using namespace Belle2;

void ARICHGeoCooling::print(const std::string& title) const
{

  ARICHGeoBase::print(title);

  cout << "Outer radius of merger PCB assembly envelope     : " << getEnvelopeOuterRadius() << endl
       << "Inner radius of merger PCB assembly envelope     : " << getEnvelopeInnerRadius() << endl
       << "Thickness of merger PCB assembly envelope        : " << getEnvelopeThickness() << endl;
  cout << "X0 merger PCB assembly envelope                  : " << getEnvelopeCenterPosition().X() << endl
       << "Y0 merger PCB assembly envelope                  : " << getEnvelopeCenterPosition().Y() << endl
       << "Z0 merger PCB assembly envelope                  : " << getEnvelopeCenterPosition().Z() << endl;
  cout << "material name of cooling pipe                    : " << getCoolingPipeMaterialName() << endl
       << "Size of cooling system pipe : inner radius in mm : " << getRmin() << endl
       << "Size of cooling system pipe : outer radius in mm : " << getRmax() << endl;
  cout << "Material name of cooling test plates             : " << getCoolingTestPlateMaterialName() << endl
       << "Size of cooling test plates (x)                  : " << getCoolingTestPlateslengths().X() << endl
       << "Size of cooling test plates (y)                  : " << getCoolingTestPlateslengths().Y() << endl
       << "Size of cooling test plates (z)                  : " << getCoolingTestPlateslengths().Z() << endl
       << "Radius of cold tubes                             : " << getColdTubeR() << endl
       << "Depth of the cold tube in the cooling plate      : " << getDepthColdTubeInPlate() << endl
       << "Distance from center of the cold tube to edge of cooling plate : " << getColdTubeSpacing() << endl
       << "number of cold tubes in one plate                : " << getColdTubeNumber() << endl;
  cout << "material name of cold tube                       : " << getColdTubeMaterialName() << endl
       << "outer radius of subtracted tubes for cold tube   : " << getColdTubeSubtractedR() << endl
       << "cold tube wall thickness                         : " << getColdTubeWallThickness() << endl;

  cout << setw(20) << " #" << setw(25) << "coolingTestPlatePosR" << setw(25) << "coolingTestPlatePosPhi" << setw(
         25) << "coolingTestPlatePosZ0" << endl;
  for (unsigned i = 0; i < getCoolingTestPlatePosR().size(); i++) {
    cout << setw(20) << i
         << setw(25) << getCoolingTestPlatePosR().at(i)
         << setw(25) << getCoolingTestPlatePosPhi().at(i)
         << setw(25) << getCoolingTestPlatePosZ0().at(i) << endl;
  }

  cout << setw(20) << " #" << setw(20) << "coolingGeometryID" << setw(20) << "coolingL" << setw(20) << "coolingPosPhi" << setw(
         20) << "coolingPosR" << setw(25) << "coolinRotationAngle" << endl;
  for (unsigned i = 0; i < getCoolingGeometryID().size(); i++) {
    cout << setw(20) << i
         << setw(20) << getCoolingGeometryID().at(i)
         << setw(20) << getCoolingL().at(i)
         << setw(20) << getCoolingPosPhi().at(i)
         << setw(20) << getCoolingPosR().at(i)
         << setw(25) << getCoolinRotationAngle().at(i) << endl;
  }

}

void ARICHGeoCooling::checkCoolingSystemDataConsistency() const
{

  B2ASSERT("Data of the cooling system positions is inconsisten : getCoolingGeometryID().size() != getCoolingL().size()",
           getCoolingGeometryID().size() == getCoolingL().size());
  B2ASSERT("Data of the cooling system positions is inconsisten : getCoolingL().size() != getCoolingPosPhi().size()",
           getCoolingL().size() == getCoolingPosPhi().size());
  B2ASSERT("Data of the cooling system positions is inconsisten : getCoolingPosPhi().size() != getCoolingPosR().size()",
           getCoolingPosPhi().size() == getCoolingPosR().size());
  B2ASSERT("Data of the cooling system positions is inconsisten : getCoolingPosR().size() != getCoolinRotationAngle().size()",
           getCoolingPosR().size() == getCoolinRotationAngle().size());
  B2ASSERT("Data of the cooling system positions is inconsisten : getCoolingTestPlatePosR().size() != getCoolingTestPlatePosPhi().size()",
           getCoolingTestPlatePosR().size() == getCoolingTestPlatePosPhi().size());
}
