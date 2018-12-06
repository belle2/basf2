/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once


namespace Belle2 {

  /** The CACell class
   * This Class stores all relevant information one wants to have stored in a cell for a Cellular automaton.
   */
  class CACell {
  public:
    /** ************************* CONSTRUCTORS ************************* */
    /** Default constructor for the ROOT IO. */
    CACell():
      m_state(0),
      m_activated(true),
      m_stateUpgrade(false),
      m_seed(false) {}


    /** ************************* OPERATORS ************************* */
    /** overloaded '=='-operator */
    bool operator==(const CACell& b) const
    { return (m_state == b.m_state and m_activated == b.m_activated); }

    /** overloaded '!='-operator */
    bool operator!=(const CACell& b) const
    { return (m_state != b.m_state or m_activated != b.m_activated); }


    /** ************************* PUBLIC MEMBER FUNCTIONS ************************* */
    /// getters:
    /** returns state of CACell (CA-feature) */
    inline unsigned int getState() const { return m_state; }

    /** returns whether CACell is allowed to be a seed for TCs */
    inline bool isSeed() const { return m_seed; }

    /** returns activationState (CA-feature) */
    inline bool isActivated() const { return m_activated; }

    /** returns info whether stateIncrease is allowed or not (CA-feature) */
    inline bool isUpgradeAllowed() const { return m_stateUpgrade; }


    /// setters:
    /** increases state during CA update step */
    inline void increaseState() { m_state++; }

    /** sets flag whether CACell is allowed to increase state during update step within CA */
    inline void setStateUpgrade(bool up) { m_stateUpgrade = up; }

    /** sets flag whether CACell is allowed to be the seed of a new track candidate or not */
    inline void setSeed(bool isSeedTrue) { m_seed = isSeedTrue; }

    /** sets flag whether CACell is active (takes part during current CA iteration) or inactive (does not take part, it is 'dead') */
    inline void setActivationState(bool activationState) { m_activated = activationState; }


  protected:
    /** ************************* DATA MEMBERS ************************* */
    /** state of CACell during CA process, begins with 0 */
    unsigned int m_state;

    /** activation state. Living CACells (active) are allowed to evolve in the CA, dead ones (inactive) are not allowed */
    bool m_activated;

    /** sets flag whether Segment is allowed to increase state during update step within CA */
    bool m_stateUpgrade;

    /** sets flag whether Segment is allowed to be the seed of a new track candidate or not */
    bool m_seed;
  };
}
