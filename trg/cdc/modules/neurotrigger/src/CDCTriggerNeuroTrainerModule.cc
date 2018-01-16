#include "trg/cdc/modules/neurotrigger/CDCTriggerNeuroTrainerModule.h"
#ifdef HAS_OPENMP
#include <parallel_fann.hpp>
#else
#include <fann.h>
#endif

#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/core/ModuleParam.templateDetails.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>

#include <fstream>
#include <cmath>
#include <TFile.h>

using namespace Belle2;
using namespace std;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
REG_MODULE(CDCTriggerNeuroTrainer)

CDCTriggerNeuroTrainerModule::CDCTriggerNeuroTrainerModule() : Module()
{
  setDescription(
    "The NeuroTriggerTrainer module of the CDC trigger.\n"
    "Takes track segments and 2D track estimates to prepare input data\n"
    "for the training of a neural network.\n"
    "Networks are trained after the event loop and saved.\n\n"
    "Data preparation is done in two steps:\n"
    "1. The MLP uses hits from a limited range around the 2D track. "
    "To find this range, a histogram with the distance of hits to the 2D track "
    "is prepared. The relevant ID range is determined by a threshold on "
    "the hit counters or on the sum of the hit counters over the relevant range.\n"
    "2. Input data is calculated from the hits, the 2D tracks and the ID ranges. "
    "Target data is collected from a MCParticle or RecoTrack related to the 2D track."
  );
  // parameters for saving / loading
  addParam("hitCollectionName", m_hitCollectionName,
           "Name of the input StoreArray of CDCTriggerSegmentHits.",
           string(""));
  addParam("EventTimeName", m_EventTimeName,
           "Name of the event time object.",
           string(""));
  addParam("inputCollectionName", m_inputCollectionName,
           "Name of the StoreArray holding the 2D input tracks.",
           string("TRGCDC2DFinderTracks"));
  addParam("trainOnRecoTracks", m_trainOnRecoTracks,
           "If true, use RecoTracks as targets instead of MCParticles.",
           false);
  addParam("targetCollectionName", m_targetCollectionName,
           "Name of the MCParticle/RecoTrack collection used as target values.",
           string("MCParticles"));
  addParam("filename", m_filename,
           "Name of the root file where the NeuroTrigger parameters will be saved.",
           string("NeuroTrigger.root"));
  addParam("trainFilename", m_trainFilename,
           "Name of the root file where the generated training samples will be saved.",
           string("NeuroTrigger.root"));
  addParam("logFilename", m_logFilename,
           "Base name of the text files where the training logs will be saved "
           "(two for each sector, named logFilename_BestRun_i.log "
           "and logFilename_AllOptima_i.log).",
           string("NeuroTrigger"));
  addParam("arrayname", m_arrayname,
           "Name of the TObjArray to hold the NeuroTrigger parameters.",
           string("MLPs"));
  addParam("trainArrayname", m_trainArrayname,
           "Name of the TObjArray to hold the training samples.",
           string("trainSets"));
  addParam("saveDebug", m_saveDebug,
           "If true, save parameter distribution of training data "
           "in train file and training curve in log file.", true);
  addParam("load", m_load,
           "Switch to load saved parameters if existing. "
           "Take care not to duplicate training sets!", false);
  // NeuroTrigger parameters
  addParam("nMLP", m_parameters.nMLP,
           "Number of expert MLPs.", m_parameters.nMLP);
  addParam("nHidden", m_parameters.nHidden,
           "Number of nodes in each hidden layer for all networks "
           "or factor to multiply with number of inputs (1 list or nMLP lists). "
           "The number of layers is derived from the shape.", m_parameters.nHidden);
  addParam("multiplyHidden", m_parameters.multiplyHidden,
           "If true, multiply nHidden with number of input nodes.",
           m_parameters.multiplyHidden);
  addParam("targetZ", m_parameters.targetZ,
           "Train one output of MLP to give z.", m_parameters.targetZ);
  addParam("targetTheta", m_parameters.targetTheta,
           "Train one output of MLP to give theta.", m_parameters.targetTheta);
  addParam("outputScale", m_parameters.outputScale,
           "Output scale for all networks (1 value list or nMLP value lists). "
           "Output[i] of the MLP is scaled from [-1, 1] "
           "to [outputScale[2*i], outputScale[2*i+1]]. "
           "(units: z[cm] / theta[degree])", m_parameters.outputScale);
  addParam("phiRange", m_parameters.phiRange,
           "Phi region in degree for which experts are trained. "
           "1 value pair, nMLP value pairs or nPhi value pairs "
           "with nPhi * nPt * nTheta * nPattern = nMLP.", m_parameters.phiRange);
  addParam("invptRange", m_parameters.invptRange,
           "Charge / Pt region in 1/GeV for which experts are trained. "
           "1 value pair, nMLP value pairs or nPt value pairs "
           "with nPhi * nPt * nTheta * nPattern = nMLP.", m_parameters.invptRange);
  addParam("thetaRange", m_parameters.thetaRange,
           "Theta region in degree for which experts are trained. "
           "1 value pair, nMLP value pairs or nTheta value pairs "
           "with nPhi * nPt * nTheta * nPattern = nMLP.", m_parameters.thetaRange);
  addParam("phiRangeTrain", m_parameters.phiRangeTrain,
           "Phi region in degree from which training events are taken. "
           "Can be larger than phiRange to avoid edge effect.", m_parameters.phiRangeTrain);
  addParam("invptRangeTrain", m_parameters.invptRangeTrain,
           "Charge / Pt region in 1/GeV from which training events are taken. "
           "Can be larger than phiRange to avoid edge effect.", m_parameters.invptRangeTrain);
  addParam("thetaRangeTrain", m_parameters.thetaRangeTrain,
           "Theta region in degree from which training events are taken. "
           "Can be larger than phiRange to avoid edge effect.", m_parameters.thetaRangeTrain);
  addParam("maxHitsPerSL", m_parameters.maxHitsPerSL,
           "Maximum number of hits in a single SL. "
           "1 value or same as SLpattern.", m_parameters.maxHitsPerSL);
  addParam("SLpattern", m_parameters.SLpattern,
           "Super layer pattern for which experts are trained. "
           "1 value, nMLP values or nPattern values "
           "with nPhi * nPt * nTheta * nPattern = nMLP.", m_parameters.SLpattern);
  addParam("SLpatternMask", m_parameters.SLpatternMask,
           "Super layer pattern mask for which experts are trained. "
           "1 value or same as SLpattern.", m_parameters.SLpatternMask);
  addParam("tMax", m_parameters.tMax,
           "Maximal drift time (for scaling, unit: trigger timing bins).", m_parameters.tMax);
  addParam("selectSectorByMC", m_selectSectorByMC,
           "If true, track parameters for sector selection are taken "
           "from MCParticle instead of CDCTriggerTrack.", false);
  // parameters for training data preparation
  addParam("nTrainPrepare", m_nTrainPrepare,
           "Number of samples for preparation of relevant ID ranges "
           "(0: use default ranges).", 1000);
  addParam("IDranges", m_IDranges,
           "If list is not empty, it will replace the default ranges. "
           "1 list or nMLP lists. Set nTrainPrepare to 0 if you use this option.",
           {});
  addParam("relevantCut", m_relevantCut,
           "Cut for preparation of relevant ID ranges.", 0.02);
  addParam("cutSum", m_cutSum,
           "If true, relevantCut is applied to the sum over hit counters, "
           "otherwise directly on the hit counters.", false);
  addParam("nTrainMin", m_nTrainMin,
           "Minimal number of training samples "
           "or factor to multiply with number of weights. "
           "If the minimal number of samples is not reached, "
           "all samples are saved but no training is started.", 10.);
  addParam("nTrainMax", m_nTrainMax,
           "Maximal number of training samples "
           "or factor to multiply with number of weights. "
           "When the maximal number of samples is reached, "
           "no further samples are added.", 10.);
  addParam("multiplyNTrain", m_multiplyNTrain,
           "If true, multiply nTrainMin and nTrainMax with number of weights.",
           true);
  addParam("nValid", m_nValid,
           "Number of validation samples for training.", 1000);
  addParam("nTest", m_nTest,
           "Number of test samples to get resolution after training.", 5000);
  addParam("stopLoop", m_stopLoop,
           "If true, stop event loop when maximal number of samples "
           "is reached for all sectors.", true);
  addParam("rescaleTarget", m_rescaleTarget,
           "If true, set target values > outputScale to 1, "
           "else skip them.", true);
  // parameters for training
  addParam("wMax", m_wMax,
           "Weights are limited to [-wMax, wMax] after each training epoch "
           "(for convenience of the FPGA implementation).",
           63.);
  addParam("nThreads", m_nThreads,
           "Number of threads for parallel training.", 1);
  addParam("checkInterval", m_checkInterval,
           "Training is stopped if validation error is higher than "
           "checkInterval epochs ago, i.e. either the validation error is increasing "
           "or the gain is less than the fluctuations.", 500);
  addParam("maxEpochs", m_maxEpochs,
           "Maximum number of training epochs.", 10000);
  addParam("repeatTrain", m_repeatTrain,
           "If >1, training is repeated several times with different start weights. "
           "The weights which give the best resolution on the test samples are kept.", 1);
}


