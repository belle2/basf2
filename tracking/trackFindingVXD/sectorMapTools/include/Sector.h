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
#include <framework/datastore/RelationsObject.h>

// includes - stl:
#include <vector>

// includes - tf-related stuff
// includes - general fw stuff


namespace Belle2 {


  /** forward declaration for the ActivatedSector */
  class ActivatedSector;
  /** forward declaration for the SectorFriendship */
  class SectorFriendship;

  /** Sector is a central part of storing information for VXD trackFinders.
   *
   * - is created by the ExportSectorMapModule, where relations between Sectors are set
   * - is linked (1:1) to an ActivatedSector
   * - carries SectorFriendships which link sectors and store Filters
   * - and the SegmentMaker which takes a related ActivatedSector (socalled FriendSector) to create segments
   **/
  class Sector : public RelationsObject {
  public:

    /** constructor */
    Sector() {}

    /** called each event - takes all spacePoints from the activated Sector and its friend Sector to produce segments */
    void segmentMaker() {}

    virtual void clear() { m_myActiveSector = NULL; }

  protected:

    /** The activated sector is created each event where this sector inhabits a spacePoint. */
    ActivatedSector* m_myActiveSector;

    /** This vector carries a pointer to each SectorFriendship for faster access during events */
    std::vector<SectorFriendship*> m_myFriends;

    ClassDef(Sector, 1)
  };
} //Belle2 namespace