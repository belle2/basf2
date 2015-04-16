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
#include <algorithm>    // std::find
#include <functional> // std::remove


namespace Belle2 {

  /** A container for storing infos to existing competitors.
   *
   * Its focus is to be a neat and encapsulating interface for quick adding, removing and checking competitors.
   */
  class TCCompetitor {
  protected:

    /** ************************* DATA MEMBERS ************************* */

    /** stores indices for given competitors */
    std::vector<unsigned int> m_competitors;


    /** returns iterator-position of given iD. is m_competitors.end() if entry not found. */
    std::vector<unsigned int>::const_iterator find(unsigned int iD) const { return std::find(m_competitors.begin(), m_competitors.end(), iD); }

    /** short cut which tells you, if given iterator is end of m_competitors */
    bool isEnd(std::vector<unsigned int>::const_iterator pos) const { return pos == m_competitors.end(); }
  public:

    /** ************************* CONSTRUCTORS ************************* */

    /** ************************* OPERATORS ************************* */

    /** ************************* MEMBER FUNCTIONS ************************* */


/// getter

    /** says whether TCCompetitorContainer has competitors stored */
    bool hasCompetitors() const { return !m_competitors.empty(); }

    /** returns non-modifyable reference to entries of the container */
    const std::vector<unsigned int>& getCompetitors() const { return m_competitors; }

    /** returns current number of competitors */
    unsigned int getNCompetitors() const { return m_competitors.size(); }

    /** for given id it will be checked whether it is one of the competitors or not */
    bool isCompetitor(unsigned int iD) const { return !isEnd(find(iD)); }


/// setter

    /** adds given id if it was no competitor yet. If id was already competitor, a warning will be given */
    void addCompetitor(unsigned int iD)
    {
      if (isCompetitor(iD)) {
        B2WARNING("TCCompetitorContainer::addCompetitor: given iD " << iD << " was already found among " << getNCompetitors() <<
                  " competitors! ID will not be added again")
      } else {
        m_competitors.push_back(iD);
      }
    }

    /** removes given id if it was a competitor. If iD was not found among the competitors, a warning will be given */
    void removeCompetitor(unsigned int iD)
    {
      if (isCompetitor(iD)) {
        std::remove(m_competitors.begin(), m_competitors.end(), iD);
        m_competitors.resize(getNCompetitors() - 1);
      } else {
        B2WARNING("TCCompetitorContainer::addCompetitor: given iD " << iD << " was not found among " << getNCompetitors() <<
                  " competitors! ID can not be removed!")
      }
    }


  };
} // end namespace Belle2