void
CDCTriggerNeuroTrainerModule::initialize()
{
  // register store objects
  m_tracks.isRequired(m_inputCollectionName);
  if (m_trainOnRecoTracks) {
    StoreArray<RecoTrack> targets(m_targetCollectionName);
    targets.isRequired(m_targetCollectionName);
  } else {
    StoreArray<MCParticle> targets(m_targetCollectionName);
    targets.isRequired(m_targetCollectionName);
  }
  // load or initialize neurotrigger
  if (!m_load ||
      !loadTraindata(m_trainFilename, m_trainArrayname) ||
      !m_NeuroTrigger.load(m_filename, m_arrayname)) {
    m_NeuroTrigger.initialize(m_parameters);
    m_trainSets.clear();
    CDC::CDCGeometryPar& cdc = CDC::CDCGeometryPar::Instance();
    for (unsigned iMLP = 0; iMLP < m_NeuroTrigger.nSectors(); ++iMLP) {
      m_trainSets.push_back(CDCTriggerMLPData());
      int layerId = 3;
      for (int iSL = 0; iSL < 9; ++iSL) {
        m_trainSets[iMLP].addCounters(cdc.nWiresInLayer(layerId));
        layerId += (iSL > 0 ? 6 : 7);
      }
    }
  }
  m_NeuroTrigger.initializeCollections(m_hitCollectionName, m_EventTimeName);
  // consistency check of training parameters
  if (m_NeuroTrigger.nSectors() != m_trainSets.size())
    B2ERROR("Number of training sets (" << m_trainSets.size() << ") should match " <<
            "number of sectors (" << m_NeuroTrigger.nSectors() << ")");
  if (m_nTrainMin > m_nTrainMax) {
    m_nTrainMin = m_nTrainMax;
    B2WARNING("nTrainMin set to " << m_nTrainMin << " (was larger than nTrainMax)");
  }
  // set IDranges if they were given in the parameters
  if (m_IDranges.size() > 0) {
    if (m_IDranges.size() == 1 || m_IDranges.size() == m_NeuroTrigger.nSectors()) {
      B2DEBUG(50, "Setting relevant ID ranges from parameters.");
      for (unsigned isector = 0; isector < m_NeuroTrigger.nSectors(); ++isector) {
        unsigned iranges = (m_IDranges.size() == 1) ? 0 : isector;
        if (m_IDranges[iranges].size() == 18)
          m_NeuroTrigger[isector].relevantID = m_IDranges[iranges];
        else
          B2ERROR("IDranges must contain 18 values (sector " << isector
                  << " has " << m_IDranges[iranges].size() << ")");
      }
      if (m_nTrainPrepare > 0)
        B2WARNING("Given ID ranges will be replaced during training. "
                  "Set nTrainPrepare = 0 if you want to give ID ranges by hand.");
    } else {
      B2ERROR("Number of IDranges should be 0, 1, or " << m_NeuroTrigger.nSectors());
    }
  }

  // initialize monitoring histograms
  if (m_saveDebug) {
    for (unsigned iMLP = 0; iMLP < m_NeuroTrigger.nSectors(); ++iMLP) {
      phiHistsMC.push_back(
        new TH1D(("phiMC" + to_string(iMLP)).c_str(),
                 ("MC phi in sector " + to_string(iMLP)).c_str(),
                 100, -2 * M_PI, 2 * M_PI));
      ptHistsMC.push_back(
        new TH1D(("ptMC" + to_string(iMLP)).c_str(),
                 ("MC charge / pt in sector " + to_string(iMLP)).c_str(),
                 100, -5., 5.));
      thetaHistsMC.push_back(
        new TH1D(("thetaMC" + to_string(iMLP)).c_str(),
                 ("MC theta in sector " + to_string(iMLP)).c_str(),
                 100, 0., M_PI));
      zHistsMC.push_back(
        new TH1D(("zMC" + to_string(iMLP)).c_str(),
                 ("MC z in sector " + to_string(iMLP)).c_str(),
                 200, -100., 100.));
      phiHists2D.push_back(
        new TH1D(("phi2D" + to_string(iMLP)).c_str(),
                 ("2D phi in sector " + to_string(iMLP)).c_str(),
                 100, -2 * M_PI, 2 * M_PI));
      ptHists2D.push_back(
        new TH1D(("pt2D" + to_string(iMLP)).c_str(),
                 ("2D charge / pt in sector " + to_string(iMLP)).c_str(),
                 100, -5., 5.));
    }
  }
}

