/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BFIELDCOMPONENTCONSTANT_H
#define BFIELDCOMPONENTCONSTANT_H

#include <geometry/bfieldmap/BFieldComponentAbs.h>

namespace Belle2 {

  /**
   * The BFieldComponentConstant class.
   *
   * This class represents an homogeneous magnetic field.
   */
  class BFieldComponentConstant : public BFieldComponentAbs {

  public:

    /** The BFieldComponentAbs constructor. */
    BFieldComponentConstant();

    /** The BFieldComponentAbs destructor. */
    virtual ~BFieldComponentConstant();

    /**
     * Calculates the magnetic field vector at the specified space point.
     *
     * All magnetic field component classes have to overwrite this method.
     * Please note: The magnetic field component class has to perform the
     * check if the space point lies within the space region it describes
     * If the space point given doesn't lie inside the space region this
     * method should return TVector(0,0,0).
     *
     * @param point The space point in Cartesian coordinates (x,y,z) in [cm] at which the magnetic field vector should be calculated.
     * @return The magnetic field vector at the given space point in [T]. Return a zero vector TVector(0,0,0) if the space point lies outside the region described by the component.
     */
    virtual TVector3 calculate(const TVector3& point) const;

    /**
     * Sets the values for the homogeneous magnetic field vector.
     * @param x The x component of the magnetic field vector
     * @param y The y component of the magnetic field vector
     * @param z The z component of the magnetic field vector
     */
    void setMagneticFieldValues(double x, double y, double z);


  protected:

  private:

    double m_magneticField[3]; /**< The values for the homogeneous magnetic field vector. */

  };

} //end of namespace Belle2

#endif /* BFIELDCOMPONENTCONSTANT_H */
