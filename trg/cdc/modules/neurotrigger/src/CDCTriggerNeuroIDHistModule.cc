#include "trg/cdc/modules/neurotrigger/CDCTriggerNeuroIDHistModule.h"
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
#include <fstream>
#include <cmath>
#include <TFile.h>
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/multi_array.hpp"
#define BOOST_MULTI_ARRAY_NO_GENERATORS

namespace Belle2 {
  REG_MODULE(CDCTriggerNeuroIDHist)
  CDCTriggerNeuroIDHistModule::CDCTriggerNeuroIDHistModule() : Module()
  {
    setDescription(
      "description" //TODO
    );
    // parameters for saving / loading
    addParam("hitCollectionName", m_hitCollectionName,
             "Name of the input StoreArray of CDCTriggerSegmentHits.",
             std::string(""));
    addParam("inputCollectionName", m_inputCollectionName,
             "Name of the StoreArray holding the 2D input tracks.",
             std::string("TRGCDC2DFinderTracks"));
    addParam("trainOnRecoTracks", m_trainOnRecoTracks,
             "If true, use RecoTracks as targets instead of MCParticles.",
             false);
    addParam("targetCollectionName", m_targetCollectionName,
             "Name of the MCParticle/RecoTrack collection used as target values.",
             std::string("MCParticles"));
    addParam("IDHistName", m_idHistName,
             "BaseName of the both the IDHistfile, where the ID Histograms will "
             "be saved and the config file, where the configuration is written.",
             std::string("idHist"));
    addParam("MaxEvents", m_nPrepare,
             "amount of events used for creating the IDHist. If it is 0, "
             "all Events are used.",
             0);
    addParam("nMLP", m_parameters.nMLP,
             "Number of expert MLPs.", m_parameters.nMLP);
    addParam("targetZ", m_parameters.targetZ,
             "Train one output of MLP to give z.", m_parameters.targetZ);
    addParam("targetTheta", m_parameters.targetTheta,
             "Train one output of MLP to give theta.", m_parameters.targetTheta);
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
             "Maximal drift time (for scaling, unit: trigger timing bins).",
             m_parameters.tMax);
    addParam("relevantCut", m_relevantCut,
             "Cut for preparation of relevant ID ranges.", 0.02);
    addParam("cutSum", m_cutSum,
             "If true, relevantCut is applied to the sum over hit counters, "
             "otherwise directly on the hit counters.", false);
    addParam("rescaleTarget", m_rescaleTarget,
             "If true, set target values > outputScale to 1, "
             "else skip them.", true);
    addParam("outputScale", m_parameters.outputScale,
             "Output scale for all networks (1 value list or nMLP value lists). "
             "Output[i] of the MLP is scaled from [-1, 1] "
             "to [outputScale[2*i], outputScale[2*i+1]]. "
             "(units: z[cm] / theta[degree])", m_parameters.outputScale);

  }


  void CDCTriggerNeuroIDHistModule::initialize()
  {
    /**
     * First Initialize the neurotrigger object, which is needed because of
     * the original structure of the training procedure in the old module.
     * Second, the dataset for the idhist data will be initialized and set up.
     */
    m_NeuroTrigger.initialize(m_parameters);
    m_trainSets_prepare.clear();
    CDC::CDCGeometryPar& cdc = CDC::CDCGeometryPar::Instance();
    for (unsigned iMLP = 0; iMLP < m_NeuroTrigger.nSectors(); ++iMLP) {
      // sectors means experts here; this is the old naming within the neurotrigger
      // class which originates from times were we had different expert networks for
      // different geometrical sectors. In future, this might come back because of
      // the 3DFinder and its rough information about the theta angle of the tracks.
      m_trainSets_prepare.push_back(CDCTriggerMLPData());
      // layerid is the layer number where the priority hits are.
      // for every 1st priority wire there is a corresponding track segment.
      int layerId = 3;
      for (int iSL = 0; iSL < 9; ++iSL) {
        std::cout << "wires in layer " << iSL << ": " << cdc.nWiresInLayer(layerId) << std::endl;

        m_trainSets_prepare[iMLP].addCounters(cdc.nWiresInLayer(layerId));
        // the first superlayer has 2 layers extra compared to the rest
        layerId += (iSL > 0 ? 6 : 7);
      }
      m_tracks.isRequired(m_inputCollectionName);
      if (m_trainOnRecoTracks) {
        StoreArray<RecoTrack> targets(m_targetCollectionName);
        targets.isRequired(m_targetCollectionName);
      } else {
        StoreArray<MCParticle> targets(m_targetCollectionName);
        targets.isRequired(m_targetCollectionName);
      }

      m_NeuroTrigger.initializeCollections(m_hitCollectionName);




    }
  }



  void
  CDCTriggerNeuroIDHistModule::event()
  {
    StoreObjPtr<EventMetaData> evtmetadata;
    for (int itrack = 0; itrack < m_tracks.getEntries(); ++itrack) {
      // get related MCParticle/RecoTrack for target
      // and retrieve track parameters

      std::vector<float> targetvector = NeuroTrainer::getTrainTargets(m_trainOnRecoTracks, m_tracks[itrack], m_targetCollectionName);
      if (targetvector[4] == 0) {continue;} // no valid representation found
      float phi0Target = targetvector[0];
      float invptTarget = targetvector[1];
      float thetaTarget = targetvector[2];
      float zTarget = targetvector[3];


      // update 2D track variables
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
        if (m_nPrepare == 0 || m_trainSets_prepare[isector].getTrackCounter() < m_nPrepare) {
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
              m_trainSets_prepare[isector].addHit(hit.getISuperLayer(), round(relId));
            }
          } else {
            MCParticle* mcTrack =
              m_tracks[itrack]->getRelatedTo<MCParticle>(m_targetCollectionName);
            for (const CDCTriggerSegmentHit& hit :
                 mcTrack->getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName)) {
              // get relative id
              double relId = m_NeuroTrigger.getRelId(hit);
              m_trainSets_prepare[isector].addHit(hit.getISuperLayer(), round(relId));
            }
          }
          m_trainSets_prepare[isector].countTrack();
        }
      }
    }
    bool stop = true;
    for (unsigned isector = 0; isector < m_trainSets_prepare.size(); ++isector) {
      if (m_nPrepare == 0 || m_trainSets_prepare[isector].getTrackCounter() < m_nPrepare) {
        stop = false;
        break;
      }
    }
    if (stop) {
      B2INFO("Training sample preparation for NeuroTrigger finished, stopping event loop.");
      // if required hit number is reached, get relevant ids
      StoreObjPtr<EventMetaData> eventMetaData;
      eventMetaData->setEndOfData();
    }
  }

  void
  CDCTriggerNeuroIDHistModule::terminate()
  {
    for (unsigned isector = 0; isector < m_trainSets_prepare.size(); ++isector) {
      std::vector<float> reid = NeuroTrainer::getRelevantID(m_trainSets_prepare[isector], m_cutSum, m_relevantCut);
      m_NeuroTrigger[isector].setRelID(reid);
    }
    std::ofstream gzipfile4(m_idHistName + ".gz", std::ios_base::app | std::ios_base::binary);
    boost::iostreams::filtering_ostream outStream;
    outStream.push(boost::iostreams::gzip_compressor());
    outStream.push(gzipfile4);
    for (unsigned isector = 0; isector < m_NeuroTrigger.nSectors(); ++isector) {
      //std::vector<float> reid = m_NeuroTrigger[isector].getRelID();
      NeuroTrainer::HeaderSet hset(isector, m_NeuroTrigger[isector].getRelID()); //, m_NeuroTrigger[isector].et_option);
      std::cout << hset << std::endl;
      outStream << hset << std::endl;
    }
    //TODO: also write the config file to be directly able to start the training
  }

}
