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
}


void
NeuroTriggerModule::initialize()
{
  StoreArray<CDCTriggerSegmentHit>::required();
  StoreArray<CDCTriggerTrack>::required();
  if (!m_NeuroTrigger.load(m_filename, m_arrayname))
    B2ERROR("NeuroTrigger could not be loaded correctly.");
}


void
NeuroTriggerModule::event()
{
  StoreArray<CDCTriggerTrack> tracks("CDCTriggerTracks");
  for (int itrack = 0; itrack < tracks.getEntries(); ++itrack) {
    m_NeuroTrigger.updateTrack(*tracks[itrack]);
    int isector = m_NeuroTrigger.selectMLP(*tracks[itrack]);
    if (isector >= 0) {
      std::vector<float> MLPinput = m_NeuroTrigger.getInputVector(isector);
      std::vector<float> target = m_NeuroTrigger.runMLP(isector, MLPinput);
      int zIndex = m_NeuroTrigger[isector].zIndex();
      if (zIndex >= 0) tracks[itrack]->setNNZ(target[zIndex]);
      int thetaIndex = m_NeuroTrigger[isector].thetaIndex();
      if (thetaIndex >= 0) tracks[itrack]->setNNTheta(target[thetaIndex]);
    } else {
      //give general warning (to get only one message in summary)
      B2WARNING("No MLP trained for this track.");
      //give specific information as debug
      B2DEBUG(100, "pt=" << tracks[itrack]->getHoughPt()
              << ", phi=" << tracks[itrack]->getHoughPhiVertex() * 180. / M_PI);
    }
  }
}
