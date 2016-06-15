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


#include <framework/logging/Logger.h>

#include <vector>
#include <list>


namespace Belle2 {


  /** Base Class for all TrackSetEvaluators.
   *
   * with given trackSet and manager for competitors, this class takes care that there are no competing tracks in the end.
   */
  template<class TCType, class CompetitorManagingType>
  class TrackSetEvaluatorBase {
  protected:
    /** ************************* DATA MEMBERS ************************* */

    /** link to TrackSet to be evaluated */
    std::vector<TCType>& m_trackSet;

    /** this member takes care for the competing tracks in given set */
    CompetitorManagingType& m_manager;

    /** for cross-checking: sums up all QI-values of given TCs at start */
    double m_totalQI;

    /** for cross-checking: sums up all QI-values of given TCs surviving the evaluation */
    double m_totalSurvivingQI;

    /** for cross-checking: counts number of TCs alive at start */
    unsigned int m_nTCsAliveAtStart;

    /** for cross-checking: counts number of TCs surviving the evaluating phase */
    unsigned int m_nSurvivors;

    /** for cross-checking: counts number of TCs competing with each other at start */
    unsigned int m_nCompetitorsAtStart;

    /** for cross-checking: counts number of TCs competing with each other surviving the evaluating phase NOTE: if value is not 0 after cleanOverlaps, the evaluation did not work! */
    unsigned int m_nFinalCompetitors;

    /** ************************* INTERNAL MEMBER FUNCTIONS ************************* */


    /** DEBUG: mini helper function for printing. */
    std::string miniPrinter(std::vector<TCType*> vec)
    {
      std::string out;
      for (auto tc : vec) { out += "tc (alive:" + std::to_string(tc->isAlive()) + ") " + std::to_string(tc->getID()) + " got QI: " + std::to_string(tc->getTrackQuality()) + "\n" ; }
      return out;
    }


    /** provides a complete list of overlapping TCs */
    inline virtual std::vector<TCType*> getOverlappingTCs()
    {
      std::vector<TCType*> overlappingTCs;

      for (unsigned int index = 0; index < m_trackSet.size(); ++index) {
//    m_trackSet[index].isAlive() and m_manager.hasCompetitors(index) and overlappingTCs.push_back(&m_trackSet[index]);
        if (m_trackSet[index].isAlive() and m_manager.hasCompetitors(index)) {
          overlappingTCs.push_back(&m_trackSet[index]);
        }
      }
      B2DEBUG(50, "TrackSetEvaluatorBase::getOverlappingTCs: in a trackset of size " << m_trackSet.size() << ", " << overlappingTCs.size()
              << " overlapping TCs were found ");

      return overlappingTCs;
    }


    /** to be called if only two TCs are overlapping */
    inline virtual void tcDuel(std::vector<TCType*>& overlappingTCs)
    {
      if (overlappingTCs.size() != 2) {
        B2ERROR("TrackSetEvaluator::tcDuel: vector of overlapping TCs has wrong size: " << overlappingTCs.size() <<
                ", should be 2! skipping rest...");
        return;
      }

      B2DEBUG(25, "TrackSetEvaluator::tcDuel: first competitor got QI: " << overlappingTCs.front()->getTrackQuality() <<
              ", and second one got: " << overlappingTCs.back()->getTrackQuality());
      if (overlappingTCs.front()->getTrackQuality() > overlappingTCs.back()->getTrackQuality()) {
        overlappingTCs.back()->setAliveState(false);
      } else {
        overlappingTCs.front()->setAliveState(false);
      }
    }


    /** to be called to find out if there are TCs which are overlapping but not with each other - returns number of compatible TCs */
    inline virtual unsigned int findCompatibleOverlaps(std::vector<TCType*>& overlappingTCs) const
    {
      unsigned nOverlaps = overlappingTCs.size();
      unsigned nCompatibleTCs = 0;

      for (unsigned int iTC = 0; iTC < nOverlaps; iTC++) {
        for (unsigned int jTC = iTC + 1; jTC < nOverlaps; jTC++) {
          //      (m_manager.areCompetitors(overlappingTCs[iTC]->getID(), overlappingTCs[jTC]->getID()) == false) and nCompatibleTCs++;
          if (m_manager.areCompetitors(overlappingTCs[iTC]->getID(), overlappingTCs[jTC]->getID()) == false) { nCompatibleTCs++; }
        }
      }
      return nCompatibleTCs;
    }


