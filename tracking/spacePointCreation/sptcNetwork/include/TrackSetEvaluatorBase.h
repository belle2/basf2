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

    /** ************************* INTERNAL MEMBER FUNCTIONS ************************* */


    /** provides a complete list of overlapping TCs */
    virtual std::vector<TCType*> getOverlappingTCs()
    {
      std::vector<TCType*> overlappingTCs;

      for (unsigned int index = 0; index < m_trackSet.size(); ++index) {
//    m_trackSet[index].isAlive() and m_manager.hasCompetitors(index) and overlappingTCs.push_back(&m_trackSet[index]);
        if (m_trackSet[index].isAlive() and m_manager.hasCompetitors(index)) {
          overlappingTCs.push_back(&m_trackSet[index]);
        }
      }
      B2DEBUG(50, "TrackSetEvaluatorBase::getOverlappingTCs: in a trackset of size " << m_trackSet.size() << ", " << overlappingTCs.size()
              <<
              " overlapping TCs were found ")

      return overlappingTCs;
    }


    /** to be called if only two TCs are overlapping */
    virtual void tcDuel(std::vector<TCType*>& overlappingTCs)
    {
      if (overlappingTCs.size() != 2) {
        B2ERROR("TrackSetEvaluator::tcDuel: vector of overlapping TCs has wrong size: " << overlappingTCs.size() <<
                ", should be 2! skipping rest...")
        return;
      }

      B2DEBUG(25, "TrackSetEvaluator::tcDuel: first competitor got QI: " << overlappingTCs.front()->getTrackQuality() <<
              ", and second one got: " << overlappingTCs.back()->getTrackQuality())
      if (overlappingTCs.front()->getTrackQuality() > overlappingTCs.back()->getTrackQuality()) {
        overlappingTCs.back()->setAliveState(false);
      } else {
        overlappingTCs.front()->setAliveState(false);
      }
    }


    /** to be called to find out if there are TCs which are overlapping but not with each other - returns number of compatible TCs */
    virtual unsigned int findCompatibleOverlaps(std::vector<TCType*>& overlappingTCs) const
    {
      unsigned nOverlaps = overlappingTCs.size();
      unsigned nCompatibleTCs = 0;

      for (unsigned int iTC = 0; iTC < nOverlaps; iTC++) {
        for (unsigned int jTC = iTC + 1; jTC < nOverlaps; jTC++) {
//      (m_manager.areCompetitors(iTC, jTC) == false) and nCompatibleTCs++;
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
    virtual void letThebestTCSurvive(std::vector<TCType*>& overlappingTCs)
    {
      B2INFO("TrackSetEvaluatorBase::letThebestTCSurvive was triggered! Best QI is: " << overlappingTCs.front()->getTrackQuality())
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
      B2INFO("TrackSetEvaluatorBase::dealWithSimpleCases was triggered! nOverlaps: " << overlappingTCs.size() <<
             " checking now for 2 TCs overlapping:")
      /* case:
       * only 2 TCs are overlapping.
       * -> tcDuel
       */
      if (overlappingTCs.size() < 3) {
        if (overlappingTCs.size() == 2) {
          tcDuel(overlappingTCs);
          return true;
        } else if (overlappingTCs.size() == 0) {
          return true;
        }
        B2ERROR("TrackSetEvaluator::dealWithSimpleCases: nOverlapping TCs is " << overlappingTCs.size() <<
                " which is illegal. Stopping cleaning overlaps!")
        return false;
      }

      unsigned int nCompatibleTCs = findCompatibleOverlaps(overlappingTCs);
      B2INFO("TrackSetEvaluatorBase::dealWithSimpleCases: checking now for compatible overlaps... (found: " << nCompatibleTCs << ")")
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

  public:

    /** constructor */
    TrackSetEvaluatorBase(std::vector<TCType>& trackSet, CompetitorManagingType& manager) : m_trackSet(trackSet), m_manager(manager) {}


    /** destructor */
    virtual ~TrackSetEvaluatorBase() {}


    /** main function. returns number of final TCs */
    virtual unsigned int cleanOverlaps()
    {
      B2ERROR("TrackSetEvaluatorBase::cleanOverlaps() was called!" <<
              " This function does actually nothing, you sure you didn't want to pass an inheriting trackSetEvaluator to the network instead?" <<
              " These Classes can really clean overlaps in the network!")
      return 0;
    }


    /** simply return number of TCs in given network */
    virtual unsigned int getNTCs() const { return m_trackSet.size(); }
  };

} // end namespace Belle2
