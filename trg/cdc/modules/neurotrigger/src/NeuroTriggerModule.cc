#include "trg/cdc/modules/neurotrigger/NeuroTriggerModule.h"

#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/gearbox/Const.h>

#include <cmath>

using namespace Belle2;
using namespace std;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
REG_MODULE(NeuroTrigger)

NeuroTriggerModule::NeuroTriggerModule() : Module()
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
  addParam("inputCollection", m_inputCollectionName,
           "Name of the StoreArray holding the 2D input tracks.",
           string("Trg2DFinderTracks"));
  addParam("outputCollection", m_outputCollectionName,
           "Name of the StoreArray holding the output tracks with neural network estimates.",
           string("TrgNNTracks"));
}


void
NeuroTriggerModule::initialize()
{
  StoreArray<CDCTriggerSegmentHit>::required();
  StoreArray<CDCTriggerTrack>::required(m_inputCollectionName);
  if (!m_NeuroTrigger.load(m_filename, m_arrayname))
    B2ERROR("NeuroTrigger could not be loaded correctly.");
  StoreArray<CDCTriggerTrack>::registerPersistent(m_outputCollectionName);

  StoreArray<CDCTriggerTrack> tracks2D(m_inputCollectionName);
  StoreArray<CDCTriggerTrack> tracksNN(m_outputCollectionName);
  tracksNN.registerRelationTo(tracks2D);
  StoreArray<CDCTriggerSegmentHit> segmentHits;
  tracksNN.registerRelationTo(segmentHits);
}


void
NeuroTriggerModule::event()
{
  StoreArray<CDCTriggerTrack> tracks2D(m_inputCollectionName);
  StoreArray<CDCTriggerTrack> tracksNN(m_outputCollectionName);
  StoreArray<CDCTriggerSegmentHit> segmentHits;
  for (int itrack = 0; itrack < tracks2D.getEntries(); ++itrack) {
    m_NeuroTrigger.updateTrack(*tracks2D[itrack]);
    int isector = m_NeuroTrigger.selectMLP(*tracks2D[itrack]);
    if (isector >= 0) {
      std::vector<float> MLPinput = m_NeuroTrigger.getInputVector(isector);
      std::vector<float> target = m_NeuroTrigger.runMLP(isector, MLPinput);
      int zIndex = m_NeuroTrigger[isector].zIndex();
      double z = (zIndex >= 0) ? target[zIndex] : 0.;
      int thetaIndex = m_NeuroTrigger[isector].thetaIndex();
      double cot = (thetaIndex >= 0) ? cos(target[thetaIndex]) / sin(target[thetaIndex]) : 0.;
      const CDCTriggerTrack* NNtrack =
        tracksNN.appendNew(tracks2D[itrack]->getPhi0(),
                           tracks2D[itrack]->getOmega(),
                           tracks2D[itrack]->getChi2D(),
                           z, cot, 0.);
      NNtrack->addRelationTo(tracks2D[itrack]);
      // relations to hits used in MLP
      std::vector<unsigned> hitIds = m_NeuroTrigger.getSelectedHitIds();
      for (unsigned i = 0; i < hitIds.size(); ++i) {
        NNtrack->addRelationTo(segmentHits[hitIds[i]]);
      }
    } else {
      B2WARNING("No MLP trained for this track.");
    }
  }
}