    /** to be called if only the best overlapping TC shall survive.
     *
     * WARNING: passed parameter will be changed (sorting, setAliveState(false);
     * whether there are any compatible overlappers or not will be ignored.
     */
    inline virtual void letThebestTCSurvive(std::vector<TCType*>& overlappingTCs)
    {
      B2INFO("TrackSetEvaluatorBase::letThebestTCSurvive was triggered! Best QI is: " << overlappingTCs.front()->getTrackQuality());
      // sort that TC with highest QI comes first
      std::sort(overlappingTCs.begin(), overlappingTCs.end(), [](const TCType * a, const TCType * b) -> bool { return *a > *b; });

      for (unsigned int i = 1; i < overlappingTCs.size(); i++) {
        overlappingTCs[i]->setAliveState(false);
      }
    }


    /** for a given vector of overlapping TCs simple cases will be detected and executed.
     *
     * Returns true if there was a simple case, false if otherwise.
     */
    virtual bool dealWithSimpleCases(std::vector<TCType*>& overlappingTCs)
    {
      B2DEBUG(10, "TrackSetEvaluatorBase::dealWithSimpleCases was triggered! nOverlaps: " << overlappingTCs.size() <<
              " checking now for 2 TCs overlapping:");
      /* case:
       * only 2 TCs are overlapping.
       * -> tcDuel
       */
      if (overlappingTCs.size() < 3) {
        if (overlappingTCs.size() == 2) {
          tcDuel(overlappingTCs);
          return true;
        }
        B2ERROR("TrackSetEvaluator::dealWithSimpleCases: nOverlapping TCs is " << overlappingTCs.size() <<
                " which is illegal. Stopping cleaning overlaps!");
        return false;
      }

      unsigned int nCompatibleTCs = findCompatibleOverlaps(overlappingTCs);
      B2DEBUG(10, "TrackSetEvaluatorBase::dealWithSimpleCases: checking now for compatible overlaps... (found: " << nCompatibleTCs << ")");
      /* case:
       * Several TCs are overlapping but all of them are from the same "tree".
       * Means that all of them are incompatible with each other
       * -> findCompatibleOverlappers
       * -> letThebestTCSurvive
       */
      if (nCompatibleTCs == 0) {
        letThebestTCSurvive(overlappingTCs);  // only the best survived
        return true;
      }

      return false;
    }


    /** loops over network and sums up QI of all TCs alive at time of function call. */
    inline double sumQI() const
    {
      double qiSum = 0;
      for (const auto& aTC : m_trackSet) { if (aTC.isAlive()) qiSum += aTC.getTrackQuality(); }
      return qiSum;
    }


    /** counts nTCs which are alive at time of function call. */
    inline unsigned int countTCsAlive() const
    {
      unsigned int nAlive = 0;
      for (const auto& aTC : m_trackSet) { if (aTC.isAlive()) nAlive++; }
      return nAlive;
    }


    /** checks relevant parameters - to be called before cleanOverlaps.
     * WARNING updates internal counters totalQI, nCompetitorsAtStart and nTCsAliveAtStart! */
    inline virtual std::string checkAtStart()
    {
      m_totalQI = sumQI();
      m_nCompetitorsAtStart = m_manager.countCompetitors();
      m_nTCsAliveAtStart = countTCsAlive();
      return " checkAtStart: totalQI " + std::to_string(m_totalQI) +
             ", nCompetitors " + std::to_string(m_nCompetitorsAtStart) +
             ", nTCs alive " + std::to_string(m_nTCsAliveAtStart);
    }


    /** checks relevant parameters - to be called after cleanOverlaps.
     * WARNING updates internal counters totalSurvivingQI, nFinalCompetitors and nSurvivors! */
    inline virtual std::string checkAtEnd()
    {
      m_totalSurvivingQI = sumQI();
      m_nFinalCompetitors = m_manager.countCompetitors();
      m_nSurvivors = countTCsAlive();
      return " checkAtEnd: totalQI " + std::to_string(m_totalSurvivingQI) +
             ", nCompetitors " + std::to_string(m_nFinalCompetitors) +
             ", nTCs alive " + std::to_string(m_nSurvivors) +
             ", nTCs killed " + std::to_string(m_nTCsAliveAtStart - m_nSurvivors);
    }


