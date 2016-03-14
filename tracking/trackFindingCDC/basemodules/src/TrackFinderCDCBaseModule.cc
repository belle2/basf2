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

#include <framework/datastore/StoreArray.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>

#include <algorithm>
#include <framework/core/Module.h>
#include <framework/logging/Logger.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

//out type
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <genfit/TrackCand.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

TrackFinderCDCBaseModule::TrackFinderCDCBaseModule() :
  Module(),
  m_param_tracksStoreObjName("CDCTrackVector"),
  m_param_tracksStoreObjNameIsInput(false)
{
  setDescription("This a base module for all track finders in the CDC");

  ModuleParamList moduleParamList = this->getParamList();
  m_trackOrienter.exposeParameters(&moduleParamList);
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
  m_trackExporter.initialize();

  // Output StoreArray
  if (m_param_tracksStoreObjNameIsInput) {
    StoreWrappedObjPtr< std::vector<CDCTrack> >::required(m_param_tracksStoreObjName);
  } else {
    StoreWrappedObjPtr< std::vector<CDCTrack> >::registerTransient(m_param_tracksStoreObjName);
  }
}

void TrackFinderCDCBaseModule::event()
{
  m_trackOrienter.beginEvent();
  m_trackExporter.beginEvent();

  // Now aquire the store vector
  StoreWrappedObjPtr< std::vector<CDCTrack> > storedTracks(m_param_tracksStoreObjName);
  if (not m_param_tracksStoreObjNameIsInput) {
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

  m_trackExporter.apply(outputTracks);
}

void TrackFinderCDCBaseModule::generate(std::vector<CDCTrack>&)
{
}