void
CDCTriggerNeuroTrainerModule::event()
{
  for (int itrack = 0; itrack < m_tracks.getEntries(); ++itrack) {
    // get related MCParticle/RecoTrack for target
    // and retrieve track parameters
    float phi0Target = 0;
    float invptTarget = 0;
    float thetaTarget = 0;
    float zTarget = 0;
    if (m_trainOnRecoTracks) {
      RecoTrack* recoTrack =
        m_tracks[itrack]->getRelatedTo<RecoTrack>(m_targetCollectionName);
      if (!recoTrack) {
        B2DEBUG(150, "Skipping CDCTriggerTrack without relation to RecoTrack.");
        continue;
      }
      // a RecoTrack has multiple representations for different particle hypothesis
      // -> just take the first one that does not give errors.
      const vector<genfit::AbsTrackRep*>& reps = recoTrack->getRepresentations();
      bool foundValidRep = false;
      for (unsigned irep = 0; irep < reps.size() && !foundValidRep; ++irep) {
        if (!recoTrack->wasFitSuccessful(reps[irep]))
          continue;
        // get state (position, momentum etc.) from hit closest to IP and
        // extrapolate to z-axis (may throw an exception -> continue to next representation)
        try {
          genfit::MeasuredStateOnPlane state =
            recoTrack->getMeasuredStateOnPlaneClosestTo(TVector3(0, 0, 0), reps[irep]);
          TVector3 closestPos = state.getPos();
          reps[irep]->extrapolateToLine(state, TVector3(0, 0, -1000), TVector3(0, 0, 2000));
          // flip tracks if necessary, such that all track go outward from the IP
          if (state.getMom().Dot(closestPos - state.getPos()) < 0) {
            state.setPosMom(state.getPos(), -state.getMom());
            state.setChargeSign(-state.getCharge());
          }
          // get track parameters
          phi0Target = state.getMom().Phi();
          invptTarget = state.getCharge() / state.getMom().Pt();
          thetaTarget = state.getMom().Theta();
          zTarget = state.getPos().Z();
        } catch (...) {
          continue;
        }
        // break loop
        foundValidRep = true;
      }
      if (!foundValidRep) {
        B2DEBUG(150, "No valid representation found for RecoTrack, skipping.");
        continue;
      }
    } else {
      MCParticle* mcTrack =
        m_tracks[itrack]->getRelatedTo<MCParticle>(m_targetCollectionName);
      if (!mcTrack) {
        B2DEBUG(150, "Skipping CDCTriggerTrack without relation to MCParticle.");
        continue;
      }
      phi0Target = mcTrack->getMomentum().Phi();
      invptTarget = mcTrack->getCharge() / mcTrack->getMomentum().Pt();
      thetaTarget = mcTrack->getMomentum().Theta();
      zTarget = mcTrack->getProductionVertex().Z();
    }

    // update 2D track variables
    m_NeuroTrigger.updateTrack(*m_tracks[itrack]);

    // find all matching sectors
    float phi0 = m_tracks[itrack]->getPhi0();
    float invpt = m_tracks[itrack]->getKappa(1.5);
    float theta = atan2(1., m_tracks[itrack]->getCotTheta());
    if (m_selectSectorByMC) {
      phi0 = phi0Target;
      invpt = invptTarget;
      theta = thetaTarget;
    }
    vector<int> sectors = m_NeuroTrigger.selectMLPs(phi0, invpt, theta);
    if (sectors.size() == 0) continue;
    // get target values
    vector<float> targetRaw = {};
    if (m_parameters.targetZ)
      targetRaw.push_back(zTarget);
    if (m_parameters.targetTheta)
      targetRaw.push_back(thetaTarget);
    for (unsigned i = 0; i < sectors.size(); ++i) {
      int isector = sectors[i];
      vector<float> target = m_NeuroTrigger[isector].scaleTarget(targetRaw);
      // skip out of range targets or rescale them
      bool outOfRange = false;
      for (unsigned itarget = 0; itarget < target.size(); ++itarget) {
        if (fabs(target[itarget]) > 1.) {
          outOfRange = true;
          target[itarget] /= fabs(target[itarget]);
        }
      }
      if (!m_rescaleTarget && outOfRange) continue;

      if (m_nTrainPrepare > 0 &&
          m_trainSets[isector].getTrackCounter() < m_nTrainPrepare) {
        // get relative ids for all hits related to the MCParticle / RecoTrack
        // and count them to find relevant id range
        // using only related hits suppresses background EXCEPT for curling tracks
        if (m_trainOnRecoTracks) {
          RecoTrack* recoTrack =
            m_tracks[itrack]->getRelatedTo<RecoTrack>(m_targetCollectionName);
          for (const CDCTriggerSegmentHit& hit :
               recoTrack->getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName)) {
            // get relative id
            double relId = m_NeuroTrigger.getRelId(hit);
            m_trainSets[isector].addHit(hit.getISuperLayer(), round(relId));
          }
        } else {
          MCParticle* mcTrack =
            m_tracks[itrack]->getRelatedTo<MCParticle>(m_targetCollectionName);
          for (const CDCTriggerSegmentHit& hit :
               mcTrack->getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName)) {
            // get relative id
            double relId = m_NeuroTrigger.getRelId(hit);
            m_trainSets[isector].addHit(hit.getISuperLayer(), round(relId));
          }
        }
        m_trainSets[isector].countTrack();
        // if required hit number is reached, get relevant ids
        if (m_trainSets[isector].getTrackCounter() >= m_nTrainPrepare) {
          updateRelevantID(isector);
        }
      } else {
        // check whether we already have enough samples
        float nTrainMax = m_multiplyNTrain ? m_nTrainMax * m_NeuroTrigger[isector].nWeights() : m_nTrainMax;
        if (m_trainSets[isector].nSamples() > (nTrainMax + m_nValid + m_nTest)) {
          continue;
        }
        // check hit pattern
        unsigned long hitPattern = m_NeuroTrigger.getInputPattern(isector, *m_tracks[itrack]);
        unsigned long sectorPattern = m_NeuroTrigger[isector].getSLpattern();
        B2DEBUG(250, "hitPattern " << hitPattern << " sectorPattern " << sectorPattern);
        if (sectorPattern > 0 && (sectorPattern & hitPattern) != sectorPattern) {
          B2DEBUG(250, "hitPattern not matching " << (sectorPattern & hitPattern));
          continue;
        }
        // get training data
        vector<unsigned> hitIds = m_NeuroTrigger.selectHits(isector, *m_tracks[itrack]);
        m_trainSets[isector].addSample(m_NeuroTrigger.getInputVector(isector, hitIds), target);
        if (m_saveDebug) {
          phiHistsMC[isector]->Fill(phi0Target);
          ptHistsMC[isector]->Fill(invptTarget);
          thetaHistsMC[isector]->Fill(thetaTarget);
          zHistsMC[isector]->Fill(zTarget);
          phiHists2D[isector]->Fill(m_tracks[itrack]->getPhi0());
          ptHists2D[isector]->Fill(m_tracks[itrack]->getKappa(1.5));
        }
        if (m_trainSets[isector].nSamples() % 1000 == 0) {
          B2DEBUG(50, m_trainSets[isector].nSamples() << " samples collected for sector " << isector);
        }
      }
    }
  }
  // check number of samples for all sectors
  if (m_stopLoop) {
    bool stop = true;
    for (unsigned isector = 0; isector < m_trainSets.size(); ++isector) {
      float nTrainMax = m_multiplyNTrain ? m_nTrainMax * m_NeuroTrigger[isector].nWeights() : m_nTrainMax;
      if (m_trainSets[isector].nSamples() < (nTrainMax + m_nValid + m_nTest)) {
        stop = false;
        break;
      }
    }
    if (stop) {
      B2INFO("Training sample preparation for NeuroTrigger finished, stopping event loop.");
      StoreObjPtr<EventMetaData> eventMetaData;
      eventMetaData->setEndOfData();
    }
  }
}

