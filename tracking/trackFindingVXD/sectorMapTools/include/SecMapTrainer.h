/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <tracking/trackFindingVXD/sectorMapTools/SecMapTrainerHit.h>
#include <tracking/trackFindingVXD/sectorMapTools/SecMapTrainerTC.h>
#include <tracking/trackFindingVXD/sectorMapTools/TrainerConfigData.h>

#include <tracking/trackFindingVXD/sectorMapTools/FilterMill.h>
#include <tracking/trackFindingVXD/sectorMapTools/RawSecMapRootInterface.h>
#include <tracking/trackFindingVXD/sectorMapTools/SectorTools.h>

#include <tracking/vectorTools/B2Vector3.h>

#include <tracking/dataobjects/FullSecID.h>
#include <vxd/dataobjects/VxdID.h>

#include <string>
#include <vector>
#include <utility> // std::pair, std::move
#include <functional>
#include <limits>       // std::numeric_limits


namespace Belle2 {

  /** This class contains all relevant tools for training a sectorMap. */
  template <class FilterFactoryType>
  class SecMapTrainer {
  protected:
    /** Contains all relevant configurations needed for training a sectorMap. */
    TrainerConfigData m_config;


    /** A factory taking care of having the correct filters prepared for secMapTraining. */
    FilterFactoryType m_factory;


    /** Stores the prepared filters and applies them on given hit-combinations. */
    FilterMill<SecMapTrainerHit> m_filterMill;


    /** Interface for nice and neat storing in root-ttree. */
    RawSecMapRootInterface m_rootInterface;


    /** contains tcs of event, reset after each event. */
    std::vector<SecMapTrainerTC> m_tcs;


    /** contains current experiment number. */
    unsigned m_expNo;


    /** contains current run number. */
    unsigned m_runNo;


    /** contains current event number. */
    unsigned m_evtNo;



    /** checks if given Hit is acceptable for this trainer.
    *
    * Returns true if accepted and false otherwise. */
    bool acceptHit(const SpacePoint& hit)
    {
      // rejects if layerID is not acceptable for this secMap-training.
      auto layerID = hit.getVxdID().getLayerNumber();
      B2DEBUG(10, "SecMapTrainer::acceptHit: the TC has layerID/thresholdmin/-max: " << layerID << "/" << m_config.minMaxLayer.first <<
              "/" << m_config.minMaxLayer.second)
      if (m_config.minMaxLayer.first > layerID
          or m_config.minMaxLayer.second < layerID)
      { return false; }

      B2DEBUG(20, "SecMapTrainer::acceptHit: isAccepted")
      return true;
    }



    /** processes all two-hit-combinations of given TC and stores their results. returns number of stored values. */
    unsigned process2HitCombinations(const SecMapTrainerTC& aTC)
    {
      unsigned nValues = 0;
      B2DEBUG(10, "SecMapTrainer::process2HitCombinations: nHits/trackID/pdg: " << aTC.size() << "/" << aTC.getTrackID() << "/" <<
              aTC.getPDG())
      if (aTC.size() < 2) { return nValues; }

      // the iterators mark the hits to be used:
      SecMapTrainerTC::ConstIterator outerIt = aTC.outermostHit();
      SecMapTrainerTC::ConstIterator innerIt = ++aTC.outermostHit();

      // loop over all 2-hit-combis, collect data for each filterType and store it in root-tree:
      std::vector<std::pair<std::string, double> > collectedResults;
      for (; innerIt != aTC.innerEnd();) {
        B2DEBUG(10, "SecMapTrainer::process2HitCombinations: outerHit-/innerHitSecID: " << outerIt->getSectorIDString() <<
                "/" << innerIt->getSectorIDString())

        auto& dataSet = m_rootInterface.get2HitDataSet();
        dataSet.expNo = m_expNo;
        dataSet.runNo = m_runNo;
        dataSet.evtNo = m_evtNo;
        dataSet.trackNo = aTC.getTrackID();
        dataSet.pdg = aTC.getPDG();
        dataSet.secIDs.outer = outerIt->getSectorID().getFullSecID();
        dataSet.secIDs.inner = innerIt->getSectorID().getFullSecID();

        // create data for each filterType:
        FilterMill<SecMapTrainerHit>::HitPair newHitPair;
        newHitPair.outer = &(*outerIt);
        newHitPair.inner = &(*innerIt);
        m_filterMill.grindData2Hit(newHitPair, collectedResults);

        // fill data for each filter type:
        for (const auto& entry : collectedResults) {
          B2DEBUG(50, "SecMapTrainer::process2HitCombinations: filter/value: " << entry.first << "/" << entry.second)
          dataSet.setValueOfFilter(entry.first, entry.second);
          ++nValues;
        }
        m_rootInterface.fill2Hit();
        collectedResults.clear();
        ++outerIt;
        ++innerIt;
      }
      return nValues;
    }



