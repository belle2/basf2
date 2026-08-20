/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "trg/cdc/modules/neurotrigger/CDCTrigger3DHNeuroDataModule.h"

#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>

#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include "trg/cdc/dataobjects/CDCTriggerSegmentHit.h"
#include "trg/cdc/dataobjects/CDCTrigger3DHTrack.h"
#include "tracking/dataobjects/RecoTrack.h"
#include "framework/datastore/StoreArray.h"
#include "framework/core/ModuleParam.templateDetails.h"
#include "framework/geometry/B2Vector3.h"

#define BOOST_MULTI_ARRAY_NO_GENERATORS

using namespace Belle2;

REG_MODULE(CDCTrigger3DHNeuroData);

CDCTrigger3DHNeuroDataModule::CDCTrigger3DHNeuroDataModule() : Module()
{
  setDescription(
    "This module takes 3dtracks, track segments, and target tracks (recotracks)\n"
    "as input and generates training data for the neurotrigger in a tab separated, gzip compressed file."
  );
  addParam("hitCollectionName", m_hitCollectionName,
           "Name of the input StoreArray of CDCTriggerSegmentHits (relations to input tracks required).",
           std::string(""));
  addParam("inputCollectionName", m_inputCollectionName,
           "Name of the StoreArray holding the 3DHough Finder input tracks.",
           std::string("CDCTriggerNNInput2DTracks"));
  addParam("targetCollectionName", m_targetCollectionName,
           "Name of the RecoTrack collection used as target values.",
           std::string("RecoTracks"));
  addParam("configFileName", m_configFileName,
           "Name of the configuration file.",
           std::string(""));
  addParam("gzipFilename", m_filename,
           "Name of the gzip file, where the training samples will be saved.",
           std::string("out.gz"));
  addParam("saveFakeEventTracks", m_saveFakeEventTracks,
           "Flag to save the 3DFinder tracks from fake events (no reconstructed track present).",
           false);
  addParam("saveFakeUnrelatedTracks", m_saveFakeUnrelatedTracks,
           "Flag to save the 3DFinder tracks that have no relation to a reconstructed track.",
           false);
}

void CDCTrigger3DHNeuroDataModule::initialize()
{
  m_ndFinderTracks.isRequired(m_inputCollectionName);
  m_recoTracks.isRequired(m_targetCollectionName);
  m_neuroParameters3DH = CDCTrigger3DHMLP::loadConfigFromFile(m_configFileName);
  m_neuroTrigger3DH.initialize();
  m_neuroTrigger3DH.setNeuroParameters(m_neuroParameters3DH);
  m_neuroTrigger3DH.initializeCollections(m_hitCollectionName);
  writeHeadline();
}

// Write the headline to the .gz file
void CDCTrigger3DHNeuroDataModule::writeHeadline() const
{
  std::ostringstream oss;
  const size_t inputPerSL = m_neuroParameters3DH.nInput / m_nSL;
  for (size_t superLayerIdx = 0; superLayerIdx < m_nSL; ++superLayerIdx) {
    oss << "SL" << superLayerIdx << "-relID\t";
    oss << "SL" << superLayerIdx << "-driftT\t";
    oss << "SL" << superLayerIdx << "-alpha\t";
    for (size_t i = 0; i < inputPerSL - 3; ++i) {
      oss << "SL" << superLayerIdx << "-extra_input" << i << "\t";
    }
  }
  oss << "RecoZ\tRecoTheta\tRecoNNTClassification\tRecoSTTClassification\tTrackType\n";
  // Write the headline (oss) to the .gz file
  std::ofstream gzipfile(m_filename, std::ios_base::trunc | std::ios_base::binary);
  boost::iostreams::filtering_ostream outStream;
  outStream.push(boost::iostreams::gzip_compressor());
  outStream.push(gzipfile);
  outStream << oss.str() << std::endl;
}