void
CDCTriggerNeuroTrainerModule::terminate()
{
  // save the training data
  saveTraindata(m_trainFilename, m_trainArrayname);
  // do training for all sectors with sufficient training samples
  for (unsigned isector = 0; isector < m_NeuroTrigger.nSectors(); ++isector) {
    // skip sectors that have already been trained
    if (m_NeuroTrigger[isector].isTrained())
      continue;
    float nTrainMin = m_multiplyNTrain ? m_nTrainMin * m_NeuroTrigger[isector].nWeights() : m_nTrainMin;
    if (m_trainSets[isector].nSamples() < (nTrainMin + m_nValid + m_nTest)) {
      B2WARNING("Not enough training samples for sector " << isector << " (" << (nTrainMin + m_nValid + m_nTest)
                << " requested, " << m_trainSets[isector].nSamples() << " found)");
      continue;
    }
    train(isector);
    m_NeuroTrigger[isector].trained = true;
    // set sector ranges
    vector<unsigned> indices = m_NeuroTrigger.getRangeIndices(m_parameters, isector);
    vector<float> phiRange = m_parameters.phiRange[indices[0]];
    vector<float> invptRange = m_parameters.invptRange[indices[1]];
    vector<float> thetaRange = m_parameters.thetaRange[indices[2]];
    //convert phi and theta from degree to radian
    phiRange[0] *= Unit::deg;
    phiRange[1] *= Unit::deg;
    thetaRange[0] *= Unit::deg;
    thetaRange[1] *= Unit::deg;
    m_NeuroTrigger[isector].phiRange = phiRange;
    m_NeuroTrigger[isector].invptRange = invptRange;
    m_NeuroTrigger[isector].thetaRange = thetaRange;
    // save all networks (including the newly trained)
    m_NeuroTrigger.save(m_filename, m_arrayname);
  }
}

