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
#include <string>

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

    /** unique identifier */
    const std::int64_t m_identifier;

  public:
    /** ************************* CONSTRUCTORS ************************* */

    /** Default constructor for the ROOT IO. */
    Segment():
      m_outerHit(NULL),
      m_innerHit(NULL),
      m_outerSector(FullSecID()),
      m_innerSector(FullSecID()),
      m_identifier(-1)
    {}


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
      m_identifier(static_cast<std::int64_t>(outerNode->getID()) << 32 | static_cast<std::int64_t>
                   (innerNode->getID())) // Use int TrackNode IDs to construct int Segment ID
    {}

    /** ************************* OPERATORS ************************* */

    /** overloaded '=='-operator */
    bool operator==(const Segment& b) const
    {
      return (*getInnerHit() == *b.getInnerHit() and * getOuterHit() == *b.getOuterHit());
    }

    /** ************************* PUBLIC MEMBER FUNCTIONS ************************* */
/// getters:

    /** return ID of this segment */
    std::int64_t getID() const { return m_identifier; }

    /** returns longer debugging name of this segment */
    std::string getName() const
    {
      if (m_identifier >= 0)
        return "Out: " + m_outerHit->getName() + ")" + ",\nin: " + m_innerHit->getName();
      else
        return "Out: missing,\nin: missing";
    }

    /** returns inner hit of current Segment */
    inline const HitType* getInnerHit() const { return m_innerHit; }


    /** returns outer hit of current Segment */
    inline const HitType* getOuterHit() const { return m_outerHit; }


    /** returns inner secID of current Segment */
    inline FullSecID::BaseType getInnerSecID() const { return m_innerSector; }


    /** returns outer secID of current Segment */
    inline FullSecID::BaseType getOuterSecID() const { return m_outerSector; }
  };
} //Belle2 namespace
