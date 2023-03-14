/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/vxdCDCTrackMerger/VXDCDCTrackMergerModule.h>

#include <tracking/trackFitting/fitter/base/TrackFitter.h>

using namespace Belle2;

REG_MODULE(VXDCDCTrackMerger);

VXDCDCTrackMergerModule::VXDCDCTrackMergerModule() :
  Module(), m_CDC_wall_radius(16.25)
{
  setDescription(
    "This module merges tracks which are reconstructed, separately, in the silicon (PXD+VXD) and in the CDC");
  setPropertyFlags(c_ParallelProcessingCertified);

  //input
  addParam("CDCRecoTrackColName", m_CDCRecoTrackColName, "CDC Reco Tracks");
  addParam("VXDRecoTrackColName", m_VXDRecoTrackColName, "VXD Reco Tracks");

  //merging parameters
  addParam("merge_radius", m_merge_radius,
           "Maximum distance between extrapolated tracks on the CDC wall",
           double(2.0));
}

void VXDCDCTrackMergerModule::initialize()
{
  m_CDCRecoTracks.isRequired(m_CDCRecoTrackColName);
  m_VXDRecoTracks.isRequired(m_VXDRecoTrackColName);

  m_CDCRecoTracks.registerRelationTo(m_VXDRecoTracks);
  m_VXDRecoTracks.registerRelationTo(m_CDCRecoTracks);
}

void VXDCDCTrackMergerModule::event()
{
  //get CDC tracks
  unsigned int nCDCTracks = m_CDCRecoTracks.getEntries();
  B2DEBUG(29, "VXDCDCTrackMerger: input Number of CDC Tracks: " << nCDCTracks);

  //get VXD tracks
  unsigned int nVXDTracks = m_VXDRecoTracks.getEntries();
  B2DEBUG(29,
          "VXDCDCTrackMerger: input Number of VXD Tracks: " << nVXDTracks);

  // position and momentum used for extrapolations to the CDC Wall
  B2Vector3D position(0., 0., 0.);
  B2Vector3D momentum(0., 0., 1.);
  // position and momentum of the track extrapolated from the CDC fit to the CDC Wall
  B2Vector3D cdcpos;
  B2Vector3D cdcmom;
  // position and momentum of the track extrapolated from the VXD fit to the CDC Wall
  B2Vector3D vxdpos;
  B2Vector3D vxdmom;

  // Fit all cdc and vxd tracks
  TrackFitter fitter;
  for (auto& cdcTrack : m_CDCRecoTracks) {
    fitter.fit(cdcTrack);
  }
  for (auto& vxdTrack : m_VXDRecoTracks) {
    fitter.fit(vxdTrack);
  }

  for (auto& cdcTrack : m_CDCRecoTracks) {
    // skip CDC Tracks which were not properly fitted
    if (!cdcTrack.wasFitSuccessful())
      continue;

    // skip CDC if it has already a match
    if (cdcTrack.getRelated<RecoTrack>(m_VXDRecoTrackColName)) {
      continue;
    }

    bool matched_track = false;

    // start search with worst chi^2 imaginable
    double this_chi_2 = std::numeric_limits<double>::max();
    double chi_2_max_this_cdc_track = std::numeric_limits<double>::max();
    try {
      genfit::MeasuredStateOnPlane cdc_sop =
        cdcTrack.getMeasuredStateOnPlaneFromFirstHit();
      cdc_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
      cdcpos = cdc_sop.getPos();
      cdcmom = cdc_sop.getMom();
    } catch (...) {
      B2DEBUG(29, "CDCTrack extrapolation to cylinder failed!");
      continue;
    }

    // TODO: this can be done independent of each other ....
    int currentVxdTrack = -1;
    int bestMatchedVxdTrack = 0;
    for (auto& vxdTrack : m_VXDRecoTracks) {
      currentVxdTrack++;
      // skip VXD Tracks which were not properly fitted
      if (!vxdTrack.wasFitSuccessful()) {
        continue;
      }

      // skip VXD if it has already a match
      if (vxdTrack.getRelated<RecoTrack>(m_CDCRecoTrackColName)) {
        continue;
      }

      try {
        genfit::MeasuredStateOnPlane vxd_sop =
          vxdTrack.getMeasuredStateOnPlaneFromLastHit();
        vxd_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
        vxdpos = vxd_sop.getPos();
        vxdmom = vxd_sop.getMom();
      } catch (genfit::Exception const&) {
        // extrapolation not possible, skip this track
        B2DEBUG(29, "VXDTrack extrapolation to cylinder failed!");
        continue;
      }

      // Extrapolate tracks to same plane & Match Tracks
      if (TMath::Sqrt((cdcpos - vxdpos) * (cdcpos - vxdpos)) < m_merge_radius) {
        genfit::MeasuredStateOnPlane vxd_sop;
        genfit::MeasuredStateOnPlane cdc_sop;
        try {
          vxd_sop = vxdTrack.getMeasuredStateOnPlaneFromLastHit();
          cdc_sop = cdcTrack.getMeasuredStateOnPlaneFromFirstHit();
          cdc_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
          vxd_sop.extrapolateToPlane(cdc_sop.getPlane());
        } catch (genfit::Exception const&) {
          // extrapolation not possible, skip this track
          B2DEBUG(29, "VXDTrack extrapolation to plane failed!");
          continue;
        }

        try {
          TMatrixDSym inv_covmtrx = (vxd_sop.getCov() + cdc_sop.getCov()).Invert();
          TVectorD state_diff = cdc_sop.getState() - vxd_sop.getState();
          state_diff *= inv_covmtrx;
          this_chi_2 = state_diff * (cdc_sop.getState() - vxd_sop.getState());
        } catch (...) {
          B2WARNING("Matrix is singular!");
          continue;
        }

        if ((this_chi_2 < chi_2_max_this_cdc_track) && (this_chi_2 > 0)) {
          matched_track = true;
          chi_2_max_this_cdc_track = this_chi_2;
          bestMatchedVxdTrack = currentVxdTrack;
        }
      }    //If on radius
    }    //end loop on VXD tracks

    if (matched_track) {
      // -1 is the convention for "before the CDC track" in the related tracks combiner
      m_VXDRecoTracks[bestMatchedVxdTrack]->addRelationTo(&cdcTrack, -1);
    }
  }
}
