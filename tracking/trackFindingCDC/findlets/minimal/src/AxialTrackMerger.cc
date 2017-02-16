/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun, Oliver Frost                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackMerger.h>

#include <tracking/trackFindingCDC/processing/TrackQualityTools.h>
#include <tracking/trackFindingCDC/processing/TrackProcessor.h>
#include <tracking/trackFindingCDC/processing/HitProcessor.h>

#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string AxialTrackMerger::getDescription()
{
  return "Merges axial tracks found in the legendre search";
}

void AxialTrackMerger::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
}

void AxialTrackMerger::apply(std::vector<CDCTrack>& axialTracks,
                             const std::vector<const CDCWireHit*>& allAxialWireHits)
{
  TrackProcessor::mergeAndFinalizeTracks(axialTracks, allAxialWireHits);
  HitProcessor::resetMaskedHits(axialTracks, allAxialWireHits);
  erase_remove_if(axialTracks, Size() == 0u);
}
