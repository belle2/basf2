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
#include <string>     // std::string, std::to_string


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


    /** stores own index number for identification */
    unsigned int m_identifier;

    /** ************************* INTERNAL MEMBER FUNCTIONS ************************* */

    /** returns iterator-position of given iD. is m_competitors.end() if entry not found. */
    std::vector<unsigned int>::const_iterator find(unsigned int iD) const { return std::find(m_competitors.begin(), m_competitors.end(), iD); }


    /** short cut which tells you, if given iterator is end of m_competitors */
    bool isEnd(std::vector<unsigned int>::const_iterator pos) const { return pos == m_competitors.end(); }
  public:

    /** constructor expecting to become its own iD during construction */
    TCCompetitor(unsigned int iD) : m_identifier(iD) {}

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


    /** returns iD of this competitor */
    unsigned int getID() const { return m_identifier; }


/// setter

    /** adds given id if it was no competitor yet. If id was already competitor, a warning will be given */
    void addCompetitor(unsigned int iD)
    {
      if (isCompetitor(iD)) {
        B2WARNING("TCCompetitor(iD: " << m_identifier << ")::addCompetitor: given iD " << iD << " was already found among " <<
                  getNCompetitors() <<
                  " competitors! ID will not be added again");
      } else if (iD == m_identifier) {
        B2ERROR("TCCompetitor(iD: " << m_identifier << ")::addCompetitor: given iD " << iD <<
                " is identical with itself. This iD is not allowed to be added to the  " <<
                getNCompetitors() <<
                " competitors! Attempt to allow competing with itself aborted!");
      } else {
        m_competitors.push_back(iD);
      }
    }


    /** removes given id if it was a competitor. If iD was not found among the competitors, a warning will be given */
    void removeCompetitor(unsigned int iD)
    {
      using namespace std;
      if (isCompetitor(iD)) {
        auto newEndOfVector = std::remove(m_competitors.begin(), m_competitors.end(), iD);
        unsigned int newSize = std::distance(m_competitors.begin(), newEndOfVector);
        B2DEBUG(50, "TCCompetitor::removeCompetitor: own iD " << getID() << " removes competing id: " << iD << " with nOldCompetitors: " <<
                getNCompetitors() << ", nNewCompetitors: " << newSize);
        m_competitors.resize(std::distance(m_competitors.begin(), newEndOfVector));
      } else {
        // storing lambda as function, compiler does not accept direct usage in B2WARNING (do not know why)
        auto miniPrinter = [](vector<unsigned>& vec) -> string { string out; for (auto iD : vec) { out += " " + to_string(iD); } return out; };
        B2WARNING("TCCompetitor(iD: " << m_identifier << ")::removeCompetitor: given iD " << iD
                  << " was not found among " << getNCompetitors()
                  << " competitors (with iDs:" << miniPrinter(m_competitors)
                  << ")! ID can not be removed!");
      }
    }


    /** removes all competing competitors from this one. */
    void clearAllCompetitors()
    {
      unsigned int oldSize = m_competitors.size();
      auto miniPrinter = [](std::vector<unsigned>& vec) -> std::string { std::string out; for (auto iD : vec) { out += " " + std::to_string(iD); } return out; };
      std::string out = miniPrinter(m_competitors);
      m_competitors.clear();
      B2DEBUG(50, "TCCompetitor::clearAllCompetitors: competitor " << getID() << " had " << oldSize << " competitors and " <<
              m_competitors.size() << " competitors now.\nOld ones were: " << out);
    }
  };
} // end namespace Belle2
