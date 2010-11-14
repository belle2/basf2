/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/geodetector/CreatorUtils.h>
#include <framework/logging/Logger.h>

#include <boost/format.hpp>

#include <TGeoManager.h>
#include <TGeoNode.h>
#include <TGeoTube.h>
#include <TGeoMatrix.h>

#include <cmath>

using namespace Belle2;
using namespace boost;
using namespace std;


void CreatorUtils::addArrow(const TVector3& position, const TVector3& direction, TGeoVolume* mother)
{
  //Make sure the gGeoManager was already created
  if (gGeoManager == NULL) {
    B2WARNING("Could not add arrow geometry because the TGeoManager was not yet created !")
    return;
  }

  //Make sure the length of the direction vector is not zero.
  double dirMag = direction.Mag();
  if (dirMag < 1e-7) {
    B2WARNING("Could not add arrow geometry because the direction vector has length zero !")
    return;
  }

  //Make sure the direction vector does not exactly point to the world up vector
  TVector3 input_direction = direction;
  if ((fabs(direction[0]) < 1e-7) && (fabs(direction[2]) < 1e-7)) {
    input_direction[2] = 1e-6;
  }

  TVector3 dir = input_direction.Unit();
  TVector3 up(-dir[1]*dir[0], 1 - (dir[1]*dir[1]), -dir[1]*dir[2]);
  TVector3 upNorm = up.Unit();
  TVector3 side = dir.Cross(upNorm);
  double currMatrix[9] = {side[0], upNorm[0], dir[0],
                          side[1], upNorm[1], dir[1],
                          side[2], upNorm[2], dir[2]
                         };

  TGeoRotation rotMatrix;
  rotMatrix.SetMatrix(currMatrix);
  TGeoTranslation moveMatrix(0.0, 0.0, dirMag);
  TGeoTranslation posMatrix(position[0], position[1], position[2]);

  TGeoHMatrix combiMatrix = posMatrix * rotMatrix * moveMatrix;
  TGeoHMatrix* finalMatrix = new TGeoHMatrix(combiMatrix);

  s_arrowCount++;

  TGeoVolume* arrowVol = gGeoManager->MakeTube((format("Arrow_%1%") % s_arrowCount).str().c_str(), NULL, 0.0, 0.15, dirMag);
  arrowVol->SetLineColor(3);

  //If the specified mother volume is not NULL, check if a subgroup
  //"Arrows" exist and use it as the mother volume. If it doesn't exist create it.
  TGeoVolume* motherVol = mother;
  if (motherVol == NULL) {
    motherVol = gGeoManager->FindVolumeFast("Arrows");
    if (motherVol == NULL) {
      TGeoVolume* topVolume = gGeoManager->GetTopVolume();
      motherVol = new TGeoVolumeAssembly("Arrows");
      topVolume->AddNode(motherVol, 1);
    }
  }
  motherVol->AddNode(arrowVol, s_arrowCount, finalMatrix);
}