    /** processes all three-hit-combinations of given TC and stores their results. returns number of stored values. */
    unsigned process3HitCombinations(const SecMapTrainerTC& aTC)
    {
      unsigned nValues = 0;
      B2DEBUG(10, "SecMapTrainer::process3HitCombinations: nHits/trackID/pdg: " << aTC.size() << "/" << aTC.getTrackID() << "/" <<
              aTC.getPDG())
      if (aTC.size() < 3) { return nValues; }

      // the iterators mark the hits to be used:
      SecMapTrainerTC::ConstIterator outerIt = aTC.outermostHit();
      SecMapTrainerTC::ConstIterator centerIt = ++aTC.outermostHit();
      SecMapTrainerTC::ConstIterator innerIt = ++(++aTC.outermostHit());

      // loop over all 3-hit-combis, collect data for each filterType and store it in root-tree:
      std::vector<std::pair<std::string, double> > collectedResults;
      for (; innerIt != aTC.innerEnd();) {
        B2DEBUG(10, "SecMapTrainer::process3HitCombinations: outer-/center-/innerHitSecID: " << outerIt->getSectorIDString() <<
                "/" << centerIt->getSectorIDString() <<
                "/" << innerIt->getSectorIDString())

        auto& dataSet = m_rootInterface.get3HitDataSet();
        dataSet.expNo = m_expNo;
        dataSet.runNo = m_runNo;
        dataSet.evtNo = m_evtNo;
        dataSet.trackNo = aTC.getTrackID();
        dataSet.pdg = aTC.getPDG();
        dataSet.secIDs.outer = outerIt->getSectorID().getFullSecID();
        dataSet.secIDs.center = centerIt->getSectorID().getFullSecID();
        dataSet.secIDs.inner = innerIt->getSectorID().getFullSecID();

        // create data for each filterType:
        FilterMill<SecMapTrainerHit>::HitTriplet newHitTriplet;
        newHitTriplet.outer = &(*outerIt);
        newHitTriplet.center = &(*centerIt);
        newHitTriplet.inner = &(*innerIt);
        m_filterMill.grindData3Hit(newHitTriplet, collectedResults);

        // fill data for each filter type:
        for (const auto& entry : collectedResults) {
          B2DEBUG(50, "SecMapTrainer::process3HitCombinations: filter/value: " << entry.first << "/" << entry.second)
          dataSet.setValueOfFilter(entry.first, entry.second);
          ++nValues;
        }
        m_rootInterface.fill3Hit();
        collectedResults.clear();
        ++outerIt;
        ++centerIt;
        ++innerIt;
      }
      return nValues;
    }



    /** processes all four-hit-combinations of given TC and stores their results. returns number of stored values. */
    unsigned process4HitCombinations(const SecMapTrainerTC& aTC)
    {
      unsigned nValues = 0;
      B2DEBUG(10, "SecMapTrainer::process4HitCombinations: nHits/trackID/pdg: " << aTC.size() << "/" << aTC.getTrackID() << "/" <<
              aTC.getPDG())
      if (aTC.size() < 4) { return nValues; }

      // the iterators mark the hits to be used:
      SecMapTrainerTC::ConstIterator outerIt = aTC.outermostHit();
      SecMapTrainerTC::ConstIterator outerCenterIt = ++aTC.outermostHit();
      SecMapTrainerTC::ConstIterator innerCenterIt = ++(++aTC.outermostHit());
      SecMapTrainerTC::ConstIterator innerIt = ++(++(++aTC.outermostHit()));

      // loop over all 2-hit-combis, collect data for each filterType and store it in root-tree:
      std::vector<std::pair<std::string, double> > collectedResults;
      for (; innerIt != aTC.innerEnd();) {
        B2DEBUG(10, "SecMapTrainer::process4HitCombinations: outer-/oCenter-/iCenter-/innerHitSecID: " << outerIt->getSectorIDString() <<
                "/" << outerCenterIt->getSectorIDString() <<
                "/" << innerCenterIt->getSectorIDString() <<
                "/" << innerIt->getSectorIDString())


        auto& dataSet = m_rootInterface.get4HitDataSet();
        dataSet.expNo = m_expNo;
        dataSet.runNo = m_runNo;
        dataSet.evtNo = m_evtNo;
        dataSet.trackNo = aTC.getTrackID();
        dataSet.pdg = aTC.getPDG();
        dataSet.secIDs.outer = outerIt->getSectorID().getFullSecID();
        dataSet.secIDs.outerCenter = outerCenterIt->getSectorID().getFullSecID();
        dataSet.secIDs.innerCenter = innerCenterIt->getSectorID().getFullSecID();
        dataSet.secIDs.inner = innerIt->getSectorID().getFullSecID();

        // create data for each filterType:
        FilterMill<SecMapTrainerHit>::HitQuadruplet newHitQuadruplet;
        newHitQuadruplet.outer = &(*outerIt);
        newHitQuadruplet.outerCenter = &(*outerCenterIt);
        newHitQuadruplet.innerCenter = &(*innerCenterIt);
        newHitQuadruplet.inner = &(*innerIt);
        m_filterMill.grindData4Hit(newHitQuadruplet, collectedResults);

        // fill data for each filter type:
        for (const auto& entry : collectedResults) {
          B2DEBUG(50, "SecMapTrainer::process4HitCombinations: filter/value: " << entry.first << "/" << entry.second)
          dataSet.setValueOfFilter(entry.first, entry.second);
          ++nValues;
        }
        m_rootInterface.fill4Hit();
        collectedResults.clear();
        ++outerIt;
        ++outerCenterIt;
        ++innerCenterIt;
        ++innerIt;
      }
      return nValues;
    }



