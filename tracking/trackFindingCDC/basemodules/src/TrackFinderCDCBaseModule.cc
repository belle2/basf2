/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <framework/logging/Logger.h>

#include <functional>
#include <algorithm>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFinderCDCBaseModule::TrackFinderCDCBaseModule()
  : Module()
  , m_param_tracksStoreObjName("CDCTrackVector")
  , m_param_tracksStoreObjNameIsInput(false)
{
  this->setDescription("This a base module for all track finders in the CDC");

  this->addParam("TracksStoreObjName",
                 m_param_tracksStoreObjName,
                 "Name of the output StoreObjPtr of the tracks generated within this module.",
                 m_param_tracksStoreObjName);

  this->addParam("TracksStoreObjNameIsInput",
                 m_param_tracksStoreObjNameIsInput,
                 "Flag to use the CDCTracks in the given StoreObjPtr as input and output of the module",
                 m_param_tracksStoreObjNameIsInput);

  this->setPropertyFlags(c_ParallelProcessingCertified bitor c_TerminateInAllProcesses);
}

void TrackFinderCDCBaseModule::initialize()
{
  // Output store vector
  StoreWrappedObjPtr<std::vector<CDCTrack>> storedTracks(m_param_tracksStoreObjName);
  if (m_param_tracksStoreObjNameIsInput) {
    storedTracks.isRequired();
  } else {
    storedTracks.registerInDataStore(DataStore::c_DontWriteOut);
  }
}

void TrackFinderCDCBaseModule::event()
{
  // Now aquire the store vector
  StoreWrappedObjPtr<std::vector<CDCTrack>> storedTracks(m_param_tracksStoreObjName);

  if (m_param_tracksStoreObjNameIsInput) {
    // Somehow it is possible that we receive empty tracks (probably from the LegendreFinder)
    // Drop them before proceding
    std::vector<CDCTrack>& tracks = *storedTracks;
    auto itLast = std::remove_if(tracks.begin(), tracks.end(), std::mem_fun_ref(&CDCTrack::empty));
    tracks.erase(itLast, tracks.end());
  } else {
    storedTracks.create();
  }

  // We now let the generate-method fill or update the outputTracks
  std::vector<CDCTrack>& outputTracks = *storedTracks;
  this->generate(outputTracks);
}
