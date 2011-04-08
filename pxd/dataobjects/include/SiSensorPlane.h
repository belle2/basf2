/*************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef SISENSORPLANE_H_
#define SISENSORPLANE_H_

#include <GFAbsFinitePlane.h>
#include <cmath>

namespace Belle2 {

  class SiSensorPlane: public GFAbsFinitePlane {
  public:
    /**
     * Useful constructor.
     * Constructs the plane from central point and sizes. Trapezoidal detectors
     * are acceptable.
     */
    SiSensorPlane(double u, double v, double du, double dv, double dv2 = -1);

    /**
     * Default constructor.
     * Constructs a default plane and inActive returns false for any point.
     */
    SiSensorPlane();

    /**
     * inActive is true if point (u,v) in the plane is inside the finite region.
     */
    bool inActive(const double& u, const double& v) const {
      return ((fabs(u - m_u) < m_du) && (fabs(v - m_v) < m_dv0 + m_dvSlope *(u - m_u)));
    }

    /**
     * Prints object data.
     */
    void Print() const;

    /**
     * Deep copy of the object.
     */
    virtual SiSensorPlane* clone() const {
      return new SiSensorPlane(*this);
    }

    /**
     * Destructor.
     */
    virtual ~SiSensorPlane() {
      ;
    }

  private:
    double m_u;          /**< u coordinate of center. */
    double m_v;          /**< v coordinate of center. */
    double m_du;         /**< size along u coordinate. */
    double m_dv;         /**< size along v at minimal u. */
    double m_dv2;        /**< size along v at maximal u. */
    double m_dv0;        /**< mean width in v. */
    double m_dvSlope;    /**< edge slope along u. */

    ClassDef(SiSensorPlane, 1)
  };

#endif /* SIDETECTORPLANE_H_ */

} // end namespace
