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
#include <geometry/bfieldmap/BFieldMap.h>
#include <genfit/AbsBField.h>

/** Interface of the Belle II B-field with GenFit.
 */
class GFGeant4RecoField : public genfit::AbsBField {
public:
  /** Constructor.
   *
   *  The same B field as in the simulation is used.
   *  If you want to use a different field in reconstruction, please run two jobs and change
   *  the used B field in the geometry data (Belle2.xml file).
   */
  GFGeant4RecoField(): genfit::AbsBField(), m_bField(Belle2::BFieldMap::Instance()) {}

  /** Getter for the magnetic field.
   *
   *  As Genfit uses kGauss, but we use T in basf2, we need to apply a factor 10 in the calculation.
   *  @param position   Position at which the magnetic field should be evaluated.
   */
  TVector3 get(const TVector3& position) const
  {
    if (!GFGeant4RecoField::getSwitcher())
      return TVector3(0., 0., 0.);

    return (10.0 * m_bField.getBField(position));
  }

  /// Get reference to the magnet switcher (true=ON, false=OFF)
  static bool& getSwitcher()
  {
    static bool switcher = true;
    return switcher;
  }

protected:
  /** Reference to the central magnetic field map. */
  Belle2::BFieldMap& m_bField;
};

