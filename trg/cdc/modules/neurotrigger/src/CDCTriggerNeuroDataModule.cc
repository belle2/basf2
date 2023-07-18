/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


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
#include <framework/geometry/B2Vector3.h>


#include <iostream>
#include <cmath>
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/multi_array.hpp"
#define BOOST_MULTI_ARRAY_NO_GENERATORS

namespace Belle2 {
  REG_MODULE(CDCTriggerNeuroData);

  CDCTriggerNeuroDataModule::CDCTriggerNeuroDataModule() : Module()
  {
    setDescription(
      "This module takes 2dtracks, track segments, and targettracks (either recotracks or mcparticles) as input and generates training data for the neurotrigger in a tab separated, gzip compressed file."
    );
    // parameters for saving / loading
    addParam("hitCollectionName", m_hitCollectionName,
             "Name of the input StoreArray of CDCTriggerSegmentHits. The Axials need to have a relation to inputtracks",
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
    addParam("NeuroTrackInputMode", m_neuroTrackInputMode,
             "When using real tracks, use neurotracks instead of 2dtracks as input to the neurotrigger. this is important to get the relations right.",
             false);
    addParam("singleUse", m_singleUse,
             "Only use a track for a single expert", true);
    addParam("configFileName", m_configFileName,
             "Name of the configuration file. This File should be created by the CDCTriggerIDHistModule and will be extended in this module",
             std::string(""));
    addParam("writeconfigFileName", m_writeconfigFileName,
             "Name of the configuration file, which will be written. If left blank, the same file as the input configuration file is used and it will be overwritten.",
             std::string(""));
    addParam("gzipFilename", m_filename,
             "Name of the gzip file, where the training samples will be saved.",
             std::string("out.gz"));




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
    if (m_configFileName != "") {
      m_neuroParameters.loadconfigtxt(m_configFileName);
      m_NeuroTrigger.initialize(m_neuroParameters);
    } else {
      B2ERROR("The Neurotrigger needs to be initialized by a configuration file! Make sure to give the configuration file as a parameter.");
    }
    // in this version, we first need an idhistfile in prior to collect the training data.
    // this idhistfile is created by running another steering file before
    // and stored in the config file loaded in the previous step.
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
    m_NeuroTrigger.initializeCollections(m_hitCollectionName, m_EventTimeName, m_neuroParameters.et_option());
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
    CDCTriggerMLPData::NeuroSet<27, 2> sample;
    outStream << sample.headline << std::endl;
  }
  void
  CDCTriggerNeuroDataModule::event()
  {
    StoreObjPtr<EventMetaData> evtmetadata;
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
          if (!recoTrack->wasFitSuccessful(reps[irep])) {
            continue;
          }

          // get state (position, momentum etc.) from hit closest to IP and
          // extrapolate to z-axis (may throw an exception -> continue to next representation)
          try {
            genfit::MeasuredStateOnPlane state =
              recoTrack->getMeasuredStateOnPlaneClosestTo(ROOT::Math::XYZVector(0, 0, 0), reps[irep]);
            reps[irep]->extrapolateToLine(state, TVector3(0, 0, -1000), TVector3(0, 0, 2000));
            // flip tracks if necessary, such that trigger tracks and reco tracks
            // point in the same direction
            if (state.getMom().Dot(B2Vector3D(m_tracks[itrack]->getDirection())) < 0) {
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
        if (!m_neuroParameters.rescaleTarget && outOfRange) {
          continue;
        }
        //
        // read out or determine event time
        m_NeuroTrigger.getEventTime(isector, *m_tracks[itrack], m_neuroParameters.et_option(), m_neuroTrackInputMode);
        // check hit pattern
        unsigned long hitPattern = m_NeuroTrigger.getCompleteHitPattern(isector, *m_tracks[itrack], m_neuroTrackInputMode); // xxxxx0xxx
        // sectorpattern holds the absolut necessary SLs for the expert
        unsigned long sectorPattern = m_NeuroTrigger[isector].getSLpattern(); // 010100010
        // sectorpatternmask holds the SLs, which are generally used to determine the right expert
        unsigned long sectorPatternMask = m_NeuroTrigger[isector].getSLpatternMask(); // 010101010
        B2DEBUG(250, "hitPattern " << hitPattern << " sectorPattern " << sectorPattern);
        // if multiple experts should be trained with one track, the necessary SLs have to be in the hitpattern
        if (!m_singleUse && sectorPattern > 0 && (sectorPattern & hitPattern) != sectorPattern) {
          B2DEBUG(250, "hitPattern not matching " << (sectorPattern & hitPattern));
          continue;
          // if one track should only be used for one expert, the absolute necessary SLs for this expert should match exactly the set within the hitpattern of the generally used SLs for determining the expert.
        } else if (m_singleUse && sectorPattern > 0 && (sectorPattern) != (hitPattern & sectorPatternMask)) {
          // 010100010 != 0x0x0x0x0
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
        // add a "sample" it is a full training set including some zeroes for the neurotrigger values (raw and scaled z and theta). Those are sometimes used in the training for reference, but cannot be added here without running *some* neuotrigger instance.
        CDCTriggerMLPData::NeuroSet<27, 2> sample(m_NeuroTrigger.getInputVector(isector, hitIds).data(), target.data(),
                                                  evtmetadata->getExperiment(), evtmetadata->getRun(), evtmetadata->getSubrun(), evtmetadata->getEvent(), itrack, i,
                                                  m_tracks.getEntries(), 0, 0, 0, 0, phi0, theta, invpt);
        //check whether we already have enough samples
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
    // lock the parameters, which were used to obtain the training data, so they cannot be altered in the next steps.
    m_neuroParameters.ETOption.lock();
    m_neuroParameters.rescaleTarget.lock();
    m_neuroParameters.targetZ.lock();
    m_neuroParameters.targetTheta.lock();
    for (auto x : m_neuroParameters.maxHitsPerSL) {
      x.lock();
    }
    if (m_writeconfigFileName == "") {
      B2INFO("write writeconfig now: " + m_configFileName);
      m_neuroParameters.saveconfigtxt(m_configFileName);
    } else {
      B2INFO("write writeconfig now: " + m_writeconfigFileName);
      m_neuroParameters.saveconfigtxt(m_writeconfigFileName);
    }

    B2DEBUG(10, "Collected events: " << ss.str());
  }
}
