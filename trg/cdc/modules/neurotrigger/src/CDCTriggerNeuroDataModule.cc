#include "trg/cdc/modules/neurotrigger/CDCTriggerNeuroDataModule.h"

#include <fstream>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/core/ModuleParam.templateDetails.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <framework/gearbox/Unit.h>

#include <iostream>
#include <cmath>
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/multi_array.hpp"
#define BOOST_MULTI_ARRAY_NO_GENERATORS

namespace Belle2 {
  REG_MODULE(CDCTriggerNeuroData)

  CDCTriggerNeuroDataModule::CDCTriggerNeuroDataModule() : Module()
  {
    setDescription(
      "TODO"
    );
    // parameters for saving / loading
    addParam("hitCollectionName", m_hitCollectionName,
             "Name of the input StoreArray of CDCTriggerSegmentHits. Need to have a relation to inputtracks",
             std::string(""));
    addParam("IDHistFileName", m_idHistFilename,
             "Name of the IDHist file.",
             std::string(""));
    addParam("inputCollectionName", m_inputCollectionName,
             "Name of the StoreArray holding the 2D input tracks.",
             std::string("CDCTriggerNNInput2DTracks"));
    addParam("targetCollectionName", m_targetCollectionName,
             "Name of the MCParticle/RecoTrack collection used as target values.",
             std::string("RecoTracks"));
    addParam("trainOnRecoTracks", m_trainOnRecoTracks,
             "If true, use RecoTracks as targets instead of MCParticles.",
             true);
    addParam("EventTimeName", m_EventTimeName,
             "Name of the event time object.",
             std::string("CDCTriggerNeuroETFT0"));
    addParam("nMLP", m_parameters.nMLP,
             "Number of expert MLPs.",
             m_parameters.nMLP);
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
    addParam("et_option", m_parameters.et_option,
             "option on how to obtain the event time. Possibilities are: "
             "'etf_only', 'fastestpriority', 'zero', 'etf_or_fastestpriority', "
             "'etf_or_zero', 'etf_or_fastest2d', 'fastest2d'.",
             m_parameters.et_option);
    addParam("outputScale", m_parameters.outputScale,
             "Output scale for all networks (1 value list or nMLP value lists). "
             "Output[i] of the MLP is scaled from [-1, 1] "
             "to [outputScale[2*i], outputScale[2*i+1]]. "
             "(units: z[cm] / theta[degree])", m_parameters.outputScale);
    addParam("rescaleTarget", m_rescaleTarget,
             "If true, set target values > outputScale to 1, "
             "else skip them.", true);
    addParam("NeuroTrackInputMode", m_neuroTrackInputMode,
             "When using real tracks, use neurotracks instead of 2dtracks as input to the neurotrigger",
             true);
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
    addParam("gzipFilename", m_filename,
             "Name of the gzip file, where the test samples will be loaded from.",
             std::string("out.gz"));
    addParam("targetZ", m_parameters.targetZ,
             "Train one output of MLP to give z.", m_parameters.targetZ);
    addParam("targetTheta", m_parameters.targetTheta,
             "Train one output of MLP to give theta.", m_parameters.targetTheta);
    addParam("singleUse", m_singleUse,
             "Only use a track for a single expert", true);




  }
  void
  CDCTriggerNeuroDataModule::initialize()
  {
    // register store objects
    m_tracks.isRequired(m_inputCollectionName);
    // decided wether to load mcparticles or recotracks as training targets.
    // This has to be done because the train target values are obtained
    // for both classes in a different way.
    if (m_trainOnRecoTracks) {
      StoreArray<RecoTrack> targets(m_targetCollectionName);
      targets.isRequired(m_targetCollectionName);
    } else {
      StoreArray<MCParticle> targets(m_targetCollectionName);
      targets.isRequired(m_targetCollectionName);
    }
    // initialize the neurotrigger object, but use the parameters given in the module
    m_NeuroTrigger.initialize(m_parameters);
    // in this version, we first need an idhistfile in prior to collect the training data.
    // this idhistfile is created by running another steering file before.
    m_NeuroTrigger.loadIDHist(m_idHistFilename);
    m_trainSet.clear();
    CDC::CDCGeometryPar& cdc = CDC::CDCGeometryPar::Instance();
    // create an empty dataset of training data for each expert network
    for (unsigned iMLP = 0; iMLP < m_NeuroTrigger.nSectors(); ++iMLP) {
      m_trainSet.push_back(CDCTriggerMLPData());
      int layerId = 3;
      for (int iSL = 0; iSL < 9; ++iSL) {
        m_trainSet[iMLP].addCounters(cdc.nWiresInLayer(layerId));
        layerId += (iSL > 0 ? 6 : 7);
      }
    }
    // this one sets up the other root store arrays needed to collect training data
    m_NeuroTrigger.initializeCollections(m_hitCollectionName, m_EventTimeName, m_parameters.et_option);
    // consistency check of training parameters
    if (m_NeuroTrigger.nSectors() != m_trainSet.size()) {
      B2ERROR("Number of ID sets (" << m_trainSet.size() << ") should match " <<
              "number of sectors (" << m_NeuroTrigger.nSectors() << ")");
    }
    // overwrite previous file with empty file, in case it already exists
    std::ofstream gzipfile4(m_filename, std::ios_base::trunc | std::ios_base::binary);
    boost::iostreams::filtering_ostream outStream;
    outStream.push(boost::iostreams::gzip_compressor());
    outStream.push(gzipfile4);
  }
  void
  CDCTriggerNeuroDataModule::event()
  {
    StoreObjPtr<EventMetaData> evtmetadata;
    //vector<CDCTriggerMLPData>* trainSets_current;
    for (int itrack = 0; itrack < m_tracks.getEntries(); ++itrack) {
      // get related MCParticle/RecoTrack for target
      // and retrieve track parameters
      //float phi0Target = 0;
      //float invptTarget = 0;
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
        const std::vector<genfit::AbsTrackRep*>& reps = recoTrack->getRepresentations();
        bool foundValidRep = false;
        for (unsigned irep = 0; irep < reps.size() && !foundValidRep; ++irep) {
          if (!recoTrack->wasFitSuccessful(reps[irep]))
            continue;
          // get state (position, momentum etc.) from hit closest to IP and
          // extrapolate to z-axis (may throw an exception -> continue to next representation)
          try {
            genfit::MeasuredStateOnPlane state =
              recoTrack->getMeasuredStateOnPlaneClosestTo(TVector3(0, 0, 0), reps[irep]);
            reps[irep]->extrapolateToLine(state, TVector3(0, 0, -1000), TVector3(0, 0, 2000));
            // flip tracks if necessary, such that trigger tracks and reco tracks
            // point in the same direction
            if (state.getMom().Dot(m_tracks[itrack]->getDirection()) < 0) {
              state.setPosMom(state.getPos(), -state.getMom());
              state.setChargeSign(-state.getCharge());
            }
            // get track parameters
            //phi0Target = state.getMom().Phi();
            //invptTarget = state.getCharge() / state.getMom().Pt();
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
        if (not mcTrack) {
          B2DEBUG(150, "Skipping CDCTriggerTrack without relation to MCParticle.");
          continue;
        }
        //phi0Target = mcTrack->getMomentum().Phi();
        //invptTarget = mcTrack->getCharge() / mcTrack->getMomentum().Pt();
        thetaTarget = mcTrack->getMomentum().Theta();
        zTarget = mcTrack->getProductionVertex().Z();
      }
      m_NeuroTrigger.updateTrack(*m_tracks[itrack]);

      // find all matching sectors
      float phi0 = m_tracks[itrack]->getPhi0();
      float invpt = m_tracks[itrack]->getKappa(1.5);
      float theta = atan2(1., m_tracks[itrack]->getCotTheta());
      std::vector<int> sectors = m_NeuroTrigger.selectMLPs(phi0, invpt, theta);
      if (sectors.size() == 0) continue;
      // get target values
      std::vector<float> targetRaw = {};
      if (m_parameters.targetZ)
        targetRaw.push_back(zTarget);
      if (m_parameters.targetTheta)
        targetRaw.push_back(thetaTarget);
      for (unsigned i = 0; i < sectors.size(); ++i) {
        int isector = sectors[i];
        std::vector<float> target = m_NeuroTrigger[isector].scaleTarget(targetRaw);
        if (fabs(target[1]) < 0.0000000001) {
          std::cout << "Problem, target too small!!!! target: " << target[0] << ", " << target[1] << " targetraw: " << targetRaw[0] << ", " <<
                    targetRaw[1] << std::endl;
        }
        // skip out of range targets or rescale them
        bool outOfRange = false;
        for (unsigned itarget = 0; itarget < target.size(); ++itarget) {
          if (fabs(target[itarget]) > 1.) {
            outOfRange = true;
            target[itarget] /= fabs(target[itarget]);
          }
        }
        if (!m_rescaleTarget && outOfRange) continue;
        //
        // read out or determine event time
        //std::cout << "time: " << m_NeuroTrigger.m_T0 << std::endl;
        //std::cout << "getting event time" << std::endl;
        m_NeuroTrigger.getEventTime(isector, *m_tracks[itrack], m_parameters.et_option, m_neuroTrackInputMode);
        //std::cout << "time: " << m_NeuroTrigger.m_T0 << std::endl;
        // check hit pattern
        unsigned long hitPattern = m_NeuroTrigger.getInputPattern(isector, *m_tracks[itrack], m_neuroTrackInputMode);
        //std::cout << "hitpattern: " << hitPattern << std::endl;
        unsigned long sectorPattern = m_NeuroTrigger[isector].getSLpattern();
        unsigned long sectorPatternMask = m_NeuroTrigger[isector].getSLpatternMask();
        B2DEBUG(250, "hitPattern " << hitPattern << " sectorPattern " << sectorPattern);
        if (!m_singleUse && sectorPattern > 0 && (sectorPattern & hitPattern) != sectorPattern) {
          B2DEBUG(250, "hitPattern not matching " << (sectorPattern & hitPattern));
          continue;
        } else if (m_singleUse && sectorPattern > 0 && (sectorPattern) != (hitPattern & sectorPatternMask)) {
          B2DEBUG(250, "hitPattern not matching " << (sectorPatternMask & hitPattern));
          continue;
        }
        // check, if enough axials are there. first, we select the axial bits from the
        // hitpattern (341 = int('101010101',base=2)) and then check if the number of
        // ones is equal or greater than 4.
        if ((hitPattern & 341) != 341 && // this is an ugly workaround, because popcount is only
            (hitPattern & 341) != 340 && // available with c++20 and newer
            (hitPattern & 341) != 337 &&
            (hitPattern & 341) != 325 &&
            (hitPattern & 341) != 277 &&
            (hitPattern & 341) != 85) {
          B2DEBUG(250, "Not enough axial hits (<4), skipping!");
          continue;
        }
        // get training data
        std::vector<unsigned> hitIds;
        if (m_neuroTrackInputMode) {
          hitIds = m_NeuroTrigger.selectHitsHWSim(isector, *m_tracks[itrack]);
        } else {
          hitIds = m_NeuroTrigger.selectHits(isector, *m_tracks[itrack]);
        }
        std::cout << "hitids: " << hitIds.size() << std::endl;
        CDCTriggerMLPData::NeuroSet<27, 2> sample(m_NeuroTrigger.getInputVector(isector, hitIds).data(), target.data(),
                                                  evtmetadata->getExperiment(), evtmetadata->getRun(), evtmetadata->getSubrun(), evtmetadata->getEvent(), itrack, i);
        //check whether we already have enough samples
        std::cout << sample << std::endl;
        m_trainSet[isector].addSample(sample);
        if ((m_trainSet)[isector].nSamples() % 1000 == 0) {
          B2DEBUG(50, m_trainSet[isector].nSamples() << " samples for training collected for sector " << isector);
        }
        std::ofstream gzipfile4(m_filename, std::ios_base::app | std::ios_base::binary);
        boost::iostreams::filtering_ostream outStream;
        outStream.push(boost::iostreams::gzip_compressor());
        outStream.push(gzipfile4);
        outStream << sample << std::endl;
        m_trackcounter++;
      }

    }
  }
  void
  CDCTriggerNeuroDataModule::terminate()
  {
    std::stringstream ss;
    for (unsigned int i = 0; i < m_trainSet.size(); ++i) {
      ss << "expert " << i << " : " << m_trainSet[i].nSamples() << ", ";
    }
    B2DEBUG(10, "Collected events: " << ss.str());
  }
}
