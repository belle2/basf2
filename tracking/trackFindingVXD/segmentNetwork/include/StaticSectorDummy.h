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

// C++-std:
#include <vector>

namespace Belle2 {


  /** interim mockup since we have no Static sectorMap at the moment */
  struct StaticSectorDummy {

/// DATA MEMBERS


    /** stores its own secID */
    FullSecID aSecID;

    /** stores innerSecIDs */
    std::vector<FullSecID> innerSecIDs;

/// CONSTRUCTORS AND OPERATORS


    /** standard constructor */
    StaticSectorDummy() : aSecID(FullSecID()) {}

    /** constructor */
    StaticSectorDummy(FullSecID secID) : aSecID(secID) {}

    /** == -operator - compares if two StaticSectors are identical */
    inline bool operator == (const StaticSectorDummy& b) const { return (getFullSecID() == b.getFullSecID()); }

    /** == -operator - compares if two StaticSectors are identical */
    inline bool operator == (const FullSecID& b) const { return (getFullSecID() == b); }

/// ACCESS FUNCTIONS


    /** returns innerSecIDs */
    const std::vector<Belle2::FullSecID>& getInnerSecIDs() const { return innerSecIDs; }

    /** returns FullSecID of this sector */
    FullSecID getFullSecID() const { return aSecID; }
  };

  /** non-memberfunction Comparison for equality with FullSecID <-> StaticSectorDummy */
  inline bool operator == (const FullSecID& a, const StaticSectorDummy& b)
  {
    return (a == b.getFullSecID());
  }
} //Belle2 namespace