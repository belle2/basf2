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
#include <string>

namespace Belle2 {

  /**
   * Geometry parameters of  public TObject {
  public
   */
  class CryostatGeo:  public IRGeoBase {

  public:

    /**
     * Default constructor
     */

    CryostatGeo()
    {}

    void initialize(const GearDir& content);

  private:

    ClassDef(CryostatGeo, 1); /**< ClassDef */

  };

} // end namespace Belle2
