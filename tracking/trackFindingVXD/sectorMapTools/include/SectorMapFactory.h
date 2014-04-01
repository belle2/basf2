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

#include <TObject.h>

// #include <map>
// #include <algorithm>
// #include <vector>

namespace Belle2 {


  /**SectorMapFactory - creates and handles a specific sectorMap.
   **/
  class SectorMapFactory : public TObject {
  public:

    /** constructor ??? */
    SectorMapFactory() {}

    void linkSpacePointsToActiveSectors() {}

    /** Imports the spacePoints into the sector map after converting them
     * into an internal hit type. */
    void fillSpacePoints() {}


  protected:

  };
} //Belle2 namespace