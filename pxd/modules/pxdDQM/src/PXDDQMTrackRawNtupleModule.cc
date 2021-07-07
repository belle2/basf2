/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdDQM/PXDDQMTrackRawNtupleModule.h>
#include <tracking/dataobjects/ROIid.h>

#include <pxd/reconstruction/PXDPixelMasker.h>
#include <mdst/dataobjects/Track.h>
#include <framework/gearbox/Const.h>

#include "TMatrixDSym.h"
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDDQMTrackRawNtuple)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDDQMTrackRawNtupleModule::PXDDQMTrackRawNtupleModule() : Module(), m_vxdGeometry(VXD::GeoCache::getInstance())
{
  // Set module properties
  setDescription("Create tuple PXD trigger studies");

  // setPropertyFlags(c_ParallelProcessingCertified);// for ntuple not certified!!!

  // Parameter definitions
  addParam("ntupleName", m_ntupleName, "name of ntuple file", std::string("rawhits.root"));
  addParam("pxdHitsName", m_pxdHitsName, "name of StoreArray with PXD raw hits", std::string(""));
  addParam("recoTracksName", m_recoTracksName, "name of StoreArray with RecoTracks", std::string(""));
  addParam("tracksName", m_tracksName, "name of StoreArray with Tracks", std::string(""));
  addParam("PXDInterceptListName", m_PXDInterceptListName, "name of the list of interceptions", std::string(""));
  addParam("useAlignment", m_useAlignment, "if true the alignment will be used", true);
  addParam("pCut", m_pcut, "Set a cut on the track fit p-value (0=no cut)", double(1e-20));
  addParam("minSVDHits", m_minSVDHits, "Number of SVD hits required in a track to be considered", 5u);
  addParam("momCut", m_momCut, "Set a cut on the track momentum in GeV/c, 0 disables", double(0.3));
  addParam("uDist", m_uDist, "distance in ucell to intercept to accept hit", int(10));
  addParam("vDist", m_vDist, "distance in vcell to intercept to accept hit", int(10));
}


void PXDDQMTrackRawNtupleModule::terminate()
{
  auto dir = gDirectory;
  if (m_tuple) {
    if (m_file) { // no file -> no write to file
      m_file->cd();
    }
    m_tuple->Write();
    delete m_tuple;
    m_tuple = nullptr;
  }
  if (m_file) {
    m_file->Write();
    m_file->Close();
    delete m_file;
    m_file = nullptr;
  }
  dir->cd();
}


void PXDDQMTrackRawNtupleModule::initialize()
{
  m_file = new TFile(m_ntupleName.data(), "recreate");
  if (m_file) m_file->cd();
  m_tuple = new TNtuple("trackraw", "trackraw", "vxdid:u:v:p:pt:framenr:triggergate");

  //register the required arrays
  //Register as optional so validation for cases where they are not available still succeeds, but module will not do any meaningful work without them
  m_pxdhits.isOptional(m_pxdHitsName);
  m_recoTracks.isOptional(m_recoTracksName);
  m_tracks.isOptional(m_tracksName);
  m_intercepts.isOptional(m_PXDInterceptListName);
  m_storeDAQEvtStats.isRequired();
}


void PXDDQMTrackRawNtupleModule::event()
{
  if (!m_pxdhits.isValid()) {
    B2INFO("PXDHits array is missing, will not do anything");
    return;
  }
  if (!m_recoTracks.isValid()) {
    B2INFO("RecoTrack array is missing, will not do anything");
    return;
  }
  if (!m_tracks.isValid()) {
    B2INFO("Track array is missing, will not do anything");
    return;
  }
  if (!m_intercepts.isValid()) {
    B2INFO("Intercept array is missing, will not do anything");
    return;
  }

  std::map<unsigned int, int> triggergate;
  auto evt = *m_storeDAQEvtStats;
  for (auto& pkt : evt) {
    for (auto& dhc : pkt) {
      for (auto& dhe : dhc) {
        triggergate[(unsigned int)dhe.getSensorID()] = dhe.getTriggerGate();
        // dhe.getFirstDataGate());
      }
    }
  }

  for (auto& track : m_tracks) {
    RelationVector<RecoTrack> recoTrack = track.getRelationsTo<RecoTrack>(m_recoTracksName);
    if (!recoTrack.size()) continue;

    auto a_track = recoTrack[0];
    //If fit failed assume position pointed to is useless anyway
    if (!a_track->wasFitSuccessful()) continue;

    if (a_track->getNumberOfSVDHits() < m_minSVDHits) continue;

    RelationVector<PXDIntercept> interceptList = a_track->getRelationsTo<PXDIntercept>(m_PXDInterceptListName);
    if (!interceptList.size()) continue;

    const genfit::FitStatus* fitstatus = a_track->getTrackFitStatus();
    if (fitstatus->getPVal() < m_pcut) continue;

    genfit::MeasuredStateOnPlane trackstate;
    trackstate = a_track->getMeasuredStateOnPlaneFromFirstHit();
    if (trackstate.getMom().Mag() < m_momCut) continue;
    if (trackstate.getMom().Pt() < m_pTCut) continue;

    const TrackFitResult* ptr2 = track.getTrackFitResultWithClosestMass(Const::pion);
    if (!ptr2) {
      B2ERROR("expect a track fit result for mass");
      continue;
    }

    //loop over all PXD sensors to get the intersections
    std::vector<VxdID> sensors = m_vxdGeometry.getListOfSensors();
    for (auto intercept : interceptList) {
      VxdID aVxdID = intercept.getSensorID();
      auto& info = m_vxdGeometry.getSensorInfo(aVxdID);
      //Search for intersections of the track with all PXD layers
      //Traditional (aka the person before did it like this) method
      //If there is a way to find out sensors crossed by a track directly, that would most likely be faster

      double u_fit = intercept.getCoorU();
      double v_fit = intercept.getCoorV();

      int ucell_fit = info.getUCellID(u_fit); // check wie overflow!!!
      int vcell_fit = info.getVCellID(v_fit); // Check wie overflow

      //loop the hits
      for (auto& hit : m_pxdhits) {
        //Do not consider as different if only segment differs!
        //As of this writing segment is never filled for hits, but just to be sure
        VxdID hitID = hit.getSensorID();
        if (aVxdID.getLayerNumber() != hitID.getLayerNumber() ||
            aVxdID.getLadderNumber() != hitID.getLadderNumber() ||
            aVxdID.getSensorNumber() != hitID.getSensorNumber()) {
          continue;
        }
        //only hit on the correct sensor and direction should survive

        auto u = hit.getUCellID();
        auto v = hit.getVCellID();
        if (abs(ucell_fit - u) < m_uDist && abs(vcell_fit - v) < m_vDist) {

          float fill[7] = {float((int)aVxdID), float(u), float(v), float(trackstate.getMom().Mag()), float(trackstate.getMom().Pt()), float(hit.getFrameNr()), float(triggergate[hitID])};
          m_tuple->Fill(fill);
        }
      }
    }
  }
}
