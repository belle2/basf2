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
#include <tracking/dataobjects/SectorMapConfig.h>
#include <tracking/trackFindingVXD/environment/VXDTFFilters.h>

#include <tracking/trackFindingVXD/sectorMapTools/FilterMill.h>
#include <tracking/trackFindingVXD/sectorMapTools/RawSecMapRootInterface.h>
//#include <tracking/trackFindingVXD/sectorMapTools/SectorTools.h>
#include <tracking/trackFindingVXD/filterMap/map/FiltersContainer.h>
#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariableNamesToFunctions.h>
#include <framework/geometry/B2Vector3.h>

#include <tracking/dataobjects/FullSecID.h>
#include <vxd/dataobjects/VxdID.h>

#include <string>
#include <vector>
#include <utility> // std::pair, std::move
#include <functional>
#include <limits>       // std::numeric_limits


namespace Belle2 {

  /** This class contains all relevant tools for training a VXDTFFilters. */

  template <class FilterFactoryType>
  class SecMapTrainer {
  protected:
    /** name of the setting to be used for this training */
    const std::string m_nameSetup;

    /** a reference to the singleton FiltersContainer used for this training. Needed as some information are
    only accessable from the filters (in the container).  */
    FiltersContainer<SpacePoint>& m_filtersContainer = Belle2::FiltersContainer<SpacePoint>::getInstance();

    /** Contains all relevant configurations needed for training a sectorMap.
    Copy of the config in the SectorMap needed as it is modified (the one in VXDTFFilters is const).  */
    SectorMapConfig m_config;

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
      bool found = false;
      std::string ids = "";
      for (auto allowedLayer : m_config.allowedLayers) {
        ids += std::to_string(allowedLayer) + " ";
        if (allowedLayer == layerID) found = true;
      }
      B2DEBUG(10, "SecMapTrainer::acceptHit: the TC has layerID: " << layerID << " and allowd layers: " << ids << " and was " <<
              (found ? "accepted" : "rejected"));

      return found;
    }



    /** processes all two-hit-combinations of given TC and stores their results. returns number of stored values. */
    unsigned process2HitCombinations(const SecMapTrainerTC& aTC)
    {
      unsigned nValues = 0;
      B2DEBUG(10, "SecMapTrainer::process2HitCombinations: nHits/trackID/pdg: " << aTC.size() << "/" << aTC.getTrackID() << "/" <<
              aTC.getPDG());
      if (aTC.size() < 2) { return nValues; }

      // the iterators mark the hits to be used:
      SecMapTrainerTC::ConstIterator outerIt = aTC.outermostHit();
      SecMapTrainerTC::ConstIterator innerIt = ++aTC.outermostHit();

      // loop over all 2-hit-combis, collect data for each filterType and store it in root-tree:
      std::vector<std::pair<std::string, double> > collectedResults;
      for (; innerIt != aTC.innerEnd();) {
        B2DEBUG(10, "SecMapTrainer::process2HitCombinations: outerHit-/innerHitSecID: " << outerIt->getSectorIDString() <<
                "/" << innerIt->getSectorIDString());

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
          B2DEBUG(50, "SecMapTrainer::process2HitCombinations: filter/value: " << entry.first << "/" << entry.second);
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
              aTC.getPDG());
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
                "/" << innerIt->getSectorIDString());

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
          B2DEBUG(50, "SecMapTrainer::process3HitCombinations: filter/value: " << entry.first << "/" << entry.second);
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


    /** converts the SpacePoints into a SecMapTrainerTC */
    void convertSP2TC(
      std::vector<std::pair< FullSecID, const SpacePoint*> >& goodSPs,
      unsigned tcID, double pTValue, int pdgCode)
    {
      B2DEBUG(10, "SecMapTrainer::convertSPTC: nGoodHits: " << goodSPs.size());

      SecMapTrainerTC newTrack(tcID, pTValue, pdgCode);

      for (const auto& secIDAndSPpair : goodSPs) {
        FullSecID fullSecID = secIDAndSPpair.first;
        B2DEBUG(20, "SecMapTrainer::convertSPTC: found fullSecID: " << fullSecID.getFullSecString());

        newTrack.addHit(SecMapTrainerHit(fullSecID,  *secIDAndSPpair.second));
      }

      // add vertex (but without real vertexPosition, since origin is assumed)
      SecMapTrainerHit newVirtualHit(FullSecID().getFullSecString(), m_config.vIP);

      newTrack.addHit(std::move(newVirtualHit));

      m_tcs.push_back(std::move(newTrack));
    }


