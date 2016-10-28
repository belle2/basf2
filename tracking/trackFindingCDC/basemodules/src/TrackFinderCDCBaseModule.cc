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
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>

#include <framework/datastore/StoreArray.h>
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
  setDescription("This a base module for all track finders in the CDC");

  ModuleParamList moduleParamList = this->getParamList();
  m_trackOrienter.exposeParameters(&moduleParamList);
  m_trackFlightTimeAdjuster.exposeParameters(&moduleParamList);
  m_trackExporter.exposeParameters(&moduleParamList);
  this->setParamList(moduleParamList);

  addParam("TracksStoreObjName",
           m_param_tracksStoreObjName,
           "Name of the output StoreObjPtr of the tracks generated within this module.",
           std::string("CDCTrackVector"));

  addParam("TracksStoreObjNameIsInput",
           m_param_tracksStoreObjNameIsInput,
           "Flag to use the CDCTracks in the given StoreObjPtr as input and output of the module",
           false);

  setPropertyFlags(c_ParallelProcessingCertified);
}

void TrackFinderCDCBaseModule::initialize()
{
  m_trackOrienter.initialize();
  m_trackFlightTimeAdjuster.initialize();
  m_trackExporter.initialize();

  // Output StoreArray
  StoreWrappedObjPtr<std::vector<CDCTrack>> storedTracks(m_param_tracksStoreObjName);
  if (m_param_tracksStoreObjNameIsInput) {
    storedTracks.isRequired();
  } else {
    storedTracks.registerInDataStore(DataStore::c_DontWriteOut);
  }
}

void TrackFinderCDCBaseModule::event()
{
  m_trackOrienter.beginEvent();
  m_trackFlightTimeAdjuster.beginEvent();
  m_trackExporter.beginEvent();

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

  std::vector<CDCTrack>& outputTracks = *storedTracks;

  // Either we just have to let the generate-method fill the outputTracks,
  // or we start from scratch with a cleared outputTracks.
  generate(outputTracks);

  std::vector<CDCTrack> orientedTracks;
  orientedTracks.reserve(20);
  m_trackOrienter.apply(outputTracks, orientedTracks);
  std::swap(outputTracks, orientedTracks);

  m_trackFlightTimeAdjuster.apply(outputTracks);
  m_trackExporter.apply(outputTracks);
}

void TrackFinderCDCBaseModule::generate(std::vector<CDCTrack>&)
{
}