    /** converts the SpacePoints into a SecMapTrainerTC */
    void convertSP2TC(
      std::vector<std::pair< FullSecID, const SpacePoint*> >& goodSPs,
      unsigned tcID, double pTValue, int pdgCode)
    {
      B2DEBUG(10, "SecMapTrainer::convertSPTC: nGoodHits: " << goodSPs.size())

      SecMapTrainerTC newTrack(tcID, pTValue, pdgCode);

      for (const auto& secIDAndSPpair : goodSPs) {
        FullSecID fullSecID = secIDAndSPpair.first;
        B2DEBUG(20, "SecMapTrainer::convertSPTC: found fullSecID: " << fullSecID.getFullSecString())

        newTrack.addHit(std::move(
                          SecMapTrainerHit(fullSecID, secIDAndSPpair.second->getPosition())));
      }

      // add vertex (but without real vertexPosition, since origin is assumed)
      SecMapTrainerHit newVirtualHit(FullSecID().getFullSecString(), m_config.vIP);

      newTrack.addHit(std::move(newVirtualHit));

      m_tcs.push_back(std::move(newTrack));
    }


  public:

    /** constructor. */
    SecMapTrainer(TrainerConfigData& configData, int rngAppendix = 0) :
      m_config(configData),
      m_factory(
        configData.vIP.X(),
        configData.vIP.Y(),
        configData.vIP.Z(),
        configData.mField),
      m_filterMill(),
      m_rootInterface(configData.secMapName, rngAppendix)
    {
      // stretch the cuts:
      m_config.pTCuts.first -= m_config.pTCuts.first * m_config.pTSmear;
      m_config.pTCuts.second += m_config.pTCuts.second * m_config.pTSmear;
    }


    /** initialize the trainer (to be called in Module::initialize(). */
    void initialize()
    {
      // prepare the filters:
      m_rootInterface.initialize2Hit(m_config.twoHitFilters);
      for (auto& fName : m_config.twoHitFilters) {
        auto filter = m_factory.get2HitInterface(fName);
        m_filterMill.add2HitFilter({fName, filter});
      }

      m_rootInterface.initialize3Hit(m_config.threeHitFilters);
      for (auto& fName : m_config.threeHitFilters) {
        auto filter = m_factory.get3HitInterface(fName);
        m_filterMill.add3HitFilter({fName, filter});
      }

      m_rootInterface.initialize4Hit(m_config.fourHitFilters);
      for (auto& fName : m_config.fourHitFilters) {
        auto filter = m_factory.get4HitInterface(fName);
        m_filterMill.add4HitFilter({fName, filter});
      }

      // prevent further modifications:
      m_filterMill.lockMill();
    }

    /** initialize the trainer (to be called in Module::terminate(). */
//     void terminate(TFile* file)
//     { m_rootInterface.write(file); }
    void terminate()
    { m_rootInterface.write(); }

    /** Initialize event. */
    void initializeEvent(int expNo, int runNo, int evtNo)
    {
      m_expNo = expNo;
      m_runNo = runNo;
      m_evtNo = evtNo;
    }


