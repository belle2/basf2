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


    /** notify function called by class which wants to be observed.
    *
    * For each competitor of given iD, the iD will be removed as competitor for them.
    * Additionally given ID gets rid of competitors as well.
    */
    void notifyRemove(unsigned int iD)
    {
      if (iD >= m_links.size()) {
        B2WARNING("TCCompetitorGuard:notifyRemove: TC not officially registered in network yet! Skipping notifyRemove...");
        return;
      }
      if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 50, PACKAGENAME()) == true) {
        B2DEBUG(50, "TCCompetitorGuard:notifyRemove: id " << iD << " got " << m_links[iD].getNCompetitors() <<
                " competitors which will now be informed");
//    auto vecPrint = [] (const std::vector<unsigned int>& vec) -> std::string { std::string out; for (auto iD : vec) { out += "competitors: " + std::to_string(iD) + "\n" ; } return out; };
        B2DEBUG(50, print(m_links[iD].getCompetitors()) << "\n");
      }

      // clean now the competitors of given iD
      for (unsigned int aCompetitor : m_links[iD].getCompetitors()) {
        unsigned int nCompetitorsB4 = m_links[aCompetitor].getNCompetitors();
//    B2INFO("TCCompetitorGuard:notifyRemove: inform now competitor: " << aCompetitor << " to be killed!")
        m_links[aCompetitor].removeCompetitor(iD);
        B2DEBUG(50, "TCCompetitorGuard:notifyRemove: current competitor " << aCompetitor << " has removed " << iD << ". nCompetitorsB4: " <<
                nCompetitorsB4 << ", now: " <<
                m_links.at(aCompetitor).getNCompetitors());
      }
      // clean now the node of the given iD, has to be done in separate step to prevent undefined behavior
      m_links[iD].clearAllCompetitors();

//       for (unsigned int aCompetitor : m_links[iD].getCompetitors()) {
//    B2INFO("TCCompetitorGuard:notifyRemove: in node " << iD << " killing now competitor: " << aCompetitor)
//    m_links[iD].removeCompetitor(aCompetitor);
//    }
    }


    /** removes all competing competitors of given iD. */
    void clearAllCompetitors(unsigned int iD)
    {
      if (iD >= m_links.size()) {
        B2WARNING("TCCompetitorGuard:clearAllCompetitors: given ID is not part of the network! Skipping clearAllCompetitors...");
        return;
      }
      m_links[iD].clearAllCompetitors();
    }


    /** says whether given TCCompetitor has competitors stored */
    bool hasCompetitors(unsigned int iD) const
    {
      return iD < m_links.size() and m_links[iD].hasCompetitors();
    }


    /** getCompetitors - returns non-modifyable reference to entries of the container */
    const std::vector<unsigned int>& getCompetitors(unsigned int iD) const { return  m_links[iD].getCompetitors(); }


    /** for given pair of iDs it will be checked if they are competitors or not (this function relies on the fact that the competitor-info is stored in both competitors) */
    bool areCompetitors(unsigned int a, unsigned int b) const
    {
      if (m_links[a].getNCompetitors() > m_links[b].getNCompetitors()) {
        return m_links[b].isCompetitor(a);
      }
      return m_links[a].isCompetitor(b);
    }


    /** returns current number of competing entries still alive at time of function call */
    unsigned int countCompetitors() const
    {
      unsigned int nCompetitors = 0;
      // by design nCompetitors for dead links is 0, therefore no alive-state has to be checked:
      for (const auto& link : m_links) { if (link.hasCompetitors()) nCompetitors++; }
      return nCompetitors;
    }


    /** print the competitors */
    std::string print(const std::vector<unsigned int>& vec) const
    {
      std::string out;
      for (auto iD : vec) {
        out += "competitors: " + std::to_string(iD) + "\n" ;
      }
      return out;
    }
  };

} // end namespace Belle2
