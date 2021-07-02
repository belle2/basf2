/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <alignment/GblMultipleScatteringController.h>

#include <genfit/GblFitter.h>

using namespace std;
using namespace Belle2;

void GblMultipleScatteringController::controlTrackSegment(TVector3, TVector3, double scatTheta, genfit::GblFitter* fitter)
// void GblMultipleScatteringController::controlTrackSegment(B2Vector3D, B2Vector3D, double scatTheta, genfit::GblFitter* fitter)
{
  bool enableScattering = true;

  if (scatTheta < 0.0001) enableScattering = false;

  if (!enableScattering)
    // No scattering
    fitter->setMSOptions(false, false);
  else
    // Thick scatterers
    fitter->setMSOptions(true, true);
}

