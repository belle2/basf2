#include "trg/cdc/modules/trgcdc/CDCTriggerTrackCombinerModule.h"

#include <framework/datastore/RelationVector.h>

using namespace std;
using namespace Belle2;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
REG_MODULE(CDCTriggerTrackCombiner);

CDCTriggerTrackCombinerModule::CDCTriggerTrackCombinerModule() : Module::Module()
{
  setDescription(
    "The combiner module of the CDC trigger.\n"
    "Takes tracks from the various trigger stages (finder, fitter, neuro) "
    "and combines them to a single track list. "
    "The 2D track parameters are taken from the fitter, if present. "
    "For the 3D track parameters there are different options (see parameters). "
    "Combined tracks are not produced for tracks without 3D information "
    "from either 3D fitter or neuro.\n"
  );

  addParam("finder2DCollectionName", m_2DfinderCollectionName,
           "Name of the 2D finder track list.",
           string("TRGCDC2DFinderTracks"));
  addParam("fitter2DCollectionName", m_2DfitterCollectionName,
           "Name of the 2D fitter track list.",
           string("TRGCDC2DFitterTracks"));
  addParam("fitter3DCollectionName", m_3DfitterCollectionName,
           "Name of the 3D fitter track list.",
           string("TRGCDC3DFitterTracks"));
  addParam("neuroCollectionName", m_neuroCollectionName,
           "Name of the neuro track list.",
           string("TRGCDCNeuroTracks"));
  addParam("outputCollectionName", m_outputCollectionName,
           "Name of the output track list for the combined tracks.",
           string("TRGCDCTracks"));
  addParam("hitCollectionName", m_hitCollectionName,
           "Name of track segment hit list, needed for hit relations.",
           string(""));
  addParam("thetaDefinition", m_thetaDefinition,
           "Select how to define cot(theta). Valid options are "
           "3Dfitter (use 3Dfitter if present, otherwise neuro), "
           "neuro (use neuro if present, otherwise 3Dfitter) or "
           "avg (average 3Dfitter and neuro).",
           string("avg"));
  addParam("zDefinition", m_zDefinition,
           "Select how to define z0. Valid options are "
           "3Dfitter (use 3Dfitter if present, otherwise neuro), "
           "neuro (use neuro if present, otherwise 3Dfitter), "
           "avg (average 3Dfitter and neuro), "
           "min (smaller |z0| of 3Dfitter and neuro) or "
           "max (larger |z0| of 3Dfitter and neuro).",
           string("min"));
}

void
CDCTriggerTrackCombinerModule::initialize()
{
  // register DataStore elements
  StoreArray<CDCTriggerTrack> tracks2Dfitter(m_2DfitterCollectionName);
  StoreArray<CDCTriggerTrack> tracks3Dfitter(m_3DfitterCollectionName);
  StoreArray<CDCTriggerTrack> tracksNeuro(m_neuroCollectionName);
  m_tracks2Dfinder.isRequired(m_2DfinderCollectionName);
  tracks2Dfitter.isRequired(m_2DfitterCollectionName);
  tracks3Dfitter.isRequired(m_3DfitterCollectionName);
  tracksNeuro.isRequired(m_neuroCollectionName);
  m_tracksCombined.registerInDataStore(m_outputCollectionName);
  // register relations
  m_tracks2Dfinder.registerRelationTo(m_tracksCombined);
  m_tracksCombined.registerRelationTo(m_segmentHits);
}

