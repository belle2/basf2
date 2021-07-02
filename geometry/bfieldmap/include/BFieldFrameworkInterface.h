/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/dbobjects/MagneticFieldComponent.h>
#include <framework/gearbox/Unit.h>
#include <geometry/bfieldmap/BFieldMap.h>

namespace Belle2 {
  /** Simple BFieldComponent to just wrap the existing BFieldMap with the new BFieldManager */
  class BFieldFrameworkInterface: public MagneticFieldComponent {
  public:
    /** this component is exclusive: ignore all others */
    BFieldFrameworkInterface(): MagneticFieldComponent(true) {}
    /** everything is inside this component */
    virtual bool inside(const B2Vector3D&) const final override { return true; }
    /** and we return the values from the exsiting BFieldMap */
    virtual B2Vector3D getField(const B2Vector3D& position) const final override
    {
      return BFieldMap::Instance().getBField(position) * Unit::T;
    }
  };
}
