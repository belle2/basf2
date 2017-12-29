/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef GEOMETRY_BFIELDMAP_BFIELDFRAMEWORKINTERFACE_H
#define GEOMETRY_BFIELDMAP_BFIELDFRAMEWORKINTERFACE_H

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
#endif // GEOMETRY_BFIELDMAP_FRAMEWORKINTERFACE_H