void
CDCTriggerTrackCombinerModule::event()
{
  for (int itrack = 0; itrack < m_tracks2Dfinder.getEntries(); ++itrack) {
    CDCTriggerTrack* track2Dfinder = m_tracks2Dfinder[itrack];
    CDCTriggerTrack* track2Dfitter =
      track2Dfinder->getRelatedTo<CDCTriggerTrack>(m_2DfitterCollectionName);
    CDCTriggerTrack* track3Dfitter =
      (track2Dfitter)
      ? track2Dfitter->getRelatedTo<CDCTriggerTrack>(m_3DfitterCollectionName)
      : nullptr;
    CDCTriggerTrack* trackNeuro =
      track2Dfinder->getRelatedTo<CDCTriggerTrack>(m_neuroCollectionName);

    if (!track3Dfitter && !trackNeuro) {
      B2DEBUG(100, "skip track without 3D parameters");
      continue;
    }

    // 2D: take fitter if present, otherwise finder
    CDCTriggerTrack* track2D = (track2Dfitter) ? track2Dfitter : track2Dfinder;

    // 3D: get parameters separately according to options
    double z = 0;
    double cotTheta = 0;
    double chi3D = 0;
    bool usedFit = false;
    bool usedNN = false;

    if (!track3Dfitter) {
      B2DEBUG(100, "no 3D fitter results, use neuro results");
      z = trackNeuro->getZ0();
      cotTheta = trackNeuro->getCotTheta();
      chi3D = trackNeuro->getChi3D();
      usedNN = true;
    } else if (!trackNeuro) {
      B2DEBUG(100, "no neuro results, use 3D fitter results");
      z = track3Dfitter->getZ0();
      cotTheta = track3Dfitter->getCotTheta();
      chi3D = track3Dfitter->getCotTheta();
      usedFit = true;
    } else {
      B2DEBUG(100, "combine 3D fitter and neuro results");
      // z options
      if (m_zDefinition == "3Dfitter") {
        z = track3Dfitter->getZ0();
        usedFit = true;
      } else if (m_zDefinition == "neuro") {
        z = trackNeuro->getZ0();
        usedNN = true;
      } else if (m_zDefinition == "avg") {
        z = (track3Dfitter->getZ0() + trackNeuro->getZ0()) / 2.;
        usedFit = true;
        usedNN = true;
      } else if (m_zDefinition == "min") {
        z = (abs(track3Dfitter->getZ0()) < abs(trackNeuro->getZ0()))
            ? track3Dfitter->getZ0()
            : trackNeuro->getZ0();
        usedFit = true;
        usedNN = true;
      } else if (m_zDefinition == "max") {
        z = (abs(track3Dfitter->getZ0()) > abs(trackNeuro->getZ0()))
            ? track3Dfitter->getZ0()
            : trackNeuro->getZ0();
        usedFit = true;
        usedNN = true;
      } else {
        B2ERROR("invalid option for zDefinition " << m_zDefinition
                << " (choose one of 3Dfitter, neuro, avg, min, max)");
      }
      // theta options
      if (m_thetaDefinition == "3Dfitter") {
        cotTheta = track3Dfitter->getCotTheta();
        usedFit = true;
      } else if (m_thetaDefinition == "neuro") {
        cotTheta = trackNeuro->getCotTheta();
        usedNN = true;
      } else if (m_thetaDefinition == "avg") {
        cotTheta = (track3Dfitter->getCotTheta() + trackNeuro->getCotTheta()) / 2.;
        usedFit = true;
        usedNN = true;
      } else {
        B2ERROR("invalid option for thetaDefinition " << m_thetaDefinition
                << " (choose one of 3Dfitter, neuro, avg)");
      }
      // only fitter calculates a chi2 value
      if (track3Dfitter)
        chi3D = track3Dfitter->getChi3D();
    }

    // create new track
    CDCTriggerTrack* combinedTrack =
      m_tracksCombined.appendNew(track2D->getPhi0(), track2D->getOmega(), track2D->getChi2D(),
                                 z, cotTheta, chi3D);
    track2Dfinder->addRelationTo(combinedTrack);
    // get hit relations from all tracks, but without duplicates
    vector<bool> usedHit(m_segmentHits.getEntries(), false);
    for (const CDCTriggerSegmentHit& hit :
         track2Dfinder->getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName)) {
      combinedTrack->addRelationTo(&hit);
      usedHit[hit.getArrayIndex()] = true;
    }
    if (track2Dfitter) {
      for (const CDCTriggerSegmentHit& hit :
           track2Dfitter->getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName)) {
        if (!usedHit[hit.getArrayIndex()]) {
          combinedTrack->addRelationTo(&hit);
          usedHit[hit.getArrayIndex()] = true;
        }
      }
    }
    if (track3Dfitter && usedFit) {
      for (const CDCTriggerSegmentHit& hit :
           track3Dfitter->getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName)) {
        if (!usedHit[hit.getArrayIndex()]) {
          combinedTrack->addRelationTo(&hit);
          usedHit[hit.getArrayIndex()] = true;
        }
      }
    }
    if (trackNeuro && usedNN) {
      for (const CDCTriggerSegmentHit& hit :
           trackNeuro->getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName)) {
        if (!usedHit[hit.getArrayIndex()]) {
          combinedTrack->addRelationTo(&hit);
          usedHit[hit.getArrayIndex()] = true;
        }
      }
    }
  }
}
