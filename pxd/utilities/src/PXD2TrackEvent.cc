/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Qingyuan Liu                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/utilities/PXD2TrackEvent.h>
#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/BeamSpot.h>
#include <mdst/dataobjects/Track.h>
#include <analysis/VertexFitting/RaveInterface/RaveVertexFitter.h>

using namespace Belle2;

bool PXD2TrackEvent::setValues(const StoreArray<RecoTrack>& recoTracks)
{
  // Exactly 2 tracks
  //if(recoTracks.size() != 2) return false;
  if (!recoTracks.isValid() or recoTracks.getEntries() != 2) return false;

  // Valid Tracks
  auto track1Ptr = recoTracks[0]->getRelated<Track>("Tracks");
  auto track2Ptr = recoTracks[1]->getRelated<Track>("Tracks");
  if (!track1Ptr or !track2Ptr) return false;

  // Valid trackFitResults
  auto tfr1Ptr = track1Ptr->getTrackFitResultWithClosestMass(Const::pion);
  auto tfr2Ptr = track2Ptr->getTrackFitResultWithClosestMass(Const::pion);
  if (!tfr1Ptr or !tfr2Ptr) return false;

  // Valid P Values
  if (tfr1Ptr->getPValue() <= 0 or tfr2Ptr->getPValue() <= 0) return false;

  // Opposite charges
  if (tfr1Ptr->getChargeSign() * tfr2Ptr->getChargeSign() >= 0) return false;

  // Setup Rave vertex fitter
  auto bField = BFieldManager::getField(TVector3(0, 0, 0)).Z() / Unit::T;
  B2DEBUG(20, "B Field = " << bField << " T");
  analysis::RaveSetup::initialize(1, bField);
  analysis::RaveVertexFitter rvf;
  try {
    rvf.addTrack(tfr1Ptr);
    rvf.addTrack(tfr2Ptr);
  }  catch (...) {
    B2DEBUG(20, "Adding tracks to RaveVertexFitter failed.");
    return false;
  }
  if (rvf.fit() == 0) return false;
  // get vertex from fitting
  auto vertex = rvf.getPos();
  // Reset RAVE
  analysis::RaveSetup::getInstance()->reset();

  // Set vertex
  m_vx = vertex.X();
  m_vy = vertex.Y();
  m_vz = vertex.Z();
  B2DEBUG(20, "Vertex = (" << m_vx
          << ", " << m_vy
          << ", " << m_vz
          << ")");

  // Set track containers
  DBObjPtr<BeamSpot> beamSpotDB; // beam spot is required to correct d0/z0
  auto ip = beamSpotDB->getIPPosition();
  if (tfr1Ptr->getChargeSign() > 0) {
    m_track_p.setValues(*recoTracks[0], ip);
    m_track_m.setValues(*recoTracks[1], ip);
  } else {
    m_track_p.setValues(*recoTracks[1], ip);
    m_track_m.setValues(*recoTracks[0], ip);
  }
  return true;
}
