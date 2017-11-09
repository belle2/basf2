#include "trg/cdc/modules/neurotrigger/CDCTriggerNeuroModule.h"

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <framework/dataobjects/EventT0.h>
#include <mdst/dataobjects/MCParticle.h>

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
}


void
CDCTriggerNeuroModule::initialize()
{
  StoreArray<CDCTriggerSegmentHit>::required(m_hitCollectionName);
  StoreArray<CDCTriggerTrack>::required(m_inputCollectionName);
  StoreObjPtr<EventT0>::required(m_EventTimeName);
  if (!m_NeuroTrigger.load(m_filename, m_arrayname))
    B2ERROR("NeuroTrigger could not be loaded correctly.");
  StoreArray<CDCTriggerTrack>::registerPersistent(m_outputCollectionName);

  StoreArray<CDCTriggerTrack> tracks2D(m_inputCollectionName);
  StoreArray<CDCTriggerTrack> tracksNN(m_outputCollectionName);
  tracks2D.registerRelationTo(tracksNN);
  StoreArray<CDCTriggerSegmentHit> segmentHits(m_hitCollectionName);
  tracksNN.registerRelationTo(segmentHits);

  if (m_fixedPoint) {
    m_NeuroTrigger.setPrecision(m_precision);
  }
  m_NeuroTrigger.setCollectionNames(m_hitCollectionName, m_EventTimeName);
}


void
CDCTriggerNeuroModule::event()
{
  StoreArray<CDCTriggerTrack> tracks2D(m_inputCollectionName);
  StoreArray<CDCTriggerTrack> tracksNN(m_outputCollectionName);
  StoreArray<CDCTriggerSegmentHit> segmentHits(m_hitCollectionName);
  for (int itrack = 0; itrack < tracks2D.getEntries(); ++itrack) {
    // calculate parameters that depend only on track
    if (m_fixedPoint) {
      m_NeuroTrigger.updateTrackFix(*tracks2D[itrack]);
    } else {
      m_NeuroTrigger.updateTrack(*tracks2D[itrack]);
    }
    // get all MLPs that match the phase space sector
    vector<int> geoSectors =
      m_NeuroTrigger.selectMLPs(tracks2D[itrack]->getPhi0(),
                                tracks2D[itrack]->getKappa(1.5),
                                atan2(1., tracks2D[itrack]->getCotTheta()));
    if (geoSectors.size() == 0) continue;
    // get the hit pattern (depends on phase space sector)
    unsigned long hitPattern =
      m_NeuroTrigger.getInputPattern(geoSectors[0], *tracks2D[itrack]);
    // get the MLP that matches the hit pattern
    int isector = m_NeuroTrigger.selectMLPbyPattern(geoSectors, hitPattern);
    if (isector < 0) continue;
    // get the input for the MLP
    vector<unsigned> hitIds = m_NeuroTrigger.selectHits(isector, *tracks2D[itrack]);
    vector<float> MLPinput = m_NeuroTrigger.getInputVector(isector, hitIds, *tracks2D[itrack]);
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
      tracksNN.appendNew(tracks2D[itrack]->getPhi0(),
                         tracks2D[itrack]->getOmega(),
                         tracks2D[itrack]->getChi2D(),
                         z, cot, 0.);
    tracks2D[itrack]->addRelationTo(NNtrack);
    // relations to hits used in MLP
    for (unsigned i = 0; i < hitIds.size(); ++i) {
      NNtrack->addRelationTo(segmentHits[hitIds[i]]);
    }
  }
}
