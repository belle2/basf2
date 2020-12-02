/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <geometry/bfieldmap/BFieldComponentAbs.h>

namespace Belle2 {

  /**
   * The BFieldComponentConstant class.
   *
   * This class represents an homogeneous magnetic field.
   */
  class BFieldComponentConstant : public BFieldComponentAbs {

  public:

    /** The BFieldComponentConstant constructor. */
    BFieldComponentConstant() = default;

    /** The BFieldComponentConstant destructor. */
    virtual ~BFieldComponentConstant() = default;

    /**
     * Calculates the magnetic field vector at the specified space point.
     *
     * @param point The space point in Cartesian coordinates (x,y,z) in [cm] at which the magnetic field vector should be calculated.
     * @return The magnetic field vector at the given space point in [T]. Returns a zero vector TVector(0,0,0) if the space point lies outside the region described by the component.
     */
    virtual B2Vector3D calculate(const B2Vector3D& point) const override;

    /**
     * Sets the values for the homogeneous magnetic field vector.
     * @param x The x component of the magnetic field vector.
     * @param y The y component of the magnetic field vector.
     * @param z The z component of the magnetic field vector.
     * @param rmax Maximum radial coordinate.
     * @param zmin Minimum z coordinate.
     * @param zmax Maximum z coordinate.
     */
    void setMagneticFieldValues(double x, double y, double z, double rmax, double zmin, double zmax);

  private:
    /** The values for the homogeneous magnetic field vector. */
    double m_magneticField[3] {0};
    /** The values for the maximum radius for the homogeneous field. */
    double maxRadius4BField{0};
    /** The values for the minimum z position for the homogeneous field. */
    double minZ4BField{0};
    /** The values for the maximum z position for the homogeneous field. */
    double maxZ4BField{0};

  };

} //end of namespace Belle2
