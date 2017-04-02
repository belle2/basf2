/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <alignment/GblMultipleScatteringController.h>
#include <framework/logging/Logger.h>
#include <TGeoManager.h>

using namespace std;
using namespace Belle2;

void GblMultipleScatteringController::controlTrackSegment(TVector3 entry, TVector3 exit, genfit::GblFitter* fitter)
{
  TVector3 midPoint = (exit + entry) * 0.5;
  double R = sqrt(midPoint[0] * midPoint[0] + midPoint[1] * midPoint[1]);
  if (R > 16.8 and R < 110.23) {
    fitter->setMSOptions(false, false);
  } else {
    fitter->setMSOptions(true, true);
  }
//   int midPointInCDC = false;
//   TVector3 midPoint = (exit + entry) * 0.5;
//   if (!gGeoManager) {
//     B2ERROR("Could not access geometry manager.");
//     fitter->setMSOptions(true, true);
//     return;
//   }
//
//   TGeoNode* node    = gGeoManager->FindNode(midPoint[0], midPoint[1], midPoint[2]);
//
//   if (!node) {
//     B2ERROR("Could not find geometry node between entry and exit point in the track segment. Follows: entry, exit");
//     entry.Print();
//     exit.Print();
//     // As fall-back, use thick scatterers
//     fitter->setMSOptions(true, true);
//     return;
//   }
//
//   const string cdcVolumePrefix("logicalSD_CDCLayer");
//   string name = node->GetName();
//
//   if (name.compare(0, cdcVolumePrefix.length(), cdcVolumePrefix) == 0)
//     midPointInCDC = true;
//
//   if (midPointInCDC)
//     // No scattering
//     fitter->setMSOptions(false, false);
//   else
//     // Thick scatterers
//     fitter->setMSOptions(true, true);
}