void
CDCTriggerNeuroTrainerModule::updateRelevantID(unsigned isector)
{
  B2DEBUG(50, "Setting relevant ID ranges for sector " << isector);
  vector<float> relevantID;
  relevantID.assign(18, 0.);
  CDC::CDCGeometryPar& cdc = CDC::CDCGeometryPar::Instance();
  int layerId = 3;
  for (unsigned iSL = 0; iSL < 9; ++iSL) {
    int nWires = cdc.nWiresInLayer(layerId);
    layerId += (iSL > 0 ? 6 : 7);
    B2DEBUG(90, "SL " << iSL << " (" <<  nWires << " wires)");
    // get maximum hit counter
    unsigned maxCounter = 0;
    int maxId = 0;
    unsigned counterSum = 0;
    for (int iTS = 0; iTS < nWires; ++iTS) {
      if (m_trainSets[isector].getHitCounter(iSL, iTS) > 0)
        B2DEBUG(90, iTS << " " << m_trainSets[isector].getHitCounter(iSL, iTS));
      if (m_trainSets[isector].getHitCounter(iSL, iTS) > maxCounter) {
        maxCounter = m_trainSets[isector].getHitCounter(iSL, iTS);
        maxId = iTS;
      }
      counterSum += m_trainSets[isector].getHitCounter(iSL, iTS);
    }
    // use maximum as starting range
    if (maxId > nWires / 2) maxId -= nWires;
    relevantID[2 * iSL] = maxId;
    relevantID[2 * iSL + 1] = maxId;
    if (m_cutSum) {
      // add neighboring wire with higher hit count
      // until sum over unused wires is less than relevantCut * sum over all wires
      double cut = m_relevantCut * counterSum;
      B2DEBUG(50, "Threshold on counterSum: " << cut);
      unsigned relevantSum = maxCounter;
      while (counterSum - relevantSum > cut) {
        int prev = m_trainSets[isector].getHitCounter(iSL, relevantID[2 * iSL] - 1);
        int next = m_trainSets[isector].getHitCounter(iSL, relevantID[2 * iSL + 1] + 1);
        if (prev > next ||
            (prev == next &&
             (relevantID[2 * iSL + 1] - maxId) > (maxId - relevantID[2 * iSL]))) {
          --relevantID[2 * iSL];
          relevantSum += prev;
          if (relevantID[2 * iSL] <= -nWires) break;
        } else {
          ++relevantID[2 * iSL + 1];
          relevantSum += next;
          if (relevantID[2 * iSL + 1] >= nWires - 1) break;
        }
      }
    } else {
      // add wires from both sides until hit counter drops below relevantCut * track counter
      double cut = m_relevantCut * m_trainSets[isector].getTrackCounter();
      B2DEBUG(50, "Threshold on counter: " << cut);
      while (m_trainSets[isector].getHitCounter(iSL, relevantID[2 * iSL] - 1) > cut) {
        --relevantID[2 * iSL];
        if (relevantID[2 * iSL] <= -nWires) break;
      }
      while (m_trainSets[isector].getHitCounter(iSL, relevantID[2 * iSL + 1] + 1) > cut) {
        ++relevantID[2 * iSL + 1];
        if (relevantID[2 * iSL + 1] >= nWires - 1) break;
      }
    }
    // add +-0.5 to account for rounding during preparation
    relevantID[2 * iSL] -= 0.5;
    relevantID[2 * iSL + 1] += 0.5;
    B2DEBUG(50, "SL " << iSL << ": "
            << relevantID[2 * iSL] << " " << relevantID[2 * iSL + 1]);
  }
  m_NeuroTrigger[isector].relevantID = relevantID;
}