void CDCTrigger3DHNeuroDataModule::event()
{
  bool isFakeEvent = (m_recoTracks.getEntries() == 0);

  for (int trackIdx = 0; trackIdx < m_ndFinderTracks.getEntries(); ++trackIdx) {
    const CDCTrigger3DHTrack* ndFinderTrack = m_ndFinderTracks[trackIdx];

    TargetResult targetResult = computeTargetVector(*ndFinderTrack, isFakeEvent);
    std::vector<float> target = targetResult.targetVector;
    unsigned short trackType = targetResult.trackType;
    if (target.empty()) {
      continue;
    }

    m_neuroTrigger3DH.calculateTrackParameters(*ndFinderTrack);
    m_neuroTrigger3DH.setEventTime(*m_ndFinderTracks[trackIdx]);
    std::vector<size_t> hitIds = m_neuroTrigger3DH.load3DHits(*ndFinderTrack);
    std::vector<float> inputVector = m_neuroTrigger3DH.getInputVector(hitIds);

    std::ostringstream oss;
    for (size_t i = 0; i < inputVector.size(); ++i)
      oss << inputVector[i] << "\t";
    for (size_t i = 0; i < target.size(); ++i)
      oss << target[i] << "\t";
    oss << trackType << "\n";

    std::ofstream gzipfile(m_filename, std::ios_base::app | std::ios_base::binary);
    boost::iostreams::filtering_ostream outStream;
    outStream.push(boost::iostreams::gzip_compressor());
    outStream.push(gzipfile);
    outStream << oss.str();
  }
}

// Compute scaled target vector from reco track
CDCTrigger3DHNeuroDataModule::TargetResult CDCTrigger3DHNeuroDataModule::computeTargetVector(
  const CDCTrigger3DHTrack& ndFinderTrack, const bool isFakeEvent) const
{
  RecoTrack* recoTrack = ndFinderTrack.getRelatedTo<RecoTrack>(m_targetCollectionName);
  float z = 0., theta = 0., classificationNNT = -1., classificationSTT = -1.;

  const bool isUnrelatedFake = !isFakeEvent && (recoTrack == nullptr);
  const bool isFakeTrack     = isFakeEvent || isUnrelatedFake;

  if (isFakeEvent && !m_saveFakeEventTracks) { return {}; }
  if (isUnrelatedFake && !m_saveFakeUnrelatedTracks) { return {}; }

  if (!isFakeTrack) {
    const auto& reps = recoTrack->getRepresentations();
    for (auto* rep : reps) {
      if (!recoTrack->wasFitSuccessful(rep)) continue;
      try {
        auto state = recoTrack->getMeasuredStateOnPlaneClosestTo(ROOT::Math::XYZVector(0, 0, 0), rep);
        rep->extrapolateToLine(state, TVector3(0, 0, -1000), TVector3(0, 0, 2000));
        if (state.getMom().Dot(B2Vector3D(ndFinderTrack.getDirection())) < 0) {
          state.setPosMom(state.getPos(), -state.getMom());
          state.setChargeSign(-state.getCharge());
        }
        z = state.getPos().Z();
        theta = state.getMom().Theta();
        const auto& pRaw = state.getMom();
        float totalMomentum = std::sqrt(pRaw.Px() * pRaw.Px() + pRaw.Py() * pRaw.Py() + pRaw.Pz() * pRaw.Pz());
        bool fromIP = (std::abs(z) <= 1);
        if (fromIP) classificationNNT = 1.0f;
        if (fromIP && totalMomentum >= 0.7) classificationSTT = 1.0f;
        break;
      } catch (...) {
        continue;
      }
    }
  }

  std::vector<float> rawTarget = {
    isFakeTrack ? 0.0f : z,
    isFakeTrack ? static_cast<float>(std::atan2(1., ndFinderTrack.getCotTheta())) : theta,
    isFakeTrack ? -1.0f : classificationNNT,
    isFakeTrack ? -1.0f : classificationSTT
  };

  auto scaled = m_neuroTrigger3DH.scaleTarget(rawTarget);
  for (float& v : scaled) {
    v = std::clamp(v, -1.0f, 1.0f);
  }

  TrackType trackType = determineTrackType(classificationNNT, isFakeEvent, isUnrelatedFake);
  TargetResult targetResult;
  targetResult.targetVector = scaled;
  targetResult.trackType = static_cast<unsigned short>(trackType);
  return targetResult;
}

// Get the (target) track type
CDCTrigger3DHNeuroDataModule::TrackType CDCTrigger3DHNeuroDataModule::determineTrackType(
  const float classificationNNT, const bool isFakeEvent, const bool isUnrelatedFake) const
{
  if (classificationNNT == 1.0f)  return TrackType::Real;
  if (isFakeEvent)                return TrackType::Fake;
  if (isUnrelatedFake)            return TrackType::UnrelatedFake;
  if (classificationNNT == -1.0f) return TrackType::Background;
  return TrackType::Unknown;
}
