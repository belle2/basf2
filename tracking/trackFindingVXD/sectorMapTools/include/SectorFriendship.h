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
#include <tracking/trackFindingVXD/sectorMapTools/Sector.h>

// includes - general fw stuff
#include <framework/datastore/RelationsObject.h>


namespace Belle2 {


  /** Forward declaration for the FilterBase */
  class FilterBase;

  /** SectorFriendship is carrying the link between parent sector and a connected sector (socalled Friendsector).
   *
   * - is created once per run and carries all the Filters including cutoff-values which are relevant for the friendship
   * - is linked (1:1) to a Sector-Sector-Friendship
   * - carries SectorFriendships which link sectors and store Filters
   * - and the SegmentMaker which takes a related ActivatedSector (socalled FriendSector) to create segments
   **/
  class SectorFriendship : public RelationsObject {
  public:
    /** is currently a counter which counts number of passed filterTests */
    typedef unsigned int CompatibilityValue;
    /** is currently a table of CompatibilityValues which allows to check which combination of spacepoints/segments are allowed to be combined */
    typedef std::vector<std::vector<CompatibilityValue> > CompatibilityTable;

    /** constructor */
    SectorFriendship() :
      m_mainSector(nullptr),
      m_friendSector(nullptr) {}

    /** returns friend sector */
    Sector* getFriend() { return m_friendSector; }

    /** creates a compatibility-table so that the filters can use it */
    void prepareCompatibilityTable();

    /** applies SegmentFilters on each spacepoint of this sector-sector-combination */
    void applySegmentFilters();

    /** iterates through each combination of spacepoints and counts the number of combinations which are still allowed */
    unsigned int checkCombinationsAlive() const;
  protected:

    /** The mainSector is stored once per run and is a link to a compatible outer sector. */
    Sector* m_mainSector;

    /** The friendSector is stored once per run and is a link to a compatible inner sector. */
    Sector* m_friendSector;

    /** This vector carries a pointer to all filters allowed for this sector-combination */
    std::vector<FilterBase*> m_myFilters;

    /** This table carries the compatibility for each combination of hits or segments. First dimension maps iD of hit on mainSector, second one maps iD on friendSector */
    std::vector<std::vector<CompatibilityValue> > m_compatibilityTable;

    ClassDef(SectorFriendship, 1)
  };
} //Belle2 namespace
