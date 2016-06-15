/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *

 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once


#include <tracking/spacePointCreation/sptcNetwork/TrackSetEvaluatorBase.h>

#include <framework/logging/Logger.h>

#include <vector>
#include <list>
#include <functional>


namespace Belle2 {


  /** TrackSetEvaluator-Class for greedy algorithm.
   *
   * with given trackSet and manager for competitors, this class takes care that there are no competing tracks in the end.
   */
  template<class TCType, class CompetitorManagingType>
  class TrackSetEvaluatorGreedy : public TrackSetEvaluatorBase<TCType, CompetitorManagingType> {
  protected:
    /** simple typedef to increase readability. BaseClass simply means the base class of this one */
    typedef TrackSetEvaluatorBase<TCType, CompetitorManagingType> BaseClass;

    /** ************************* INTERNAL MEMBER FUNCTIONS ************************* */


    /** main Greedy function, returns true if it was successful.
     *
     * Algorithm:
     * search for nonOverlapping trackCandidates using Greedy algorithm:
     * - start with TC of highest QI
     * - remove all TCs incompatible with current TC
     * - if there are still TCs there
     * - repeat step until no incompatible TCs are there any more
     */
    bool doGreedy(std::vector<TCType*>& overlappingTCs)
    {
      using namespace std;
      B2DEBUG(25, "doGreedy:b4Sorting:\n" << BaseClass::miniPrinter(overlappingTCs));

      // sort that TC with highest QI comes first
      std::sort(overlappingTCs.begin(), overlappingTCs.end(), [](const TCType * a, const TCType * b) -> bool { return *a > *b; });

      B2DEBUG(25, "doGreedy:afterSorting:\n" << BaseClass::miniPrinter(overlappingTCs));

      // start recursive greedy algorithm...
      bool wasSuccsessfull = greedyRecursive(0, overlappingTCs);

      if (wasSuccsessfull) {
        B2DEBUG(50, "doGreedy: at end of greedy algoritm: total number of TCs alive: " << BaseClass::checkAtEnd() <<
                "\n Overview:\n" << BaseClass::miniPrinter(overlappingTCs));
        return true;
      }
      B2WARNING("doGreedyRecursive aborted!" << BaseClass::checkAtEnd() << "\n Overview:\n" << BaseClass::miniPrinter(overlappingTCs));
      return false;
    }


    /** recursive function which takes tc with highest QI and kills all its rivals. returns true, if it was successful
     *
     * After that, TC gets removed and process is repeated with shrinking list of TCs until no TCs alive has got rivals alive
    * TODO: currently no abort-conditions implemented! -> returns always true!
     */
    inline bool greedyRecursive(unsigned int currentIndex,
                                std::vector<TCType*>& overlappingTCs)
    {
      B2DEBUG(50, "doGreedyRecursive-start: current index: " << currentIndex << ", fullList:\n" << BaseClass::miniPrinter(overlappingTCs));
      // if end of container is reached: end greedy recursive for good.
      if ((currentIndex < overlappingTCs.size()) == false) return true;

      // bypass all dead entries, skip if end of container is reached:
      while (overlappingTCs.size() > currentIndex and (overlappingTCs[currentIndex]->isAlive() == false)) {
        B2DEBUG(50, "doGreedyRecursive-while-loop: current index: " << currentIndex << " is dead, skipping...");
        currentIndex++;
        if (currentIndex == overlappingTCs.size()) { return true; }
      }

      auto vecPrint = [](const std::vector<unsigned int>& vec) -> std::string { std::string out = "competitor:"; for (auto iD : vec) { out += " " + std::to_string(iD) + "," ; } return out; };
      B2DEBUG(50, "\ndoGreedyRecursive before killing stuff: these are the competitors of index (overlap/total) " << currentIndex
              << "/" << overlappingTCs[currentIndex]->getID() << ":\n" << vecPrint(BaseClass::m_manager.getCompetitors(
                    overlappingTCs[currentIndex]->getID())) << " (in total index)\n");

      // copy competitor-IDs to prevent undefined behavior:
      auto competitors = BaseClass::m_manager.getCompetitors(overlappingTCs[currentIndex]->getID());
      // kill all remaining competitors of current TC (all should have a smaller QI than this one...)
      for (unsigned int competitorID : competitors) {
        B2DEBUG(50, "\ndoGreedyRecursive-killCompetitors: index (overlap/total) " << currentIndex << "/" <<
                overlappingTCs[currentIndex]->getID() <<
                " and QI " << overlappingTCs[currentIndex]->getTrackQuality() <<
                " got competitor with index (total, _not_ overlap!) " << competitorID <<
                " and QI " << BaseClass::m_trackSet[competitorID].getTrackQuality() <<
                ". The competitor " << competitorID << " got following competitors itself:\n" << vecPrint(BaseClass::m_manager.getCompetitors(
                      competitorID)) << "\n");

        // warning currentIndex is running in overlap-system, but competitorID is in total system!
        BaseClass::m_trackSet[competitorID].setAliveState(false);
      }

      B2DEBUG(50, "doGreedyRecursive-after killing competitors: current index: " << currentIndex
              << " has now " <<  BaseClass::m_manager.getCompetitors(overlappingTCs[currentIndex]->getID()).size() << " competitors (should be 0)"
              << ", fullList:\n" << BaseClass::miniPrinter(overlappingTCs));

      currentIndex++;

      return greedyRecursive(currentIndex, overlappingTCs);
    }


  public:
    /** constructor */
    TrackSetEvaluatorGreedy(std::vector<TCType>& trackSet, CompetitorManagingType& manager) : BaseClass(trackSet, manager) {}


    /** destructor */
    virtual ~TrackSetEvaluatorGreedy() {}


    /** main function. returns true, if clean overlaps was successfull and returns false if not.
    *
    * after executing cleanOverlaps, the counters are updated.
    * These counters can give you a clue why there was a problem.
    */
    virtual bool cleanOverlaps()
    {
      std::string result = BaseClass::checkAtStart();
      B2DEBUG(25, "TrackSetEvaluatorGreedy::cleanOverlaps: " << result);

      std::vector<TCType*> overlaps = BaseClass::getOverlappingTCs();
      unsigned int nOverlaps = overlaps.size();

      if (nOverlaps == 0) return true;  // nothing to be done, no checks needed

      // deal with simple cases first (contains many safety checks, which can therefore be ommitted by the actual algorithms):
      bool wasSimpleCase = BaseClass::dealWithSimpleCases(overlaps);

      if (wasSimpleCase) {
        return BaseClass::doSanityChecks(true, overlaps.size(), "was easy case");
      }

      // executing actual algorithm:
      bool wasSuccsessfull = doGreedy(overlaps);

      return BaseClass::doSanityChecks(wasSuccsessfull, nOverlaps, "Greedy algorithm");
    }
  };

} // end namespace Belle2
