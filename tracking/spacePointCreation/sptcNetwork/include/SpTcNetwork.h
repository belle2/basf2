/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
//
// // tracking
#include <tracking/spacePointCreation/sptcNetwork/TCCompetitorGuard.h>
#include <tracking/spacePointCreation/sptcNetwork/SPTCAvatar.h>
#include <tracking/spacePointCreation/sptcNetwork/TCNetworkContainer.h>
//
// // framework
#include <framework/datastore/RelationsObject.h>
// #include <framework/datastore/StoreObjPtr.h>
// #include <framework/core/FrameworkExceptions.h>
// #include <framework/logging/Logger.h>
//
// // stl:
// #include <vector>
// #include <string>
// #include <utility> // std::pair
// #include <math.h>

namespace Belle2 {
  /** The SpTcNetwork class.
   *
   *  Is intended to be used as StoreObjPtr
   */
  class SpTcNetwork : public RelationsObject {

  protected:
    /** ************************* DATA MEMBERS ************************* */

    /** the actual network, packed here to be able to be stored as StoreArray-Compatible object */
    TCNetworkContainer<SPTCAvatar<TCCompetitorGuard>, TCCompetitorGuard > m_network;

    /** if true, overlaps are checked via SpacePoints. If false, overlaps are checked via clusters */
    bool m_compareSPs;

    ClassDef(SpTcNetwork, 1) // last member changed:  m_network
  public:

    /** ************************* CONSTRUCTORS ************************* */

    /** standard constructor for Root IO */
    SpTcNetwork() :
      m_network(TCNetworkContainer<SPTCAvatar<TCCompetitorGuard>, TCCompetitorGuard >()),
      m_compareSPs(false) {}

    /** virtual destructor to prevent undefined behavior for inherited classes */
    virtual ~SpTcNetwork() {}

    /** ************************* MEMBER FUNCTIONS ************************* */


/// getter


    /** return number of nodes in network */
    unsigned int size() const { return m_network.size(); }

    /** return link to the observer which observes the links */
    TCCompetitorGuard& getObserver() { return m_network.getObserver(); }

    /** returns how many TCs are currently overlapping */
    unsigned int getNCompetitors() const { return m_network.getNCompetitors(); }

    /** return how many TCs are currently alive */
    unsigned int getNTCsAlive() const  { return m_network.getNTCsAlive(); }


/// setter


    /** add new TC as node and update all links in network */
    void add(SpacePointTrackCand& newTC)
    {
      m_network.add(SPTCAvatar<TCCompetitorGuard >(newTC, m_network.getObserver(), m_network.size(), m_compareSPs));
    }

    /** deactivates a TC and updates the competing links to it */
    void killTC(unsigned int iD) { m_network.killTC(iD); }
  };
}
