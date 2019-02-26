#include "trg/cdc/modules/neurotrigger/CDCTriggerNeuroModule.h"

#include <framework/datastore/StoreObjPtr.h>

#include <framework/gearbox/Const.h>

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
           "Name of the files where the NeuroTrigger parameters are saved "
           "(compare NeuroTriggerTrainer).",
           string("NeuroTrigger.root"));
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
           "Name of the StoreArray holding the 2D input tracks.",
           string("TRGCDC2DFinderTracks"));
  addParam("outputCollectionName", m_outputCollectionName,
           "Name of the StoreArray holding the output tracks with neural network estimates.",
           string("TRGCDCNeuroTracks"));
  addParam("fixedPoint", m_fixedPoint,
           "Switch to turn on fixed point arithmetic for FPGA simulation.",
           false);
  addParam("precision", m_precision,
           "fixed point precision in bit after radix point (for track phi, "
           "scaling factor, reference id, MLP nodes, MLP weights, "
           "MLP activation function)", {12, 8, 8, 12, 10, 10});
  addParam("writeMLPinput", m_writeMLPinput,
           "if true, the MLP input vector will be written to the datastore (for DQM)",
           false);
  addParam("alwaysTrackT0", m_alwaysTrackT0,
           "Switch for always using the shortest priority time of the TS as t0.",
           false);
  addParam("hardwareCompatibilityMode", m_hardwareCompatibilityMode,
           "Switch to mimic an apparent bug in the hardware preprocessing",
           false);
}


void
CDCTriggerNeuroModule::initialize()
{
  if (!m_NeuroTrigger.load(m_filename, m_arrayname))
    B2ERROR("NeuroTrigger could not be loaded correctly.");
  if (m_fixedPoint) {
    m_NeuroTrigger.setPrecision(m_precision);
  }

  m_tracksNN.registerInDataStore(m_outputCollectionName);
  m_tracks2D.isRequired(m_inputCollectionName);
  m_segmentHits.isRequired(m_hitCollectionName);
  m_NeuroTrigger.initializeCollections(m_hitCollectionName, m_EventTimeName, m_alwaysTrackT0);

  m_tracks2D.registerRelationTo(m_tracksNN);
  m_tracks2D.requireRelationTo(m_segmentHits);
  m_tracksNN.registerRelationTo(m_segmentHits);
  if (m_writeMLPinput) {
    m_mlpInput.registerInDataStore(m_outputCollectionName + "Input",
                                   DataStore::c_DontWriteOut);
    m_tracksNN.registerRelationTo(m_mlpInput, DataStore::c_Event,
                                  DataStore::c_DontWriteOut);
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
    m_NeuroTrigger.getEventTime(geoSectors[0], *m_tracks2D[itrack], m_alwaysTrackT0);
    // get the hit pattern (depends on phase space sector)
    unsigned long hitPattern =
      m_NeuroTrigger.getInputPattern(geoSectors[0], *m_tracks2D[itrack]);
    // get the MLP that matches the hit pattern
    int isector = m_NeuroTrigger.selectMLPbyPattern(geoSectors, hitPattern);
    if (isector < 0) continue;
    // get the input for the MLP
    vector<unsigned> hitIds = m_NeuroTrigger.selectHits(isector, *m_tracks2D[itrack]);
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
    const CDCTriggerTrack* NNtrack =
      m_tracksNN.appendNew(m_tracks2D[itrack]->getPhi0(),
                           m_tracks2D[itrack]->getOmega(),
                           m_tracks2D[itrack]->getChi2D(),
                           z, cot, 0.);
    m_tracks2D[itrack]->addRelationTo(NNtrack);
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
