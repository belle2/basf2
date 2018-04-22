/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Heck                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <framework/geometry/BFieldManager.h>
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
  TVector3 get(const TVector3& position) const
  {
    static double conversion{1. / Belle2::Unit::kGauss};
    return Belle2::BFieldManager::getField(position) * conversion;
  }
};

