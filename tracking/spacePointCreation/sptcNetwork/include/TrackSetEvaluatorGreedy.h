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


namespace Belle2 {


  /** TrackSetEvaluator-Class for greedy algorithm.
   *
   * with given trackSet and manager for competitors, this class takes care that there are no competing tracks in the end.
   */
  template<class TCType, class CompetitorManagingType>
  class TrackSetEvaluatorGreedy : public TrackSetEvaluatorBase<TCType, CompetitorManagingType> {
  protected:
    typedef TrackSetEvaluatorBase<TCType, CompetitorManagingType> BaseClass;
    /** ************************* INTERNAL MEMBER FUNCTIONS ************************* */


    /** main Greedy function, returns number of TCs died.
     *
     * Algorithm:
     * search for nonOverlapping trackCandidates using Greedy algorithm:
     * - start with TC of highest QI
     * - remove all TCs incompatible with current TC
     * - if there are still TCs there
     * - repeat step until no incompatible TCs are there any more
     */
    unsigned int doGreedy(std::vector<TCType*>& overlappingTCs)
    {
      unsigned int countTCsAliveAtStart = overlappingTCs.size(), countSurvivors = 0, countKills = 0;
      double totalQI = 0, totalSurvivingQI = 0;

      B2INFO("doGreedy:b4Sorting:")
      for (TCType* aTC : overlappingTCs) {
        B2INFO("tc " << aTC->getID() << " got QI: " << aTC->getTrackQuality())
        totalQI += aTC->getTrackQuality();
      }
      std::sort(overlappingTCs.begin(), overlappingTCs.end(), [](const TCType * a, const TCType * b) -> bool { return *a > *b; });

      B2INFO("doGreedy:afterSorting:")
      for (TCType* aTC : overlappingTCs) {
        B2INFO("tc " << aTC->getID() << " got QI: " << aTC->getTrackQuality())
      }

      // start recursive greedy algorithm...
      greedyRecursive(0, overlappingTCs, totalSurvivingQI, countSurvivors, countKills);

      B2INFO("doGreedy: at begin of greedy algoritm: total number of TCs alive: " << countTCsAliveAtStart << " with totalQi: " << totalQI
             <<
             ", TCs survived: " << countSurvivors << ", TCs killed: " << countKills << ", survivingQI: " << totalSurvivingQI)
      return countKills;
    }


    /** recursive function which takes tc with highest QI and kills all its rivals.
     *
     * After that, TC gets removed and process is repeated with shrinking list of TCs until no TCs alive has got rivals alive
     */
    void greedyRecursive(unsigned int currentIndex,
                         std::vector<TCType*>& overlappingTCs,
                         double& totalSurvivingQI,
                         unsigned int& countSurvivors,
                         unsigned int& countKills)
    {
      // if end of container is reached: end greedy recursive for good.
      if ((currentIndex < overlappingTCs.size()) == false) return;

      // bypass all dead entries, skip if end of container is reached:
      while (overlappingTCs.size() > currentIndex and (overlappingTCs[currentIndex]->isAlive() == false)) {
        currentIndex++;
        if (currentIndex == overlappingTCs.size()) { return; }
      }

      countSurvivors++;
      totalSurvivingQI += overlappingTCs[currentIndex]->getTrackQuality();

      // kill all remaining competitors of current TC (all should have a smaller QI than this one...)
      for (unsigned int competitorID : BaseClass::m_manager.getCompetitors(currentIndex)) {
        BaseClass::m_trackSet[competitorID].setAliveState(false);
        countKills++;
      }

      currentIndex++;

      greedyRecursive(currentIndex, overlappingTCs, totalSurvivingQI, countSurvivors, countKills);

      return;
    }


  public:
    /** constructor */
    TrackSetEvaluatorGreedy(std::vector<TCType>& trackSet, CompetitorManagingType& manager) : BaseClass(trackSet, manager) {}


    /** destructor */
    virtual ~TrackSetEvaluatorGreedy() {}


    /** main function. returns number of final TCs */
    virtual unsigned int cleanOverlaps()
    {
      unsigned int countedTCsDied = 0;
      std::vector<TCType*> overlaps = BaseClass::getOverlappingTCs();

      // deal with simple cases:
      if (overlaps.size() < 3) {
        if (overlaps.size() == 2) {
          BaseClass::tcDuel(overlaps);
          return BaseClass::getNTCs() - 1;
        } else if (overlaps.size() == 0) {
          return BaseClass::getNTCs();
        }
        B2ERROR("TrackSetEvaluatorGreedy::cleanOverlaps: nOverlapping TCs is " << overlaps.size() <<
                " which is illegal. Stopping cleaning overlaps!")
        return BaseClass::getNTCs();
      }

      // executing actual algorithm:
      countedTCsDied = BaseClass::getNTCs() - doGreedy(overlaps);
      return countedTCsDied;
    }

  };

} // end namespace Belle2
