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

// includes - rootStuff:
// includes - stl:
// includes - tf-related stuff
#include "Sector.h"
#include "tracking/spacePointCreation/SpacePoint.h"

// includes - general fw stuff


namespace Belle2 {


  /** ActivatedSector is carrying the dynamic part of a Sector.
   *
   * - is created once per event only if there is at least one spacePoint attached to it
   * - is linked (1:1) to a Sector
   * - carries SectorFriendships which link sectors and store Filters
   * - and the SegmentMaker which takes a related ActivatedSector (socalled FriendSector) to create segments
   **/
  class ActivatedSector : public Sector {
  public:

    /** constructor */
    explicit ActivatedSector(Sector* mySector) { m_myStaticSector = mySector; }

    /** stores a spacePoint */
    void addSpacePoint(const SpacePoint* aPoint) { m_mySpacePoints.push_back(aPoint); }

    /** returns number of spacePoint inhabiting this ActivatedSector */
    unsigned int size() { return m_mySpacePoints.size(); }

    /** returns a pointer to this ActivatedSector */
    ActivatedSector* getMyPointer() { return this; }

  protected:

    /** The sector is created once per run and carries all the stuff which does not change for each event. */
    Sector* m_myStaticSector;

    /** This vector carries a pointer to each spacePoint which inhabit this sector */
    std::vector<const SpacePoint*> m_mySpacePoints;

    ClassDef(ActivatedSector, 1)
  };
} //Belle2 namespace
