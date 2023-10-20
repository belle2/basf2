/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

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
  REG_MODULE(CDCTriggerNeuroIDHist);
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
    addParam("writeconfigfile", m_writeconfigFileName,
             "Name of the config file, where all the parameters and the IDHist configuration is written.",
             std::string(""));
    addParam("configfile", m_configFileName,
             "Name of the config file, where all the parameters and the IDHist configuration is read in from.",
             std::string(""));
    addParam("MaxEvents", m_nPrepare,
             "amount of events used for creating the IDHist. If it is 0, "
             "all Events are used.",
             0);

  }


  void CDCTriggerNeuroIDHistModule::initialize()
  {
    /**
     * First Initialize the neurotrigger object, which is needed because of
     * the original structure of the training procedure in the old module.
     * Second, the dataset for the idhist data will be initialized and set up.
     */
    if (m_configFileName != "") {
      m_neuroParameters.loadconfigtxt(m_configFileName);
      m_NeuroTrigger.initialize(m_neuroParameters);
    } else {
      B2ERROR("Configuration file is missing! Make sure to give the configuration file as a parameter.");
    }
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
    if (m_NeuroTrigger.nSectors() == 0) {
      B2ERROR("No networks defined, please make sure to have a proper configuration file! Example file will be created here: ./neurotrigger_default.conf");
      m_neuroParameters.saveconfigtxt("neurotrigger_default.conf");
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
      if (targetvector[4] == 0) {
        continue;
      } // no valid representation found
      // float phi0Target = targetvector[0];  // currently unused
      // float invptTarget = targetvector[1]; // currently unused
      float thetaTarget = targetvector[2];
      float zTarget = targetvector[3];


      // update 2D track variables
      m_NeuroTrigger.updateTrack(*m_tracks[itrack]);

      // find all matching sectors
      float phi0 = m_tracks[itrack]->getPhi0();
      float invpt = m_tracks[itrack]->getKappa(1.5);
      float theta = atan2(1., m_tracks[itrack]->getCotTheta());
      std::vector<int> sectors = m_NeuroTrigger.selectMLPsTrain(phi0, invpt, theta);
      if (sectors.size() == 0) continue;
      // get target values
      std::vector<float> targetRaw = {};
      if (m_neuroParameters.targetZ)
        targetRaw.push_back(zTarget);
      if (m_neuroParameters.targetTheta)
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
        if (!m_neuroParameters.rescaleTarget && outOfRange) continue;
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

    if (m_neuroParameters.IDRanges.size() > 0) {
      if (m_neuroParameters.IDRanges[0].size() > 0) {
        if (m_neuroParameters.IDRanges[0][0].isSet()) {
          // the idranges are already set, print warning:
          if (!m_neuroParameters.IDRanges[0][0].isLocked()) {
            B2WARNING("ID ranges are already set in the config file, they will be updated now!");
          } else {
            B2ERROR("The ID ranges in the config file are already locked and cannot be updated!");
            return;
          }
        }
      }
    }
    m_neuroParameters.IDRanges.clear();
    for (unsigned isector = 0; isector < m_trainSets_prepare.size(); ++isector) {
      CDCTriggerMLPData::HeaderSet hset(isector, NeuroTrainer::getRelevantID(
                                          m_trainSets_prepare[isector],
                                          m_neuroParameters.cutSum,
                                          m_neuroParameters.relevantCut));
      std::vector<NNTParam<float>> expertline;
      expertline.push_back(float(isector));
      expertline.back().lock();
      for (auto x : m_NeuroTrigger[isector].getRelID()) {
        expertline.push_back(x);
        expertline.back().lock();
      }
      m_neuroParameters.IDRanges.push_back(expertline);
      B2DEBUG(15, hset);
    }
    m_configFileName = "IDTable_" + m_configFileName;
    // lock the variables used in this module, that are not supposed be changed
    // further down the training chain because of the danger of implications or
    // wrong assumptions.
    m_neuroParameters.relevantCut.lock();
    m_neuroParameters.cutSum.lock();
    // the IDRanges are set here; however, they can be altered manually in the
    // configuration file to achieve potentially better results, eg. widen
    // the range of the axial phi acceptance. This is why they are not to be "locked".
    m_neuroParameters.nInput.lock();
    m_neuroParameters.nOutput.lock();
    m_neuroParameters.nMLP.lock();
    for (auto x : m_neuroParameters.SLpattern) {
      x.lock();
    }
    for (auto x : m_neuroParameters.SLpatternMask) {
      x.lock();
    }
    for (auto x : m_neuroParameters.maxHitsPerSL) {
      x.lock();
    }
    m_neuroParameters.multiplyHidden.lock();

    if (m_writeconfigFileName == "") {
      m_neuroParameters.saveconfigtxt(m_configFileName);
    } else {
      m_neuroParameters.saveconfigtxt(m_writeconfigFileName);
    }


    // the *rangeTrain variables are used here, but just for obtaining the idranges.
    // because they only have a very minor effect on those, they are not locked here.
    //TODO: also write the config file to be directly able to start the training
  }

}
