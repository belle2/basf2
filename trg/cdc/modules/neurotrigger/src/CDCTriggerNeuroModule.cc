/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "trg/cdc/modules/neurotrigger/CDCTriggerNeuroModule.h"

#include <cmath>

using namespace Belle2;
using namespace std;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
REG_MODULE(CDCTriggerNeuro)

CDCTriggerNeuroModule::CDCTriggerNeuroModule() : Module()
{
  setDescription(
    "The NeuroTrigger module of the CDC trigger.\n"
    "Takes track segments and 2D track estimates as input and estimates\n"
    "the z-vertex for each track using a neural network.\n"
    "Requires one or several trained networks stored in a file.\n"
  );
  setPropertyFlags(c_ParallelProcessingCertified);
  // parameters for saving / loading
  addParam("filename", m_filename,
           "Name of the files where the NeuroTrigger parameters are saved. "
           "When left blank, the parameters are loaded from the Conditions "
           "Database."
           "(compare NeuroTriggerTrainer).",
           string(""));
  addParam("arrayname", m_arrayname,
           "Name of the TObjArray holding the NeuroTrigger parameters "
           "(compare NeuroTriggerTrainer).",
           string("MLPs"));
  addParam("hitCollectionName", m_hitCollectionName,
           "Name of the input StoreArray of CDCTriggerSegmentHits.",
           string(""));
  addParam("EventTimeName", m_EventTimeName,
           "Name of the event time object.",
           string(""));
  addParam("inputCollectionName", m_inputCollectionName,
           "Name of the StoreArray holding the 2D input tracks or Neurotracks.",
           string("TRGCDC2DFinderTracks"));
  addParam("realinputCollectionName", m_realinputCollectionName,
           "Name of the StoreArray holding the 2D input tracks in case "
           "Neurotracks were used for the inputCollectionName.",
           string("CDCTriggerNNInput2DFinderTracks"));
  addParam("outputCollectionName", m_outputCollectionName,
           "Name of the StoreArray holding the output tracks with neural "
           "network estimates.",
           string("TRGCDCNeuroTracks"));
  addParam("fixedPoint", m_fixedPoint,
           "Switch to turn on fixed point arithmetic for FPGA simulation.",
           false);
  addParam("precision", m_precision,
           "fixed point precision in bit after radix point (for track phi, "
           "scaling factor, reference id, MLP nodes, MLP weights, "
           "MLP activation function)", {12, 8, 8, 12, 10, 10});
  addParam("et_option", m_et_option,
           "option on how to obtain the event time. When left blank, the value "
           "is loaded from the Conditions Database. Possibilities are: "
           "'etf_only', 'fastestpriority', 'zero', 'etf_or_fastestpriority', "
           "'etf_or_zero', 'etf_or_fastest2d', 'fastest2d'.",
           string(""));
  addParam("writeMLPinput", m_writeMLPinput,
           "if true, the MLP input vector will be written to the datastore "
           "(for DQM)",
           false);
  addParam("hardwareCompatibilityMode", m_hardwareCompatibilityMode,
           "Switch to mimic an apparent bug in the hardware preprocessing",
           false);
  addParam("NeuroHWTrackInputMode", m_neuroTrackInputMode,
           "use Neurotracks instead of 2DTracks as input",
           false);
}


void
CDCTriggerNeuroModule::initialize()
{
  // Load Values from the conditions Database. The actual MLP-values are loaded
  // in the Neurotrigger class itself to avoid bigger changes in the code.
  if (m_et_option.size() < 1) {
    m_et_option = m_cdctriggerneuroconfig->getUseETF() ? "etf_or_fastestpriority" : "fastestpriority";
    B2DEBUG(2, "The firmware version of the Neurotrigger boards is: " + m_cdctriggerneuroconfig->getNNTFirmwareVersionID());
  }
  if (!m_NeuroTrigger.load(m_filename, m_arrayname))
    B2ERROR("NeuroTrigger could not be loaded correctly.");
  if (m_fixedPoint) {
    m_NeuroTrigger.setPrecision(m_precision);
  }
  if (m_et_option == "") {
    m_et_option = m_NeuroTrigger.get_et_option();
  }
  m_tracksNN.registerInDataStore(m_outputCollectionName);
  m_tracks2D.isRequired(m_inputCollectionName);
  m_segmentHits.isRequired(m_hitCollectionName);
  m_NeuroTrigger.initializeCollections(m_hitCollectionName, m_EventTimeName, m_et_option);

  m_tracks2D.registerRelationTo(m_tracksNN);
  m_tracks2D.requireRelationTo(m_segmentHits);
  m_tracksNN.registerRelationTo(m_segmentHits);
  if (m_neuroTrackInputMode) {
    m_realtracks2D.isRequired(m_realinputCollectionName);
    m_realtracks2D.registerRelationTo(m_tracksNN);
  }
  if (m_writeMLPinput) {
    m_mlpInput.registerInDataStore(m_outputCollectionName + "Input");
    m_tracksNN.registerRelationTo(m_mlpInput, DataStore::c_Event);
  }
}

float CDCTriggerNeuroModule::hwInputIdShuffle(float tsid, int sl)
{
  switch (sl) {
    case 8: return tsid + 0.12;
    case 4: return tsid / 2;
    case 3: return tsid - 0.12;
    case 1: return (tsid + 0.12) / 2;
    case 0: return tsid / 4;
    default: return tsid;
  }
}

