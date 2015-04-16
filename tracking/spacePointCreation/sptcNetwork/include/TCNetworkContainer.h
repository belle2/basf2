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

  /** A container for storing avatars which form a network.
   *
   * Its focus is to be a neat and encapsulating interface for quick adding, removing and checking competitors.
   */
  template <class TCType, class ObserverType>
  class TCNetworkContainer {
  protected:

    /** ************************* DATA MEMBERS ************************* */

    /** stores Nodes of the network - the TCs */
    std::vector<TCType > m_nodes;

    /** stores the links of the network - the competitors */
    std::vector<TCCompetitor> m_links;

    /** keeps the links updated, if something changes with the nodes */
    ObserverType m_obsi;

  public:


    /** ************************* CONSTRUCTORS ************************* */

    /** standard constructor, prepares observer for observing links */
    TCNetworkContainer() : m_obsi(m_links) {}

    /** ************************* OPERATORS ************************* */

    /** ************************* MEMBER FUNCTIONS ************************* */


/// getter


    /** return number of nodes in network */
    unsigned int size() const { return m_nodes.size(); }

    /** return link to the observer which observes the links */
    ObserverType& getObserver() { return m_obsi; }

    /** returns how many TCs are currently overlapping */
    unsigned int getNCompetitors() const
    {
      unsigned int nCompetitors = 0;

      for (const TCCompetitor& aCompetitor : m_links) {
        aCompetitor.hasCompetitors() and nCompetitors++;
      }
      B2DEBUG(50, "TCNetworkContainer::getNCompetitors: " << nCompetitors)
      return nCompetitors;
    }

    /** return how many TCs are currently alive */
    unsigned int getNTCsAlive() const
    {
      unsigned int nAlive = 0;

      for (const TCType& aTC : m_nodes) {
        aTC.isAlive() and nAlive++;
      }
      B2DEBUG(50, "TCNetworkContainer::getNTCsAlive: " << nAlive)
      return nAlive;
    }


/// setter


    /** add new TC as node and update all links in network */
    void add(TCType newTC)
    {
      // add new TC as competitor for each overlapping TC
      std::vector<unsigned int> newCompetitorsFound;
      for (TCType& aTC : m_nodes) {
        if (aTC.areCompetitors(newTC)) {
          newCompetitorsFound.push_back(aTC.getID());
          m_links[aTC.getID()].addCompetitor(newTC.getID());
          B2DEBUG(50, "current TC " << aTC.getID() << " got new competitor " << newTC.getID())
        } else {
          B2DEBUG(50, "current TC " << aTC.getID() << " is not competing with " << newTC.getID())
        }
      }
      m_nodes.push_back(newTC);
      m_links.push_back(TCCompetitor());

      // add each overlapping TC to new TC as competitor
      TCCompetitor& newTCCompetitor = m_links.at(newTC.getID());
      for (unsigned int competitorID : newCompetitorsFound) {
        newTCCompetitor.addCompetitor(competitorID);
      }


      // just some final sanity checks (DEBUG):
      for (unsigned int tcID = 0; tcID < size(); tcID++) {
        if (m_nodes[tcID].getID() != tcID) {
          B2ERROR("TCNEtworkContainer:add: TCAvatar got internal iD " << m_nodes[tcID].getID() << " but in-network-iD " << tcID <<
                  " this is strictly forbidden!")
        }
      }
      if (m_nodes.size() != m_links.size()) {
        B2ERROR("TCNEtworkContainer:add: m_nodes.size() " << m_nodes.size() << " and m_links.size() " << m_links.size() <<
                " differ - this is strictly forbidden!")
      }
    }

    /** deactivates a TC and updates the competing links to it */
    void killTC(unsigned int iD)
    {
      if (iD >= size()) {
        B2WARNING("TCNetworkContainer:killTC: given iD " << iD << " is not in network with size: " << size() << ", killing aborted...")
        return;
      }
      m_nodes[iD].setAliveState(false);
    }

    /** looping through the notify-function to the observer */
    void notifyRemove(unsigned int iD) { m_obsi.notifyRemove(iD); }


  };
} // end namespace Belle2
