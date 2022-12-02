/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <framework/geometry/BFieldManager.h>
#include <framework/geometry/B2Vector3.h>
#include <framework/gearbox/Unit.h>
#include <genfit/AbsBField.h>

/** Interface of the Belle II B-field with GenFit.
 */
class GFGeant4Field : public genfit::AbsBField {
public:
  /** Constructor.
   *
   *  The same B field as in the simulation is used.
   *  If you want to use a different field in reconstruction, please run two jobs and change
   *  the used B field in the geometry data (Belle2.xml file).
   */
  GFGeant4Field(): genfit::AbsBField() {}

  /** Getter for the magnetic field.
   *
   *  As Genfit uses kGauss, but we use T in basf2, we need to apply a factor 10 in the calculation.
   *  @param position   Position at which the magnetic field should be evaluated.
   */
  TVector3 get(const TVector3& position) const override
  {
    static double conversion{1. / Belle2::Unit::kGauss};
    return Belle2::B2Vector3D(Belle2::BFieldManager::getField(ROOT::Math::XYZVector(position)) * conversion);
  }
};

