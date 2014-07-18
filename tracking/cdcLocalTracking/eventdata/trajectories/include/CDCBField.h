/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCBFIELD_H
#define CDCBFIELD_H

#include <cmath>

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/InfoTypes.h>

#include <tracking/cdcLocalTracking/geometry/Vector2D.h>
#include <tracking/cdcLocalTracking/geometry/Vector3D.h>

namespace Belle2 {

  namespace CDCLocalTracking {

    //const FloatType CDCTrajectory2D::c_bFieldZMagnitude = 1.5;
    //const SignType CDCTrajectory2D::c_bFieldZSign = PLUS;
    //const FloatType CDCTrajectory2D::c_bFieldZ = c_bFieldZSign * c_bFieldZMagnitude;

    // this file should not really be the provider of this information.
    // So these functions should eventually change to the correct
    // information provider.

    /// Constant for the magnetic field strength in z direction ( in Tesla )
    const FloatType c_bFieldZMagnitude = 1.5;

    /// Constant for the sign of the magnetic field in z direction. To be checked.
    const SignType c_bFieldZSign = PLUS; // to be checked

    /// Constant for the signed magnetic field strength in z direction ( in Tesla )
    const FloatType c_bFieldZ = c_bFieldZSign* c_bFieldZMagnitude;

    /// Getter for the absolute magnetic field strength in z direction ( in Tesla )
    const FloatType& getBFieldZMagnitude(const Vector2D& pos2D = Vector2D(0.0, 0.0));

    /// Getter for the sign of the magnetic field in z direction
    const SignType& getBFieldZSign();

    /// Getter for the signed of the magnetic field stength in z direction ( in Tesla )
    const FloatType& getBFieldZ(const Vector2D& pos2D = Vector2D(0.0, 0.0));

    /// Getter for the signed of the magnetic field stength in z direction ( in Tesla )
    const FloatType& getBFieldZ(const Vector3D& pos3D = Vector3D(0.0, 0.0, 0.0));

    /// Conversion helper from clockwise or counterclockwise travel to the charge sign.
    /** Return the charge sign based on the travel direction on the fitted circle. \n
     *  With the Lorentz force F = q * v x B \n
     *  For positively charged particles we have \n
     *  Counterclockwise travel <-> Bz < 0 \n
     *  Clockwise travel        <-> Bz > 0 \n
     *  and opposite for negatively charged. \n
     *  Hence the charge sign is -CCWInfo * sign(Bz) */
    SignType ccwInfoToChargeSign(const CCWInfo& ccwInfo);

    /// Conversion helper from the charge sign to clockwise or counterclockwise travel
    CCWInfo chargeSignToCCWInfo(const SignType& chargeSign);

    /// Conversion help for charges to clockwise or counterclockwise travel.
    CCWInfo chargeToCCWInfo(const FloatType& charge);

    /// Conversion helper for momenta to radii
    FloatType absMom2DToRadius(const FloatType& absMom2D,
                               const FloatType& charge,
                               const Vector2D& pos2D = Vector2D(0.0, 0.0));

    FloatType absMom2DToCurvature(const FloatType& absMom2D,
                                  const FloatType& charge,
                                  const Vector2D& pos2D = Vector2D(0.0, 0.0));

    FloatType absMom2DToCurvature(const FloatType& absMom2D,
                                  const FloatType& charge,
                                  const Vector3D& pos3D = Vector3D(0.0, 0.0, 0.0));

    FloatType curvatureToAbsMom2D(const FloatType& curvature,
                                  const Vector2D pos2D = Vector2D(0.0, 0.0));

    FloatType curvatureToAbsMom2D(const FloatType& curvature,
                                  const FloatType& charge,
                                  const Vector2D& pos2D = Vector2D(0.0, 0.0));

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCBFIELD_H