void
CDCTriggerNeuroTrainerModule::train(unsigned isector)
{
#ifdef HAS_OPENMP
  B2INFO("Training network for sector " << isector << " with OpenMP");
#else
  B2INFO("Training network for sector " << isector << " without OpenMP");
#endif
  // initialize network
  unsigned nLayers = m_NeuroTrigger[isector].nLayers();
  unsigned* nNodes = new unsigned[nLayers];
  for (unsigned il = 0; il < nLayers; ++il) {
    nNodes[il] = m_NeuroTrigger[isector].nNodesLayer(il);
  }
  struct fann* ann = fann_create_standard_array(nLayers, nNodes);
  // initialize training and validation data
  CDCTriggerMLPData currentData = m_trainSets[isector];
  // train set
  unsigned nTrain = m_trainSets[isector].nSamples() - m_nValid - m_nTest;
  struct fann_train_data* train_data =
    fann_create_train(nTrain, nNodes[0], nNodes[nLayers - 1]);
  for (unsigned i = 0; i < nTrain; ++i) {
    vector<float> input = currentData.getInput(i);
    for (unsigned j = 0; j < input.size(); ++j) {
      train_data->input[i][j] = input[j];
    }
    vector<float> target = currentData.getTarget(i);
    for (unsigned j = 0; j < target.size(); ++j) {
      train_data->output[i][j] = target[j];
    }
  }
  // validation set
  struct fann_train_data* valid_data =
    fann_create_train(m_nValid, nNodes[0], nNodes[nLayers - 1]);
  for (unsigned i = nTrain; i < nTrain + m_nValid; ++i) {
    vector<float> input = currentData.getInput(i);
    for (unsigned j = 0; j < input.size(); ++j) {
      valid_data->input[i - nTrain][j] = input[j];
    }
    vector<float> target = currentData.getTarget(i);
    for (unsigned j = 0; j < target.size(); ++j) {
      valid_data->output[i - nTrain][j] = target[j];
    }
  }
  // set network parameters
  fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
  fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);
  fann_set_training_algorithm(ann, FANN_TRAIN_RPROP);
  double bestRMS = 999.;
  // keep full train error curve for best run
  vector<double> bestTrainLog = {};
  vector<double> bestValidLog = {};
  // keep train error of optimum for all runs
  vector<double> trainOptLog = {};
  vector<double> validOptLog = {};
  // repeat training several times with different random start weights
  for (int irun = 0; irun < m_repeatTrain; ++irun) {
    double bestValid = 999.;
    vector<double> trainLog = {};
    vector<double> validLog = {};
    trainLog.assign(m_maxEpochs, 0.);
    validLog.assign(m_maxEpochs, 0.);
    int breakEpoch = 0;
    int bestEpoch = 0;
    vector<fann_type> bestWeights = {};
    bestWeights.assign(m_NeuroTrigger[isector].nWeights(), 0.);
    fann_randomize_weights(ann, -0.1, 0.1);
    // train and save the network
    for (int epoch = 1; epoch <= m_maxEpochs; ++epoch) {
#ifdef HAS_OPENMP
      double mse = parallel_fann::train_epoch_irpropm_parallel(ann, train_data, m_nThreads);
#else
      double mse = fann_train_epoch(ann, train_data);
#endif
      trainLog[epoch - 1] = mse;
      // reduce weights that got too large
      for (unsigned iw = 0; iw < ann->total_connections; ++iw) {
        if (ann->weights[iw] > m_wMax)
          ann->weights[iw] = m_wMax;
        else if (ann->weights[iw] < -m_wMax)
          ann->weights[iw] = -m_wMax;
      }
      // evaluate validation set
      fann_reset_MSE(ann);
#ifdef HAS_OPENMP
      double valid_mse = parallel_fann::test_data_parallel(ann, valid_data, m_nThreads);
#else
      double valid_mse = fann_test_data(ann, valid_data);
#endif
      validLog[epoch - 1] = valid_mse;
      // keep weights for lowest validation error
      if (valid_mse < bestValid) {
        bestValid = valid_mse;
        for (unsigned iw = 0; iw < ann->total_connections; ++iw) {
          bestWeights[iw] = ann->weights[iw];
        }
        bestEpoch = epoch;
      }
      // break when validation error increases
      if (epoch > m_checkInterval && valid_mse > validLog[epoch - m_checkInterval]) {
        B2INFO("Training run " << irun << " stopped in epoch " << epoch);
        B2INFO("Train error: " << mse << ", valid error: " << valid_mse <<
               ", best valid: " << bestValid);
        breakEpoch = epoch;
        break;
      }
      // print current status
      if (epoch == 1 || (epoch < 100 && epoch % 10 == 0) || epoch % 100 == 0) {
        B2INFO("Epoch " << epoch << ": Train error = " << mse <<
               ", valid error = " << valid_mse << ", best valid = " << bestValid);
      }
    }
    if (breakEpoch == 0) {
      B2INFO("Training run " << irun << " finished in epoch " << m_maxEpochs);
      breakEpoch = m_maxEpochs;
    }
    trainOptLog.push_back(trainLog[bestEpoch - 1]);
    validOptLog.push_back(validLog[bestEpoch - 1]);
    // test trained network
    vector<float> oldWeights = m_NeuroTrigger[isector].getWeights();
    m_NeuroTrigger[isector].weights = bestWeights;
    vector<double> sumSqr;
    sumSqr.assign(nNodes[nLayers - 1], 0.);
    for (unsigned i = nTrain + m_nValid; i < m_trainSets[isector].nSamples(); ++i) {
      vector<float> output = m_NeuroTrigger.runMLP(isector, m_trainSets[isector].getInput(i));
      vector<float> target = m_trainSets[isector].getTarget(i);
      for (unsigned iout = 0; iout < output.size(); ++iout) {
        float diff = output[iout] - m_NeuroTrigger[isector].unscaleTarget(target)[iout];
        sumSqr[iout] += diff * diff;
      }
    }
    double sumSqrTotal = 0;
    if (m_parameters.targetZ) {
      sumSqrTotal += sumSqr[m_NeuroTrigger[isector].zIndex()];
      B2INFO("RMS z: " << sqrt(sumSqr[m_NeuroTrigger[isector].zIndex()] / m_nTest) << "cm");
    }
    if (m_parameters.targetTheta) {
      sumSqr[m_NeuroTrigger[isector].thetaIndex()] /= (Unit::deg * Unit::deg);
      sumSqrTotal += sumSqr[m_NeuroTrigger[isector].thetaIndex()];
      B2INFO("RMS theta: " << sqrt(sumSqr[m_NeuroTrigger[isector].thetaIndex()] / m_nTest) << "deg");
    }
    double RMS = sqrt(sumSqrTotal / m_nTest / sumSqr.size());
    B2INFO("RMS on test samples: " << RMS << " (best: " << bestRMS << ")");
    if (RMS < bestRMS) {
      bestRMS = RMS;
      bestTrainLog.assign(trainLog.begin(), trainLog.begin() + breakEpoch);
      bestValidLog.assign(validLog.begin(), validLog.begin() + breakEpoch);
    } else {
      m_NeuroTrigger[isector].weights = oldWeights;
    }
  }
  // save training log
  if (m_saveDebug) {
    // full error curve for best run
    ofstream logstream(m_logFilename + "_BestRun_" + to_string(isector) + ".log");
    for (unsigned i = 0; i < bestTrainLog.size(); ++i) {
      logstream << bestTrainLog[i] << " " << bestValidLog[i] << endl;
    }
    logstream.close();
    // training optimum for all runs
    ofstream logstreamOpt(m_logFilename + "_AllOptima_" + to_string(isector) + ".log");
    for (unsigned i = 0; i < trainOptLog.size(); ++i) {
      logstreamOpt << trainOptLog[i] << " " << validOptLog[i] << endl;
    }
    logstreamOpt.close();
  }
  // free memory
  fann_destroy_train(train_data);
  fann_destroy_train(valid_data);
  fann_destroy(ann);
  delete[] nNodes;
}

