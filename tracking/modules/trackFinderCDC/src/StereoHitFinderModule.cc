/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/StereoHitFinderModule.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TFCDC_StereoHitFinder);
REG_MODULE(TFCDC_MonopoleStereoHitFinder);
REG_MODULE(TFCDC_MonopoleStereoHitFinderQuadratic);

TFCDC_StereoHitFinderModule::TFCDC_StereoHitFinderModule()
  : Super( {"CDCWireHitVector", "CDCTrackVector"})
{
}

TFCDC_MonopoleStereoHitFinderQuadraticModule::TFCDC_MonopoleStereoHitFinderQuadraticModule()
  : Super( {"CDCWireHitVector", "CDCMonopoleTrackVector"})
{
}

TFCDC_MonopoleStereoHitFinderModule::TFCDC_MonopoleStereoHitFinderModule()
  : Super( {"CDCWireHitVector", "CDCMonopoleTrackVector"})
{
}
