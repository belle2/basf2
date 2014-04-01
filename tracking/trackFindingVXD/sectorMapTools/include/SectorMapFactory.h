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
#include <TObject.h>

// includes - stl:
// includes - tf-related stuff:
// includes - general fw stuff:


namespace Belle2 {


  /**SectorMapFactory - creates and handles a specific sectorMap.
   * - imports it from a xml file carrying a root stream
   * - reads relations and creates internal links for faster access between related objects
   **/
  class SectorMapFactory : public TObject {
  public:

    /** constructor ??? */
    SectorMapFactory() {}

    /** called each event - imports all spacePoints and fills them into activated sectors. */
    void linkSpacePointsToActiveSectors() {}


  protected:

    ClassDef(SectorMapFactory, 1)
  };
} //Belle2 namespace