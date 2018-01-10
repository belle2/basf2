#include "trg/cdc/modules/neurotrigger/CDCTriggerNeuroModule.h"

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventT0.h>

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
  if (!m_NeuroTrigger.load(m_filename, m_arrayname))
    B2ERROR("NeuroTrigger could not be loaded correctly.");
  if (m_fixedPoint) {
    m_NeuroTrigger.setPrecision(m_precision);
  }

  m_tracksNN.registerInDataStore(m_outputCollectionName);
  m_tracks2D.isRequired(m_inputCollectionName);
  m_segmentHits.isRequired(m_hitCollectionName);
  m_NeuroTrigger.initializeCollections(m_hitCollectionName, m_EventTimeName);

  m_tracks2D.registerRelationTo(m_tracksNN);
  m_tracks2D.requireRelationTo(m_segmentHits);
  m_tracksNN.registerRelationTo(m_segmentHits);
}


void
CDCTriggerNeuroModule::event()
{
  for (int itrack = 0; itrack < m_tracks2D.getEntries(); ++itrack) {
    if (m_fixedPoint) {
      m_NeuroTrigger.updateTrackFix(*m_tracks2D[itrack]);
    } else {
      m_NeuroTrigger.updateTrack(*m_tracks2D[itrack]);
    }
    int isector = m_NeuroTrigger.selectMLP(*m_tracks2D[itrack]);
    if (isector >= 0) {
      vector<unsigned> hitIds = m_NeuroTrigger.selectHits(isector, *m_tracks2D[itrack]);
      vector<float> MLPinput = m_NeuroTrigger.getInputVector(isector, hitIds);
      vector<float> target;
      if (m_fixedPoint) {
        target = m_NeuroTrigger.runMLPFix(isector, MLPinput);
      } else {
        target = m_NeuroTrigger.runMLP(isector, MLPinput);
      }
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
    }
  }
}
