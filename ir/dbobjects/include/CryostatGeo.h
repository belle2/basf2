/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <ir/dbobjects/IRGeoBase.h>
#include <framework/gearbox/GearDir.h>

namespace Belle2 {

  /**
   * Geometry parameters of Cryostat
   */
  class CryostatGeo:  public IRGeoBase {

  public:

    /**
     * Default constructor
     */
    CryostatGeo()
    {}

    /**
     * Initialize from gearbox (xml file)
     * @param content gearbox folder
     */
    void initialize(const GearDir& content);

  private:

    ClassDef(CryostatGeo, 1); /**< ClassDef */

  };

} // end namespace Belle2