    /** returns configuration. */
    const TrainerConfigData& getConfig() { return m_config; }


    /** checks if given TC is acceptable for this trainer and store it if it is accepted.
    *
    * The return value is true, it was accepted and stored, false if not.
    */
    bool storeTC(const SpacePointTrackCand& tc, unsigned iD)
    {
      B2DEBUG(10, "SecMapTrainer::storeTC: nHits/threshold: " << tc.getNHits() << "/" << m_config.nHitsMin)
      // catch TCS which are too short in any case
      if (tc.getNHits() < m_config.nHitsMin) return false;

      B2DEBUG(10, "SecMapTrainer::storeTC: hasHitsOnSameSensor: " << tc.hasHitsOnSameSensor())
      // catch TCs where more than one hit was on the same sensor
      if (tc.hasHitsOnSameSensor()) return false;

      // catch TCS where particle type is wrong
      for (const auto& pdg : m_config.pdgCodesAllowed) {
        if (tc.getPdgCode() == pdg) return false;
      }

      // check if momentum of TC is within range:
      auto pT = tc.getMomSeed().Perp();
      B2DEBUG(10, "SecMapTrainer::storeTC: pT/thresholdmin/-max: " << pT << "/" << m_config.pTCuts.first << "/" <<
              m_config.pTCuts.second)
      if (m_config.pTCuts.first > pT
          or m_config.pTCuts.second < pT) return false;

      // catch tracks which start too far away from orign
      B2Vector3D distance2IP = m_config.vIP - tc.getPosSeed();
      B2DEBUG(10, "SecMapTrainer::storeTC: distance2IP/thresholdXY/-Z: " << distance2IP.Mag() << "/" << m_config.seedMaxDist2IPXY << "/"
              << m_config.seedMaxDist2IPZ)
      if (m_config.seedMaxDist2IPXY > 0
          and m_config.seedMaxDist2IPXY < distance2IP.Perp()) return false;
      if (m_config.seedMaxDist2IPZ > 0
          and m_config.seedMaxDist2IPXY < distance2IP.Z()) return false;

      // collect hits which fullfill all given tests
      std::vector<std::pair<FullSecID, const SpacePoint*> > goodSPs;
      for (const SpacePoint* aSP : tc.getHits()) {
        if (!acceptHit(*aSP)) continue;

        FullSecID fSecID = createSecID(aSP->getVxdID(), aSP->getNormalizedLocalU(), aSP->getNormalizedLocalV());
        if (fSecID.getFullSecID() == std::numeric_limits<unsigned int>::max())
        { B2ERROR("a secID for spacePoint not found!"); continue; }

        goodSPs.push_back({fSecID, aSP});
      }

      // catch tracks which have not enough accepted hits.
      B2DEBUG(10, "SecMapTrainer::storeTC: the TC has now nHits/threshold: " << goodSPs.size() << "/" << m_config.nHitsMin)
      if (goodSPs.size() < m_config.nHitsMin) return false;

      // want to have hits going from outer to inner ones
      if (tc.isOutgoing()) std::reverse(goodSPs.begin(), goodSPs.end());

      convertSP2TC(goodSPs, iD, tc.getMomSeed().Perp(), tc.getPdgCode());

      return true;
    }



    /** for given normalized local variables and VxdID a FullSecID is determined.
     * returns unsigned int max if correct ID could not be found. */
    FullSecID createSecID(VxdID iD, double uVal, double vVal)
    {
      // TODO replace by new secMap-design-approach
      auto secID = SectorTools::calcSecID(m_config.uDirectionCuts, m_config.vDirectionCuts, {uVal, vVal});
      if (secID == std::numeric_limits<unsigned short>::max())
        return FullSecID(std::numeric_limits<unsigned int>::max());
      return FullSecID(iD, false, secID);
    }



    /** takes all TCs stored and processed them for root storing.
     * Takes care of cleaning event-dependent TCs. */
    unsigned processTracks()
    {
      unsigned nTwoHitResults = 0;
      unsigned nThreeHitResults = 0;
      unsigned nFourHitResults = 0;
      unsigned nTracksProcessed = m_tcs.size();

      for (const auto& tc : m_tcs) {
        // two hit:
        nTwoHitResults += process2HitCombinations(tc);
        // three hit:
        nThreeHitResults += process3HitCombinations(tc);
        //four hit:
        nFourHitResults += process4HitCombinations(tc);
      }

      m_tcs.clear();

      return nTracksProcessed;
    }
  };
}

