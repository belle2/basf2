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
     * @param u u-coordinate of sensor center.
     * @param v v-ccordinate of sensor center.
     * @param du (mean) width in u.
     * @param dv width in v.
     * @param dudv du/dv for trapezoidal sensors.
     */
    SiSensorPlane(double u, double v, double du, double dv, double dudv = 0):
        m_u(u), m_v(v), m_du(du), m_dv(dv), m_dudv(dudv)
    {;}

    /**
     * Default constructor.
     * Constructs a default plane and inActive returns false for any point.
     */
    SiSensorPlane(): m_u(0), m_v(0), m_du(0), m_dv(0), m_dudv(0)
    {;}

    /**
     * inActive is true if point (u,v) in the plane is inside the finite region.
     * @param u u-coordinate of the point.
     * @param v v-coordinate of the point.
     * @return true if (u,v) is within the sensor plane, otherwise false.
     */
    bool inActive(const double& u, const double& v) const {
      return ((fabs(u - m_u) < m_du + m_dudv *(v - m_v)) && (fabs(v - m_v) < m_dv));
    }

    /**
     * Prints object data.
     */
    void Print() const;

    /**
     * Deep copy of the object.
     * @return Pointer to a deep copy of the object.
     */
    virtual GFAbsFinitePlane* clone() const {
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
    double m_du;         /**< (mean) half-width along u coordinate. */
    double m_dv;         /**< half-width along v coordinate. */
    double m_dudv;       /**< du/dv for trapezoidal sensor. */

    ClassDef(SiSensorPlane, 1)
  };

#endif /* SIDETECTORPLANE_H_ */

} // end namespace
