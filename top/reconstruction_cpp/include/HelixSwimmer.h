/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TVector3.h>
#include <TRotation.h>
#include <cmath>
#include <algorithm>

namespace Belle2 {
  namespace TOP {

    /**
     * Utility for propagation of a particle along helix.
     * Helix representation here differs from the official one.
     * Running parameter is the signed distance along helix measured from the reference position.
     * Negative distance means propagating back in time.
     */
    class HelixSwimmer {

    public:

      /**
       * default constructor
       */
      HelixSwimmer()
      {}

      /**
       * Sets the helix
       * @param position particle position (= default reference position)
       * @param momentum particle momentum
       * @param charge particle charge in units of elementary charge
       * @param Bz magnetic field z-component (we assume other two are negligible)
       */
      void set(const TVector3& position, const TVector3& momentum, double charge, double Bz);

      /**
       * Sets transformation from the frame in which helix is constructed to module local frame.
       * Transformation is: inverse translation first then inverse rotation
       * @param rotation rotation matrix
       * @param translation translation vector
       */
      void setTransformation(const TRotation& rotation, const TVector3& translation);

      /**
       * Moves reference position along helix by length
       * @param length distance along helix measured from current reference position
       */
      void moveReferencePosition(double length);

      /**
       * Returns particle position at given length
       * @param length distance along helix measured from the reference position
       * @return position (in module local frame if transformation is set)
       */
      TVector3 getPosition(double length) const;

      /**
       * Returns particle direction at given length
       * @param length distance along helix measured from the reference position
       * @return direction (in module local frame if transformation is set)
       */
      TVector3 getDirection(double length) const;

      /**
       * Returns the distance along helix to the nearest intersection with a plane nearly parallel to z axis
       * @param point point on the plane
       * @param normal plane normal
       * @return signed distance (NaN if cross-section doesn't exist)
       */
      double getDistanceToPlane(const TVector3& point, const TVector3& normal) const;

    private:

      /**
       * Returns shortest distance
       * @param cosAlpha cosine of alpha
       * @param phi phi of plane
       * @return signed shortest distance
       */
      double shortestDistance(double cosAlpha, double phi) const;

      /**
       * Returns helix length within a single turn
       * @param t helix length
       * @return helix length within single turn
       */
      double withinSingleTurn(double t) const;

      double m_R = 0; /**< helix radius */
      double m_xc = 0; /**< helix axis position in x */
      double m_yc = 0; /**< helix axis position in y */
      double m_omega = 0; /**< angular speed [1/cm] */
      double m_kz = 0; /**< slope in z */
      double m_phi0 = 0; /**< phi of reference position */
      double m_z0 = 0; /**< z of reference position */
      double m_T0 = 0; /**< helix length of single turn */

      double& x0 = m_xc; /**< x of reference position for zero magnetic field */
      double& y0 = m_yc; /**< y of reference position for zero magnetic field */
      double& z0 = m_z0; /**< z of reference position for zero magnetic field */
      double& kx = m_R;  /**< direction in x for zero magnetic field */
      double& ky = m_phi0; /**< direction in y for zero magnetic field */
      double& kz = m_kz; /**< direction in z for zero magnetic field */

      TRotation m_rotationInv; /**< inverse of rotation matrix */
      TVector3 m_translation;  /**< translation vector */

    };


    inline double HelixSwimmer::shortestDistance(double cosAlpha, double phi) const
    {
      double alpha = acos(cosAlpha);
      double t1 = withinSingleTurn((phi - m_phi0 - alpha) / m_omega);
      double t2 = withinSingleTurn((phi - m_phi0 + alpha) / m_omega);
      double ta = std::min(t1, t2);
      double tb = std::max(t1 - m_T0, t2 - m_T0);
      double t = (abs(ta) < abs(tb)) ? ta : tb;
      return t;
    }

    inline double HelixSwimmer::withinSingleTurn(double t) const
    {
      t = fmod(t, m_T0);
      if (t < 0) t += m_T0;
      return t;
    }

  } // namespace TOP
} // namespace Belle2

