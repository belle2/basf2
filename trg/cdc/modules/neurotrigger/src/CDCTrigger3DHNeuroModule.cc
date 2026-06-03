/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "trg/cdc/modules/neurotrigger/CDCTrigger3DHNeuroModule.h"

#include <cmath>
#include <vector>
#include <array>

#include "trg/cdc/dataobjects/CDCTriggerHoughMLP.h"
#include "framework/database/DBObjPtr.h"

using namespace Belle2;

REG_MODULE(CDCTrigger3DHNeuro);

CDCTrigger3DHNeuroModule::CDCTrigger3DHNeuroModule() : Module()
{
  setDescription(
    "The NeuroTrigger3DH module of the CDC trigger.\n"
    "Takes track segments and 3D track estimates as input and estimates\n"
    "the z-vertex and classification for each track using a neural network.\n"
    "Requires a trained network stored in a file.\n"
  );
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("fileName", m_fileName,
           "Name of the network to load. Contains the configuration parameters."
           "When left blank, the network is loaded from the ConditionsDB.",
           std::string(""));
  addParam("arrayName", m_arrayName,
           "Name of the TObjArray holding the NeuroTrigger3DH parameters.",
           std::string("MLP"));
  addParam("hitCollectionName", m_hitCollectionName,
           "Name of the input StoreArray of CDCTriggerSegmentHits.",
           std::string(""));
  addParam("inputCollectionName", m_inputCollectionName,
           "Name of the StoreArray holding the 3DHough Finder input tracks.",
           std::string("TRGCDCNDFinderTracks"));
  addParam("outputCollectionName", m_outputCollectionName,
           "Name of the StoreArray holding the output Neuro tracks.",
           std::string("TRGCDC3DHNeuroTracks"));
  addParam("fixedPoint", m_fixedPoint,
           "Switch to turn on fixed point arithmetic for FPGA simulation.",
           false);
  addParam("classificationCutNNT", m_classificationCutNNT,
           "The across the board classification cut for the nnt (y)-bit (between -1 and 1).",
           static_cast<double>(0.0));
  addParam("classificationCutSTT", m_classificationCutSTT,
           "The across the board classification cut for the stt-bit (between -1 and 1, for p > 0.7GeV).",
           static_cast<double>(0.0));
}

void CDCTrigger3DHNeuroModule::initialize()
{
  m_neuroTrigger3DH.initialize();
  const CDCTrigger3DHMLP& mlp = m_fileName.empty()
                                ? *m_CDCTrigger3DHMLPConditionsDB // Load from conditions database
                                : CDCTrigger3DHMLP::loadMLPFromFile<CDCTrigger3DHMLP>(m_fileName, m_arrayName); // Load from custom file
  m_neuroTrigger3DH.setMLP(mlp);

  if (m_fixedPoint) {
    m_neuroTrigger3DH.createIntWeights();
  }
  m_neuroTrigger3DH.initializeCollections(m_hitCollectionName);

  m_ndFinderTracks.isRequired(m_inputCollectionName);
  m_trackSegmentHits.isRequired(m_hitCollectionName);
  m_neuro3DHTracks.registerInDataStore(m_outputCollectionName);

  m_ndFinderTracks.registerRelationTo(m_neuro3DHTracks);
  m_ndFinderTracks.requireRelationTo(m_trackSegmentHits);
  m_neuro3DHTracks.registerRelationTo(m_trackSegmentHits);
}

void CDCTrigger3DHNeuroModule::event()
{
  for (int trackIdx = 0; trackIdx < m_ndFinderTracks.getEntries(); ++trackIdx) {
    // Setup of the MLP input
    if (m_fixedPoint) {
      m_neuroTrigger3DH.calculateTrackParametersFixedPrecision(*m_ndFinderTracks[trackIdx]);
    } else {
      m_neuroTrigger3DH.calculateTrackParameters(*m_ndFinderTracks[trackIdx]);
    }
    m_neuroTrigger3DH.setEventTime(*m_ndFinderTracks[trackIdx]);
    std::vector<size_t> hitIds = m_neuroTrigger3DH.load3DHits(*m_ndFinderTracks[trackIdx]);
    std::vector<float> networkInput = m_neuroTrigger3DH.getInputVector(hitIds);
    // Run the MLP
    std::vector<float> networkPrediction;
    if (m_fixedPoint) {
      networkPrediction = m_neuroTrigger3DH.runMLPFixedPrecision(networkInput);
    } else {
      networkPrediction = m_neuroTrigger3DH.runMLP(networkInput);
    }
    // Create a new track with the MLP output values
    double trackPhi = m_ndFinderTracks[trackIdx]->getPhi0();
    double trackOmega = m_ndFinderTracks[trackIdx]->getOmega();
    double zPrediction = networkPrediction[0];
    double cotPrediction = std::cos(networkPrediction[1]) / std::sin(networkPrediction[1]);
    double classificationNNT = networkPrediction[2];
    double classificationSTT = networkPrediction[3];


    double trackTheta = std::atan2(1.0, cotPrediction);
    if (trackTheta < 0) trackTheta += M_PI;
    double totalMomentumPrediction = m_ndFinderTracks[trackIdx]->getPt() / std::sin(trackTheta);

    // Set the Helix
    CDCTrigger3DHTrack* neuroTrack =
      m_neuro3DHTracks.appendNew(trackPhi, trackOmega, zPrediction, cotPrediction);

    // Set further info
    neuroTrack->setTime(m_neuroTrigger3DH.getEventTime());
    neuroTrack->setFloatInput(networkInput);
    neuroTrack->setTotalMomentum(totalMomentumPrediction);
    neuroTrack->setNNTClassification(classificationNNT);
    neuroTrack->setSTTClassification(classificationSTT);
    neuroTrack->setQuadrant(m_ndFinderTracks[trackIdx]->getQuadrant());
    neuroTrack->setValidTrackBit(m_ndFinderTracks[trackIdx]->getValidTrackBit());

    // Get the trigger bits
    bool nntBit = (classificationNNT > m_classificationCutNNT);
    bool sttBit = (classificationSTT > m_classificationCutSTT);
    neuroTrack->setNNTBit(nntBit);
    neuroTrack->setSTTBit(sttBit);

    // Set relation to 3DFinder track
    m_ndFinderTracks[trackIdx]->addRelationTo(neuroTrack);

    // Set relation to Track Segments and create TSVector
    std::array<unsigned short, 9> tsVector{0};
    for (unsigned int hitIdx = 0; hitIdx < hitIds.size(); ++hitIdx) {
      neuroTrack->addRelationTo(m_trackSegmentHits[hitIds[hitIdx]]);
      unsigned short superLayer = m_trackSegmentHits[hitIds[hitIdx]]->getISuperLayer();
      tsVector[superLayer] = m_trackSegmentHits[hitIds[hitIdx]]->getLeftRight();
    }
    neuroTrack->setTSVector(tsVector);
  }
}