    /** does some sanity checks on the result of the evaluator. to be called after evaluation took place.
     *
     * Parameters:
     * wasSuccsessfull = return value of evaluator function
     * nOverlaps = number of overlapping TCs
     * evalType = name of the evaluator
     */
    bool doSanityChecks(bool wasSuccsessfull, unsigned int nOverlaps, std::string evalType = "evaluatorType")
    {
      std::string result = checkAtEnd();

      // case: all competitors died
      if ((nOverlaps != 0) and (m_nSurvivors + nOverlaps) < (m_trackSet.size() + 1)) {
        B2WARNING("TrackSetEvaluatorHopfieldNN::cleanOverlaps: " << evalType <<
                  " claims success but all former competitors are dead! " << result);
        return false;
      }

      // case: everything seems to be fine
      if (wasSuccsessfull and (m_nFinalCompetitors == 0)) {
        B2DEBUG(25, "TrackSetEvaluatorHopfieldNN::cleanOverlaps: " << evalType <<
                " was successfully completed. " << result);
        return true;
      }

      // case: hopfield itself detected an issue
      if (!wasSuccsessfull) {
        B2WARNING("TrackSetEvaluatorHopfieldNN::cleanOverlaps: " << evalType <<
                  " was not successfully finished! " << result);
        return false;
      }

      // case: something else went wrong
      B2WARNING("TrackSetEvaluatorHopfieldNN::cleanOverlaps: " << evalType <<
                " claims success but number of competitors is still not zero! " << result);
      return false;
    }


  public:

    /** constructor */
    TrackSetEvaluatorBase(std::vector<TCType>& trackSet, CompetitorManagingType& manager) :
      m_trackSet(trackSet),
      m_manager(manager),
      m_totalQI(0),
      m_totalSurvivingQI(0),
      m_nTCsAliveAtStart(0),
      m_nSurvivors(0),
      m_nCompetitorsAtStart(0),
      m_nFinalCompetitors(0) {}

    virtual ~TrackSetEvaluatorBase() {}


    /** main function. returns true, if clean overlaps was successfull and returns false if not.
    *
    * after executing cleanOverlaps, the counters are updated.
    * These counters can give you a clue why there was a problem.
    */
    virtual bool cleanOverlaps()
    {
      B2ERROR(checkAtStart() << "\nTrackSetEvaluatorBase::cleanOverlaps() was called!" <<
              " This function does actually nothing, you sure you didn't want to pass an inheriting trackSetEvaluator to the network instead?" <<
              " These Classes can really clean overlaps in the network!\n" << checkAtEnd());
      return false;
    }

/// simple getters

    /** simply return number of TCs in given network */
    virtual unsigned int getNTCs() const { return m_trackSet.size(); }

    /** for cross-checking: returns sum of all QI-values of given TCs at start - result only valid if cleanOverlaps was executed */
    double getTotalQI() const { return m_totalQI; }

    /** for cross-checking: returns sum of all QI-values of given TCs surviving the evaluation - result only valid if cleanOverlaps was executed */
    double getTotalSurvivingQI() const { return m_totalSurvivingQI; }

    /** for cross-checking: returns counted number of TCs alive at start - result only valid if cleanOverlaps was executed */
    unsigned int getNTCsAliveAtStart() const { return m_nTCsAliveAtStart; }

    /** for cross-checking: returns counted number of TCs surviving the evaluating phase (result of all TCs in network, not overlapping only) - result only valid if cleanOverlaps was executed */
    unsigned int getNSurvivors() const { return m_nSurvivors; }

    /** for cross-checking: returns counted number of TCs competing with each other at start - result only valid if cleanOverlaps was executed */
    unsigned int getNCompetitorsAtStart() const { return m_nCompetitorsAtStart; }

    /** for cross-checking: returns counted number of TCs competing with each other surviving the evaluating phase - result only valid if cleanOverlaps was executed NOTE: if value is not 0 the evaluation did not work! */
    unsigned int getNFinalCompetitors() const { return m_nFinalCompetitors; }
  };

} // end namespace Belle2
