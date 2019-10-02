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

// includes - tf-related stuff
#include "FilterBase.h"

// includes - general fw stuff


namespace Belle2 {

  /** AlwaysYesFilter is a simple filter saying always yes, which is meant for testing purposes.
   **/
  class AlwaysYesFilter : public FilterBase {
  public:

    /** constructor */
//     AlwaysYesFilter(0,0) { FilterBase(0,0); }

    /** called for each compatible sector-sector-combination inhabiting spacePoints */
    virtual void checkSpacePoints(const SectorFriendship* thisFriendship, CompatibilityTable& compatibilityTable);

  protected:

    ClassDef(AlwaysYesFilter, 1)
  };
} //Belle2 namespace
