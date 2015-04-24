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

#include <tracking/spacePointCreation/sptcNetwork/TCCompetitor.h>

#include <framework/logging/Logger.h>

#include <vector>


namespace Belle2 {


  /** a class to take care of competitors.
   *
   * Its job is to observe a TCAvatar and if they say they want to be removed from their competitors, this class is doing that for them.
   */
  class TCCompetitorGuard {
  protected:
    /** ************************* DATA MEMBERS ************************* */

    /** stores the links of the network - the competitors */
    std::vector<TCCompetitor>& m_links;

  public:
    /** constructor */
    TCCompetitorGuard(std::vector<TCCompetitor>& observedVector) : m_links(observedVector)
    {
//       B2DEBUG(150, "TCCompetitorGuard:constructor: nEntries in observedVector " << observedVector.size());
    }


    /** notify function called by class which wants to be observed. For each competitor of given iD, the iD will be removed as competitor for them. */
    void notifyRemove(unsigned int iD)
    {
      if (iD >= m_links.size()) {
        B2WARNING("TCCompetitorGuard:notifyRemove: TC not officially registered in network yet! Skipping notifyRemove...")
        return;
      }
      B2DEBUG(50, "TCCompetitorGuard:notifyRemove: id " << iD << " got " << m_links[iD].getNCompetitors() <<
              " competitors which will now be informed")

      for (unsigned int aCompetitor : m_links[iD].getCompetitors()) {
        unsigned int nCompetitorsB4 = m_links[aCompetitor].getNCompetitors();
        m_links[aCompetitor].removeCompetitor(iD);
        B2DEBUG(75, " competitor " << aCompetitor << " has removed " << iD << ". nCompetitorsB4: " << nCompetitorsB4 << ", now: " <<
                m_links.at(
                  aCompetitor).getNCompetitors())
        m_links[iD].removeCompetitor(aCompetitor);
      }
    }


    /** says whether given TCCompetitor has competitors stored */
    bool hasCompetitors(unsigned int iD)
    {
      return iD < m_links.size() and m_links[iD].hasCompetitors();
    }

    /** getCompetitors - returns non-modifyable reference to entries of the container */
    const std::vector<unsigned int>& getCompetitors(unsigned int iD) const { return  m_links[iD].getCompetitors(); }
  };

} // end namespace Belle2
