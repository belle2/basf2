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


  /**SectorMap - carries metaData to attached sectors and related objects.
   *
   * - knows what is part of the current collection of Sectors
   **/
  class SectorMap : public TObject {
  public:

    /** constructor */
    SectorMap() {}

  protected:

    ClassDef(SectorMap, 1)
  };
} //Belle2 namespace