  public:

    /** constructor. */
    SecMapTrainer(std::string setupName , std::string appendix = "") :
      m_nameSetup(setupName),
      m_config(m_filtersContainer.getFilters(m_nameSetup)->getConfig()),
      m_factory(
        m_config.vIP.X(),
        m_config.vIP.Y(),
        m_config.vIP.Z(),
        m_config.mField),

      m_filterMill(),
      m_rootInterface(m_config.secMapName, appendix),
      m_expNo(std::numeric_limits<unsigned>::max()),
      m_runNo(std::numeric_limits<unsigned>::max()),
      m_evtNo(std::numeric_limits<unsigned>::max())
    {
      // stretch the cuts:
//       m_config.pTCuts.first -= m_config.pTCuts.first * m_config.pTSmear;
//       m_config.pTCuts.second += m_config.pTCuts.second * m_config.pTSmear;
      m_config.pTmin -= m_config.pTmin * m_config.pTSmear;
      m_config.pTmax += m_config.pTmax * m_config.pTSmear;
    }


    /** initialize the trainer (to be called in Module::initialize(). */
    void initialize()
    {
      // prepare the filters!
      // 2 space points:
      auto TwoSPfilterNamesToFunctions(SelectionVariableNamesToFunctions(VXDTFFilters<SecMapTrainerHit>::twoHitFilter_t()));

      std::vector< std::string> twoHitFilters;
      for (const auto& filterNameToFunction : TwoSPfilterNamesToFunctions)
        twoHitFilters.push_back(filterNameToFunction.first);

      m_rootInterface.initialize2Hit(twoHitFilters);
      for (auto& nameToFunction : TwoSPfilterNamesToFunctions)
        m_filterMill.add2HitFilter(nameToFunction);


      // 3 space points:
      auto ThreeSPfilterNamesToFunctions(SelectionVariableNamesToFunctions(
                                           VXDTFFilters<SecMapTrainerHit>::threeHitFilter_t()));

      std::vector< std::string> threeHitFilters;
      for (const auto& filterNameToFunction : ThreeSPfilterNamesToFunctions)
        threeHitFilters.push_back(filterNameToFunction.first);

      m_rootInterface.initialize3Hit(threeHitFilters);
      for (auto& nameToFunction : ThreeSPfilterNamesToFunctions)
        m_filterMill.add3HitFilter(nameToFunction);
      for (auto& nameToFunction : ThreeSPfilterNamesToFunctions)
        m_filterMill.add3HitFilter(nameToFunction);


      // 4 space points apparently unused
      // m_rootInterface.initialize4Hit(m_config.fourHitFilters);

      // auto FourSPfilterNamesToFunctions( SelectionVariableNamesToFunctions(
      //                     VXDTFFilters<SecMapTrainerHit>::fourHitFilter_t() ));

      // std::vector< std::string> fourHitFilters;
      // for (const auto& filterNameToFunction :FourSPfilterNamesToFunctions )
      //  fourHitFilters.push_back( filterNameToFunction.first );

      // m_rootInterface.initialize4Hit(fourHitFilters);
      // for (auto& nameToFunction : FourSPfilterNamesToFunctions)
      //  m_filterMill.add4HitFilter(nameToFunction);

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
    const SectorMapConfig& getConfig() { return m_config; }


    /** returns the name of the setup used for this trainer */
    const std::string getSetupName() { return m_nameSetup; }

    /** checks if given TC is acceptable for this trainer and store it if it is accepted.
    *
    * The return value is true, it was accepted and stored, false if not.
    */
    bool storeTC(const SpacePointTrackCand& tc, unsigned iD)
    {
      B2DEBUG(10, "SecMapTrainer::storeTC: nHits/threshold: " << tc.getNHits() << "/" << m_config.nHitsMin);
      // catch TCS which are too short in any case
      if (tc.getNHits() < m_config.nHitsMin) return false;

      B2DEBUG(10, "SecMapTrainer::storeTC: hasHitsOnSameSensor: " << tc.hasHitsOnSameSensor());
      // catch TCs where more than one hit was on the same sensor
      if (tc.hasHitsOnSameSensor()) return false;

      // catch TCS where particle type is wrong
      bool found = false;
      for (const auto& pdg : m_config.pdgCodesAllowed) {
        if (tc.getPdgCode() == pdg) found = true;
      }
      if (found == false and m_config.pdgCodesAllowed.empty() == false) return false;

      // check if momentum of TC is within range:
      auto pT = tc.getMomSeed().Perp();
      B2DEBUG(10, "SecMapTrainer::storeTC: pT/thresholdmin/-max: " << pT << "/" << m_config.pTmin << "/" <<
              m_config.pTmax);
      if (m_config.pTmin > pT or m_config.pTmax < pT) return false;

      // catch tracks which start too far away from orign
      B2Vector3D distance2IP = m_config.vIP - tc.getPosSeed();
      B2DEBUG(10, "SecMapTrainer::storeTC: distance2IP/thresholdXY/-Z: " << distance2IP.Mag() << "/" << m_config.seedMaxDist2IPXY << "/"
              << m_config.seedMaxDist2IPZ);
      if (m_config.seedMaxDist2IPXY > 0
          and m_config.seedMaxDist2IPXY < distance2IP.Perp()) return false;
      if (m_config.seedMaxDist2IPZ > 0
          and m_config.seedMaxDist2IPXY < distance2IP.Z()) return false;

      // collect hits which fullfill all given tests
      std::vector<std::pair<FullSecID, const SpacePoint*> > goodSPs;
      for (const SpacePoint* aSP : tc.getHits()) {
        if (!acceptHit(*aSP)) continue;

        FullSecID fSecID = m_filtersContainer.getFilters(m_nameSetup)->getFullID(aSP->getVxdID(),
                           aSP->getNormalizedLocalU(), aSP->getNormalizedLocalV());
        if (fSecID.getFullSecID() == std::numeric_limits<unsigned int>::max())
        { B2ERROR("a secID for spacePoint not found!"); continue; }

        goodSPs.push_back({fSecID, aSP});
      }

      // catch tracks which have not enough accepted hits.
      B2DEBUG(10, "SecMapTrainer::storeTC: the TC has now nHits/threshold: " << goodSPs.size() << "/" << m_config.nHitsMin);
      if (goodSPs.size() < m_config.nHitsMin) return false;

      // want to have hits going from outer to inner ones
      if (tc.isOutgoing()) std::reverse(goodSPs.begin(), goodSPs.end());

      convertSP2TC(goodSPs, iD, tc.getMomSeed().Perp(), tc.getPdgCode());

      return true;
    }



    /** for given normalized local variables and VxdID a FullSecID is determined.
     * returns unsigned int max if correct ID could not be found. */
    /*
    FullSecID createSecID(VxdID iD, double uVal, double vVal)
    {
      // TODO replace by new secMap-design-approach
      std::vector<double> uTemp = {0.};
      uTemp.insert(uTemp.end(), m_config.uSectorDivider.begin(), m_config.uSectorDivider.end());
      std::vector<double> vTemp = {0.};
      vTemp.insert(vTemp.end(), m_config.vSectorDivider.begin(), m_config.vSectorDivider.end());
      auto secID = SectorTools::calcSecID(uTemp, vTemp, {uVal, vVal});
      if (secID == std::numeric_limits<unsigned short>::max())
        return FullSecID(std::numeric_limits<unsigned int>::max());
      return FullSecID(iD, false, secID);
    }
    */


    /** takes all TCs stored and processed them for root storing.
     * Takes care of cleaning event-dependent TCs. */
    unsigned processTracks()
    {
      unsigned n2HitResults = 0;
      unsigned n3HitResults = 0;
      unsigned n4HitResults = 0;
      unsigned nTracksProcessed = m_tcs.size();

      for (const auto& tc : m_tcs) {
        // two hit:
        n2HitResults += process2HitCombinations(tc);
        // three hit:
        n3HitResults += process3HitCombinations(tc);
      }

      m_tcs.clear();
      B2DEBUG(25, "SecMapTrainer::processTracks: nStoredValues for 2-/3-/4-hit: " << n2HitResults << "/" << n3HitResults << "/" <<
              n4HitResults);

      return nTracksProcessed;
    }
  };
}

