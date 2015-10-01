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
#include <tracking/trackFindingVXD/sectorMapTools/SecMapTrainDataSet.h>
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


    /** A standard mask for the data to be collected for the root-output. */
    SecMapTrainDataSet m_dataSet;



    /** checks if given Hit is acceptable for this trainer. */
    bool acceptHit(const SpacePoint& hit)
    {
      bool accepted(false);

      B2Vector3D distance2IP =
        m_config.vIP - B2Vector3D(hit.getPosition().X(), hit.getPosition().Y(), hit.getPosition().Z());

      if (m_config.hitMinMaxDist2IP3D.first < distance2IP.Mag()
          or m_config.hitMinMaxDist2IP3D.second > distance2IP.Mag())
      { return accepted; }

      auto layerID = hit.getVxdID().getLayerNumber();
      if (m_config.minMaxLayer.first < layerID
          or m_config.minMaxLayer.second > layerID)
      { return accepted; }

      accepted = true;

      return accepted;
    }



    /** processes all two-hit-combinations of given TC and stores their results. returns number of stored values. */
    unsigned process2HitCombinations(const SecMapTrainerTC& aTC)
    {
      unsigned nValues = 0;
      if (aTC.size() < 2) { return nValues; }
      m_dataSet.trackNo = aTC.getTrackID();
      m_dataSet.pdg = aTC.getPDG();

      // the iterators mark the hits to be used:
      std::vector<std::pair<std::string, double> > collectedResults;
      SecMapTrainerTC::ConstIterator outerIt = aTC.outermostHit();
      SecMapTrainerTC::ConstIterator innerIt = ++aTC.outermostHit();

      // loop over all 2-hit-combis, collect data for each filterType and store it in root-tree:
      for (; innerIt == aTC.innerEnd();) {
        XHitDataSet<std:: string, SecMapTrainerHit> hitBundle;
        hitBundle.sectorIDs = {outerIt->getSectorID(), innerIt->getSectorID()};
        hitBundle.hits = {outerIt, innerIt};
        m_dataSet.sectorIDs = hitBundle.sectorIDs;
        m_filterMill.grindData(hitBundle, collectedResults);

        for (const auto& entry : collectedResults) {
          m_dataSet.filterID = entry.first;
          m_dataSet.value = entry.second;

          m_rootInterface.fill(m_dataSet);
          ++nValues;
        }
        ++outerIt;
        ++innerIt;
      }
      return nValues;
    }



    /** processes all three-hit-combinations of given TC and stores their results. returns number of stored values. */
    unsigned process3HitCombinations(const SecMapTrainerTC& aTC)
    {
      unsigned nValues = 0;
      if (aTC.size() < 3) { return nValues; }
      m_dataSet.trackNo = aTC.getTrackID();
      m_dataSet.pdg = aTC.getPDG();

      // the iterators mark the hits to be used:
      std::vector<std::pair<std::string, double> > collectedResults;
      SecMapTrainerTC::ConstIterator outerIt = aTC.outermostHit();
      SecMapTrainerTC::ConstIterator centerIt = ++aTC.outermostHit();
      SecMapTrainerTC::ConstIterator innerIt = ++(++aTC.outermostHit());

      // loop over all 2-hit-combis, collect data for each filterType and store it in root-tree:
      for (; innerIt == aTC.innerEnd();) {
        XHitDataSet<std:: string, SecMapTrainerHit> hitBundle;
        hitBundle.sectorIDs = {outerIt->getSectorID(), centerIt->getSectorID(), innerIt->getSectorID()};
        hitBundle.hits = {&(*outerIt), &(*centerIt), &(*innerIt)};
        m_dataSet.sectorIDs = hitBundle.sectorIDs;
        m_filterMill.grindData(hitBundle, collectedResults);

        for (const auto& entry : collectedResults) {
          m_dataSet.filterID = entry.first;
          m_dataSet.value = entry.second;

          m_rootInterface.fill(m_dataSet);
        }
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
      if (aTC.size() < 4) { return nValues; }
      m_dataSet.trackNo = aTC.getTrackID();
      m_dataSet.pdg = aTC.getPDG();

      // the iterators mark the hits to be used:
      std::vector<std::pair<std::string, double> > collectedResults;
      SecMapTrainerTC::ConstIterator outerIt = aTC.outermostHit();
      SecMapTrainerTC::ConstIterator outerCenterIt = ++aTC.outermostHit();
      SecMapTrainerTC::ConstIterator innerCenterIt = ++(++aTC.outermostHit());
      SecMapTrainerTC::ConstIterator innerIt = ++(++(++aTC.outermostHit()));

      // loop over all 2-hit-combis, collect data for each filterType and store it in root-tree:
      for (; innerIt == aTC.innerEnd();) {
        XHitDataSet<std:: string, SecMapTrainerHit> hitBundle;
        hitBundle.sectorIDs = {
          outerIt->getSectorID(),
          outerCenterIt->getSectorID(),
          innerCenterIt->getSectorID(),
          innerIt->getSectorID()
        };
        hitBundle.hits = {
          outerIt,
          outerCenterIt,
          innerCenterIt,
          innerIt
        };
        m_dataSet.sectorIDs = hitBundle.sectorIDs;
        m_filterMill.grindData(hitBundle, collectedResults);

        for (const auto& entry : collectedResults) {
          m_dataSet.filterID = entry.first;
          m_dataSet.value = entry.second;

          m_rootInterface.fill(m_dataSet);
          ++nValues;
        }
        ++outerIt;
        ++innerIt;
      }
      return nValues;
    }



    /** converts the SPTC into a SecMapTrainerTC */
    bool convertSPTC(
      unsigned tcID,
      const SpacePointTrackCand& tc)
    {
      SecMapTrainerTC newTrack(tcID, tc.getMomSeed().Perp());

      // collect hits which fullfill all given tests
      std::vector<const SpacePoint*> goodSPs;
      for (const SpacePoint* aSP : tc.getHits()) {
        if (!acceptHit(*aSP)) continue;
        goodSPs.push_back(aSP);
      }

      // want to have hits going from outer to inner ones
      if (tc.isOutgoing()) std::reverse(goodSPs.begin(), goodSPs.end());

      for (const SpacePoint* aSP : goodSPs) {

        FullSecID fSeCID = createSecID(aSP->getVxdID(), aSP->getNormalizedLocalU(), aSP->getNormalizedLocalV());
        if (fSeCID.getFullSecID() == std::numeric_limits<unsigned int>::max())
        { B2ERROR("a secID for spacePoint not found!"); continue; }
        std::string fullSecID = fSeCID.getFullSecString();

        newTrack.addHit(std::move(
                          SecMapTrainerHit(fullSecID, aSP->getPosition())));
      }

      if (newTrack.size() < m_config.nHitsMin) return false;

      // add vertex (but without real vertexPosition, since origin is assumed)
      SecMapTrainerHit newVirtualHit(FullSecID().getFullSecString(), m_config.vIP);
//    newVirtualHit.setVertex();

      newTrack.addHit(std::move(newVirtualHit));

      m_tcs(std::move(newTrack));

      return true;
    }

  public:

    /** constructor. */
    SecMapTrainer(TrainerConfigData& configData) :
      m_config(configData),
      m_factory(
        configData.vIP.X(),
        configData.vIP.Y(),
        configData.vIP.Z(),
        configData.mField),
      m_filterMill()
    {
      // stretch the cuts:
      m_config.pTCuts.first -= m_config.pTCuts.first * m_config.pTSmear;
      m_config.pTCuts.second += m_config.pTCuts.second * m_config.pTSmear;
      // prepare the filters first:
      for (auto& fName : m_config.twoHitFilters) {
        auto filter = m_factory.get2HitInterface(fName);
        m_filterMill.add2HitFilter({fName, filter});
      }
      for (auto& fName : m_config.threeHitFilters) {
        auto filter = m_factory.get3HitInterface(fName);
        m_filterMill.add3HitFilter({fName, filter});
      }
      for (auto& fName : m_config.fourHitFilters) {
        auto filter = m_factory.get4HitInterface(fName);
        m_filterMill.add4HitFilter({fName, filter});
      }
      // prevent further modifications:
      m_filterMill.lockMill();
    }



    /** Initialize event. */
    void initializeEvent(int expNo, int runNo, int evtNo)
    {
      m_dataSet.expNo = expNo;
      m_dataSet.runNo = runNo;
      m_dataSet.evtNo = evtNo;
    }



    /** checks if given TC is acceptable for this trainer. */
    bool acceptTC(SpacePointTrackCand& tc, unsigned iD)
    {
      // catch TCS which are too short in any case
      if (tc.getNHits() < m_config.nHitsMin) return false;

      // catch TCs where more than one hit was on the same sensor
      if (tc.hasHitsOnSameSensor()) return false;

      // catch TCS where particle type is wrong
      for (const auto& pdg : m_config.pdgCodesAllowed) {
        if (tc.getPdgCode() == pdg) return false;
      }

      // check if momentum of TC is within range:
      auto pT = tc.getMomSeed().Perp();
      if (m_config.pTCuts.first > pT
          or m_config.pTCuts.second < pT) return false;

      // catch tracks which start too far away from orign
      B2Vector3D distance2IP = m_config.vIP - tc.getPosSeed();
      if (m_config.seedMaxDist2IPXY > 0
          and m_config.seedMaxDist2IPXY < distance2IP.Perp()) return false;
      if (m_config.seedMaxDist2IPZ > 0
          and m_config.seedMaxDist2IPXY < distance2IP.Z()) return false;

      return convertSPTC(tc, iD);
    }



    /** for given normalized local variables and VxdID a FullSecID is determined.
     * returns unsigned int max if correct ID could not be found. */
    FullSecID createSecID(VxdID iD, double uVal, double vVal)
    {
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
      // TODO debug output...

      m_tcs.clear();

      return nTracksProcessed;
    }
  };
}