void
CDCTriggerNeuroTrainerModule::saveTraindata(const string& filename, const string& arrayname)
{
  B2INFO("Saving traindata to file " << filename << ", array " << arrayname);
  TFile datafile(filename.c_str(), "UPDATE");
  TObjArray* trainSets = new TObjArray(m_trainSets.size());
  for (unsigned isector = 0; isector < m_trainSets.size(); ++isector) {
    trainSets->Add(&m_trainSets[isector]);
    if (m_saveDebug) {
      phiHistsMC[isector]->Write(phiHistsMC[isector]->GetName(), TObject::kOverwrite);
      ptHistsMC[isector]->Write(ptHistsMC[isector]->GetName(), TObject::kOverwrite);
      thetaHistsMC[isector]->Write(thetaHistsMC[isector]->GetName(), TObject::kOverwrite);
      zHistsMC[isector]->Write(zHistsMC[isector]->GetName(), TObject::kOverwrite);
      phiHists2D[isector]->Write(phiHists2D[isector]->GetName(), TObject::kOverwrite);
      ptHists2D[isector]->Write(ptHists2D[isector]->GetName(), TObject::kOverwrite);
    }
  }
  trainSets->Write(arrayname.c_str(), TObject::kSingleKey | TObject::kOverwrite);
  datafile.Close();
  trainSets->Clear();
  delete trainSets;
  for (unsigned isector = 0; isector < phiHistsMC.size(); ++ isector) {
    delete phiHistsMC[isector];
    delete ptHistsMC[isector];
    delete thetaHistsMC[isector];
    delete zHistsMC[isector];
    delete phiHists2D[isector];
    delete ptHists2D[isector];
  }
  phiHistsMC.clear();
  ptHistsMC.clear();
  thetaHistsMC.clear();
  zHistsMC.clear();
  phiHists2D.clear();
  ptHists2D.clear();
}

bool
CDCTriggerNeuroTrainerModule::loadTraindata(const string& filename, const string& arrayname)
{
  TFile datafile(filename.c_str(), "READ");
  if (!datafile.IsOpen()) {
    B2WARNING("Could not open file " << filename);
    return false;
  }
  TObjArray* trainSets = (TObjArray*)datafile.Get(arrayname.c_str());
  if (!trainSets) {
    datafile.Close();
    B2WARNING("File " << filename << " does not contain key " << arrayname);
    return false;
  }
  m_trainSets.clear();
  for (int isector = 0; isector < trainSets->GetEntriesFast(); ++isector) {
    CDCTriggerMLPData* samples = dynamic_cast<CDCTriggerMLPData*>(trainSets->At(isector));
    if (samples) m_trainSets.push_back(*samples);
    else B2WARNING("Wrong type " << trainSets->At(isector)->ClassName() << ", ignoring this entry.");
  }
  trainSets->Clear();
  delete trainSets;
  datafile.Close();
  B2DEBUG(100, "loaded " << m_trainSets.size() << " training sets");
  return true;
}
