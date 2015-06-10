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

#include <tracking/dataobjects/FullSecID.h>
#include <framework/logging/Logger.h>

// C++-std:
#include <vector>



namespace Belle2 {

  /** The Segment class
   * This class represents segments of track candidates needed for TrackFinderVXD-Modules
   */
  template<class HitType>
  class Segment {
  protected:
    /** ************************* DATA MEMBERS ************************* */

    /** pointer to hit forming the outer end of the Segment. */
    HitType* m_outerHit;

    /** pointer to hit forming the inner end of the Segment. */
    HitType* m_innerHit;

    /** iD of sector carrying outer hit */
    FullSecID::BaseType m_outerSector;

    /** iD of sector carrying inner hit */
    FullSecID::BaseType m_innerSector;

    /** state of Segment during CA process, begins with 0 */
    unsigned int m_state;

    /** activation state. Living Cells (active) are allowed to evolve in the CA, dead ones (inactive) are not allowed */
    bool m_activated;

    /** sets flag whether Segment is allowed to increase state during update step within CA */
    bool m_stateUpgrade;

    /** sets flag whether Segment is allowed to be the seed of a new track candidate or not */
    bool m_seed;


  public:
    /** ************************* CONSTRUCTORS ************************* */

    /** Default constructor for the ROOT IO. */
    Segment():
      m_outerHit(NULL),
      m_innerHit(NULL),
      m_outerSector(FullSecID()),
      m_innerSector(FullSecID()),
      m_state(0),
      m_activated(true),
      m_stateUpgrade(false),
      m_seed(true) {}


    /** Constructor.
    *      //      * @param outerSector secID of outer Sector associated with this segment.
    *      //      * @param innerSector secID of inner Sector associated with this segment.
    *      //      * @param outerNode pointer to outer Hit associated with this segment.
    *      //      * @param innerNode pointer to inner Hit associated with this segment.
      *      //      */
    Segment(FullSecID::BaseType outerSector, FullSecID::BaseType innerSector, HitType* outerNode, HitType* innerNode):
      m_outerHit(outerNode),
      m_innerHit(innerNode),
      m_outerSector(outerSector),
      m_innerSector(innerSector),
      m_state(0),
      m_activated(true),
      m_stateUpgrade(false),
      m_seed(true) {}


    /** ************************* OPERATORS ************************* */

    /** overloaded '=='-operator */
    bool operator==(const Segment& b) const
    {
      return (*getInnerHit() == *b.getInnerHit() and * getOuterHit() == *b.getOuterHit());
    }


    /** ************************* PUBLIC MEMBER FUNCTIONS ************************* */
/// getters:

    /** returns state of Segment (CA-feature) */
    inline int getState() const { return m_state; }


    /** returns whether Segment is allowed to be a seed for TCs */
    inline bool isSeed() const { return m_seed; }


    /** returns activationState (CA-feature) */
    inline bool isActivated() const { return m_activated; }


    /** returns info whether stateIncrease is allowed or not (CA-feature) */
    inline bool isUpgradeAllowed() const { return m_stateUpgrade; }


    /** returns inner hit of current Segment */
    inline const HitType* getInnerHit() const { return m_innerHit; }


    /** returns outer hit of current Segment */
    inline const HitType* getOuterHit() const { return m_outerHit; }


    /** returns inner secID of current Segment */
    inline FullSecID::BaseType getInnerSecID() const { return m_innerSector; }


    /** returns outer secID of current Segment */
    inline FullSecID::BaseType* getOuterSecID() const { return m_outerSector; }


/// setters:

    /** increases state during CA update step */
    inline void increaseState() { m_state++; }


    /** sets flag whether Cell is allowed to increase state during update step within CA */
    inline void setStateUpgrade(bool up) { m_stateUpgrade = up; }


    /** sets flag whether Cell is allowed to be the seed of a new track candidate or not */
    inline void setSeed(bool isSeedTrue) { m_seed = isSeedTrue; }


    /** sets flag whether Cell is active (takes part during current CA iteration) or inactive (does not take part, it is 'dead') */
    inline void setActivationState(bool activationState) { m_activated = activationState; }
  };
} //Belle2 namespace
