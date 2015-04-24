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

    /** to be called if only two TCs are overlapping */
    virtual void tcDuel(std::vector<TCType*>& overlappingTCs)
    {
      if (overlappingTCs.size() != 2) {
        B2ERROR("TrackSetEvaluator::tcDuel: vector of overlapping TCs has wrong size: " << overlappingTCs.size() <<
                ", should be 2! skipping rest...")
        return;
      }

      B2INFO("TrackSetEvaluator::tcDuel: first competitor got QI: " << overlappingTCs.front()->getTrackQuality() <<
             ", and second one got: " << overlappingTCs.back()->getTrackQuality())
      if (overlappingTCs.front()->getTrackQuality() > overlappingTCs.back()->getTrackQuality()) {
        overlappingTCs.back()->setAliveState(false);
      } else {
        overlappingTCs.front()->setAliveState(false);
      }
    }


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
      B2INFO("TrackSetEvaluatorBase::getOverlappingTCs: in a trackset of size " << m_trackSet.size() << ", " << overlappingTCs.size() <<
             " overlapping TCs were found ")

      return overlappingTCs;
    }


  public:
    /** constructor */
    TrackSetEvaluatorBase(std::vector<TCType>& trackSet, CompetitorManagingType& manager) : m_trackSet(trackSet), m_manager(manager) {}


    /** destructor */
    virtual ~TrackSetEvaluatorBase() {}


    /** main function. returns number of final TCs */
    virtual unsigned int cleanOverlaps(/*std::vector<TCType*>& overlappingTCs*/)
    {
      B2ERROR("TrackSetEvaluatorBase::cleanOverlaps() was called with a list having " << /*overlappingTCs.size() <<*/ " entries!" <<
              " This function does actually nothing, you sure you didn't want to pass an inheriting trackSetEvaluator to the network instead?" <<
              " These Classes can really clean overlaps in the network!")
      return 0;
    }


    /** simply return number of TCs in given network */
    virtual unsigned int getNTCs() const { return m_trackSet.size(); }
  };

} // end namespace Belle2
