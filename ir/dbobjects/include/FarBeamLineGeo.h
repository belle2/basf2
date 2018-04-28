/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <ir/dbobjects/IRGeoBase.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <map>

namespace Belle2 {

  /**
   * Default constructor
   */
  class FarBeamLineGeo:  public IRGeoBase {

  public:

    /**
     * Default constructor
     */

    FarBeamLineGeo()
    {}

    void initialize(const GearDir& content);

  private:

    ClassDef(FarBeamLineGeo, 1); /**< ClassDef */

  };

} // end namespace Belle2
