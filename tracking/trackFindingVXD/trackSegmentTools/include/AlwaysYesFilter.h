/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
