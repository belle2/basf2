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
  int midPointInCDC = false;
  TVector3 midPoint = (exit + entry) * 0.5;
  TGeoNode* node    = gGeoManager->FindNode(midPoint[0], midPoint[1], midPoint[2]);

  if (!node)
    B2FATAL("Could not find geometry node between entry and exit point in the track segment.");

  const string cdcVolumePrefix("logicalSD_CDCLayer");
  string name = node->GetName();

  if (name.compare(0, cdcVolumePrefix.length(), cdcVolumePrefix) == 0)
    midPointInCDC = true;

  if (midPointInCDC)
    // No scattering
    fitter->setMSOptions(false, false);
  else
    // Thick scatterers
    fitter->setMSOptions(true, true);
}
