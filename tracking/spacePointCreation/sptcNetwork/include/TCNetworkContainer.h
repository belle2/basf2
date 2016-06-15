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
#include <tracking/spacePointCreation/sptcNetwork/TrackSetEvaluatorBase.h>

#include <framework/logging/Logger.h>

#include <vector>
#include <utility>
#include <functional>


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

    /** is working with the network and can remove all overlaps by killing the correct nodes */
    TrackSetEvaluatorBase<TCType, ObserverType>* m_trackSetEvaluator;

  public:
    /** ************************* CONSTRUCTORS ************************* */

    /** standard constructor, prepares observer for observing links */
    TCNetworkContainer() :
      m_obsi(m_links),
      m_trackSetEvaluator(new TrackSetEvaluatorBase<TCType, ObserverType>(m_nodes, m_obsi)) {}


    /** destructor cleaning up the evaluator */
    ~TCNetworkContainer() { delete m_trackSetEvaluator; }

    /** ************************* MEMBER FUNCTIONS ************************* */

/// getter

    /** return number of nodes in network.
    *
    * does not consider whether nodes are alive or not.
    */
    unsigned int size() const { return m_nodes.size(); }


    /** return link to the observer which observes the links */
    ObserverType& getObserver() { return m_obsi; }


    /** returns a reference to the nodes in the network */
    std::vector<TCType>& getNodes() { return m_nodes; }


    /** returns how many TCs are currently overlapping */
    unsigned int getNCompetitors() const
    {
      unsigned int nCompetitors = 0;

      for (const TCCompetitor& aCompetitor : m_links) {
        aCompetitor.hasCompetitors() and nCompetitors++;
      }
      B2DEBUG(50, "TCNetworkContainer::getNCompetitors: " << nCompetitors);
      return nCompetitors;
    }


    /** return how many TCs are currently alive */
    unsigned int getNTCsAlive() const
    {
      unsigned int nAlive = 0;

      for (const TCType& aTC : m_nodes) {
        aTC.isAlive() and nAlive++;
      }
      B2DEBUG(50, "TCNetworkContainer::getNTCsAlive: " << nAlive);
      return nAlive;
    }


    /** allows read-access to the evaluator (to use the simple getters for debugging) */
    const TrackSetEvaluatorBase<TCType, ObserverType>* accessEvaluator() const { return m_trackSetEvaluator; }


/// setter

    /** allows to replace the former trackSetEvaluator, please only pass stuff created with new! */
    void replaceTrackSetEvaluator(TrackSetEvaluatorBase<TCType, ObserverType>* newEvaluator)
    {
      delete m_trackSetEvaluator;
      m_trackSetEvaluator = newEvaluator;
    }


    /** add new TC as node and update all links in network */
    void add(TCType newTC)
    {
      // add new TC as competitor for each overlapping TC
      std::vector<unsigned int> newCompetitorsFound;
      for (TCType& aTC : m_nodes) {
        if (aTC.areCompetitors(newTC)) {
          newCompetitorsFound.push_back(aTC.getID());
          m_links[aTC.getID()].addCompetitor(newTC.getID());
          B2DEBUG(50, "current TC " << aTC.getID() << " got new competitor " << newTC.getID());
        } else {
          B2DEBUG(50, "current TC " << aTC.getID() << " is not competing with " << newTC.getID());
        }
      }
//       B2INFO("TCNEtworkContainer:add: newTC.getID: " << newTC.getID() << ", size(old) for m_nodes/m_links: " << m_nodes.size() << "/" << m_links.size())
      m_nodes.push_back(newTC);
      m_links.push_back(TCCompetitor(newTC.getID()));

      // add each overlapping TC to new TC as competitor
      TCCompetitor& newTCCompetitor = m_links.at(newTC.getID());
      for (unsigned int competitorID : newCompetitorsFound) {
        newTCCompetitor.addCompetitor(competitorID);
      }
//       auto vecPrint = [] (const std::vector<unsigned int>& vec) -> std::string { std::string out; for (auto iD : vec) { out += "competitor: " + std::to_string(iD) + "\n" ; } return out; };
//    B2INFO("TCNEtworkContainer:add: newTC.getID: " << newTC.getID()
//        << ", got competitor with ID: " << newTCCompetitor.getID()
//        << " and its own competitors: \n" << vecPrint(newTCCompetitor.getCompetitors()))

      // just some final sanity checks (DEBUG):
      for (unsigned int tcID = 0; tcID < size(); tcID++) {
        if (m_nodes[tcID].getID() != tcID) {
          B2ERROR("TCNEtworkContainer:add: TCAvatar got internal iD " << m_nodes[tcID].getID() << " but in-network-iD " << tcID <<
                  " this is strictly forbidden!");
        }
      }
      if (m_nodes.size() != m_links.size()) {
        B2ERROR("TCNEtworkContainer:add: m_nodes.size() " << size() << " and m_links.size() " << m_links.size() <<
                " differ - this is strictly forbidden!");
      }
    }


    /** print the network */
    void print() const
    {
      auto vecPrint = [](const std::vector<unsigned int>& vec) -> std::string { std::string out; for (auto iD : vec) { out += " " + std::to_string(iD) + "," ; } return out; };

      std::string out = "TCNetwork::print: The network got " + std::to_string(m_nodes.size()) + " nodes:\n";
      for (auto& aNode : m_nodes) {
        out += "node " + std::to_string(aNode.getID())
               + "(alive: " + std::to_string(aNode.isAlive())
               + ", QI: " + std::to_string(aNode.getTrackQuality())
               + ") got competitors: " + vecPrint(m_obsi.getCompetitors(aNode.getID())) + "\n";
      }
      B2INFO(out << "\n");
    }

    /** deactivates a TC and updates the competing links to it */
    void killTC(unsigned int iD)
    {
      if (iD < size()) {
        m_nodes[iD].setAliveState(false);
        return;
      }
      B2WARNING("TCNetworkContainer:killTC: given iD " << iD << " is not in network with size: " << size() << ", killing aborted...");
    }


    /** looping through the notify-function to the observer */
    void notifyRemove(unsigned int iD) { m_obsi.notifyRemove(iD); }


    /** start trackSetEvaluator to clean the overlaps, returns number of final tcs */
    bool cleanOverlaps() { return m_trackSetEvaluator->cleanOverlaps(); }
  };
} // end namespace Belle2
