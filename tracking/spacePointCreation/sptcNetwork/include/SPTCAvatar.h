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


#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <framework/logging/Logger.h>

#include <vector>


namespace Belle2 {

  /** Carries relevant infos to a SpacePointTrackCand linked to this one.
   */
  template <class ObserverType>
  class SPTCAvatar {
  protected:
    /** ************************* DATA MEMBERS ************************* */

    /** pointer to original TC */
    SpacePointTrackCand& m_originalTC;

    /** pointer to observer */
    ObserverType& m_observer;

    /** needed for the neural network - stores the current neuron value */
    double m_neuronValue;

    /** identifying index number */
    unsigned int m_iD;

    /** activation state of TC */
    bool m_aliveState;

    /** if true, overlaps are checked via SpacePoints. If false, overlaps are checked via clusters */
    bool m_compareSPs;

  public:
    /** ************************* CONSTRUCTORS ************************* */

    /** constructor for SPTCAvatar.
     *
     * needs a SpacePointTrackCand, an observer, an ID given by the network storing that avatar.
     * As an optional feature, one can choose, whether one wants to compare spacePoints or clusters.
     */
    SPTCAvatar(SpacePointTrackCand& myOriginal, ObserverType& myObserver, unsigned int myID, bool compareSPs = false) :
      m_originalTC(myOriginal),
      m_observer(myObserver),
      m_neuronValue(0),
      m_iD(myID),
      m_compareSPs(compareSPs)
    {
      setAliveState(m_originalTC.hasRefereeStatus(SpacePointTrackCand::c_isActive));
    }


    /** ************************* OPERATORS ************************* */

    /** < -operator
     *
     * compares if this TC < other TC (only getQualityIndex is used for comparison)
     * */
    inline bool operator < (const SPTCAvatar& b) const { return (this->getTrackQuality() < b.getTrackQuality()); }

    /** > -operator
     *
     * compares if this TC > other TC (only getQualityIndex is used for comparison)
     * */
    inline bool operator > (const SPTCAvatar& b) const { return (this->getTrackQuality() > b.getTrackQuality()); }

    /** ************************* MEMBER FUNCTIONS ************************* */

/// getter

    /** if true, given tcAvatar is sharing clusters with this one */
    bool areCompetitors(const SPTCAvatar<ObserverType>& b) const
    {
      return m_originalTC.checkOverlap(b.getTC(), m_compareSPs);
    }


    /** if true, this TC is still active and can have competitors */
    bool isAlive() const { return m_aliveState; }


    /** returns pointer to linked original TC */
    const SpacePointTrackCand& getTC() const { return m_originalTC; }


    /** if true, this TC is reserved.
     * this means that it has somewhat higher priority compared to non-reserved TCs
     */
    bool isReserved() const
    {
      return m_originalTC.hasRefereeStatus(SpacePointTrackCand::c_isReserved);
    }


    /** returns estimated quality of the TC.
     *
     * defined in range [0;1], where 1 is best.
     */
    double getTrackQuality() const { return m_originalTC.getQualityIndex(); }


    /** returns current value of the neuron state */
    double getNeuronValue() const { return m_neuronValue; }


    /** returns iD of Avatar within network */
    unsigned int getID() const { return m_iD; }

/// setter

    /** sets state for TC.
     *
     * if newstate is false, all competitors are informed that this TC is now dead.
     * */
    void setAliveState(bool newState)
    {
//       B2DEBUG(50, "SPTCAvatar:setAliveState: newState = " << newState)
      if (newState == false) {
        m_observer.notifyRemove(m_iD);
      }
      m_aliveState = newState;
    }


    /** set this TC to reserved */
    void setReserved() { /* TODO */ }


    /** sets new value for the neuron state */
    void setNeuronValue(double newValue) { m_neuronValue = newValue; }

  };
} // end namespace Belle2