void
CDCTriggerNeuroModule::event()
{
  for (int itrack = 0; itrack < m_tracks2D.getEntries(); ++itrack) {
    // calculate parameters that depend only on track
    if (m_fixedPoint) {
      m_NeuroTrigger.updateTrackFix(*m_tracks2D[itrack]);
    } else {
      m_NeuroTrigger.updateTrack(*m_tracks2D[itrack]);
    }
    // get all MLPs that match the phase space sector
    vector<int> geoSectors =
      m_NeuroTrigger.selectMLPs(m_tracks2D[itrack]->getPhi0(),
                                m_tracks2D[itrack]->getKappa(1.5),
                                atan2(1., m_tracks2D[itrack]->getCotTheta()));
    if (geoSectors.size() == 0) continue;
    // read out or determine event time
    m_NeuroTrigger.getEventTime(geoSectors[0], *m_tracks2D[itrack], m_et_option, m_neuroTrackInputMode);
    // get the hit pattern (depends on phase space sector)
    unsigned long hitPattern =
      m_NeuroTrigger.getInputPattern(geoSectors[0], *m_tracks2D[itrack], m_neuroTrackInputMode);
    // get the complete hit pattern for debug purposes
    unsigned long chitPattern =
      m_NeuroTrigger.getCompleteHitPattern(geoSectors[0], *m_tracks2D[itrack], m_neuroTrackInputMode);
    // get the pure driftthreshold vector
    unsigned long puredriftth =
      m_NeuroTrigger.getPureDriftThreshold(geoSectors[0], *m_tracks2D[itrack], m_neuroTrackInputMode);
    // get the MLP that matches the hit pattern
    int isector = m_NeuroTrigger.selectMLPbyPattern(geoSectors, hitPattern, m_neuroTrackInputMode);
    if (isector < 0) continue;
    // get the input for the MLP
    vector<unsigned> hitIds;
    if (m_neuroTrackInputMode) {
      hitIds = m_NeuroTrigger.selectHitsHWSim(isector, *m_tracks2D[itrack]);
    } else {
      hitIds = m_NeuroTrigger.selectHits(isector, *m_tracks2D[itrack]);
    }
    vector<float> MLPinput = m_NeuroTrigger.getInputVector(isector, hitIds);
    if (m_hardwareCompatibilityMode) {
      for (unsigned isl = 0; isl < 9; isl++) {
        MLPinput[3 * isl] = hwInputIdShuffle(MLPinput[3 * isl], isl);
      }
    }
    // run the MLP
    vector<float> target;
    if (m_fixedPoint) {
      target = m_NeuroTrigger.runMLPFix(isector, MLPinput);
    } else {
      target = m_NeuroTrigger.runMLP(isector, MLPinput);
    }
    // create a new track with the MLP output values
    int zIndex = m_NeuroTrigger[isector].zIndex();
    double z = (zIndex >= 0) ? target[zIndex] : 0.;
    int thetaIndex = m_NeuroTrigger[isector].thetaIndex();
    double cot = (thetaIndex >= 0) ? cos(target[thetaIndex]) / sin(target[thetaIndex]) : 0.;
    bool valtrack = (m_neuroTrackInputMode) ? m_tracks2D[itrack]->getValidStereoBit() : true;
    std::vector<bool> tsvector;
    for (int k = 0; k < 9; k++) {
      tsvector.push_back(bool ((chitPattern & (1 << k)) >> k));
    }
    tsvector = (m_neuroTrackInputMode) ? m_tracks2D[itrack]->getTSVector() : tsvector;
    std::vector<bool> driftthreshold;
    for (int k = 8; k >= 0; k--) {
      driftthreshold.push_back(!tsvector[k] || static_cast<bool>((puredriftth & (1 << k)) >> k));
    }
    int expert = (m_neuroTrackInputMode) ? m_tracks2D[itrack]->getExpert() : isector;
    short quadrant = 0;
    double tphi = m_tracks2D[itrack]->getPhi0();
    if (tphi > -1 * M_PI_4 && tphi <  1 * M_PI_4) { quadrant = 0; }
    else if (tphi >  1 * M_PI_4 && tphi <  3 * M_PI_4) { quadrant = 1; }
    else if (tphi >  3 * M_PI_4 || tphi < -3 * M_PI_4) { quadrant = 2; }
    else if (tphi > -3 * M_PI_4 && tphi < -1 * M_PI_4) { quadrant = 3; }





    const CDCTriggerTrack* NNtrack =
      m_tracksNN.appendNew(m_tracks2D[itrack]->getPhi0(),
                           m_tracks2D[itrack]->getOmega(),
                           m_tracks2D[itrack]->getChi2D(),
                           z, cot, 0.,
                           m_tracks2D[itrack]->getFoundOldTrack(),
                           driftthreshold,
                           valtrack,
                           expert,
                           tsvector,
                           m_tracks2D[itrack]->getTime(),
                           quadrant //quadrant simulated from phi
                           - 1, //quadrant not known in simulation
                           m_tracks2D[itrack]->getQualityVector()
                          );
    m_tracks2D[itrack]->addRelationTo(NNtrack);
    if (m_neuroTrackInputMode) {
      m_tracks2D[itrack]->getRelatedFrom<CDCTriggerTrack>(m_realinputCollectionName)->addRelationTo(NNtrack);
    }
    // relations to hits used in MLP
    for (unsigned i = 0; i < hitIds.size(); ++i) {
      NNtrack->addRelationTo(m_segmentHits[hitIds[i]]);
    }
    if (m_writeMLPinput) {
      // for fixed point precision, round the inputs before saving
      if (m_fixedPoint) {
        for (unsigned ii = 0; ii < MLPinput.size(); ++ii) {
          MLPinput[ii] = long(MLPinput[ii] * (1 << m_precision[3])) / float(1 << m_precision[3]);
        }
      }
      auto* storeInput = m_mlpInput.appendNew(MLPinput, unsigned(isector));
      NNtrack->addRelationTo(storeInput);
    }
  }
}
