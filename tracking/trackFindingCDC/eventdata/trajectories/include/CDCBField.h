/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#ifndef CDCBFIELD_H
#define CDCBFIELD_H

#include <cmath>

#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>
#include <tracking/trackFindingCDC/typedefs/InfoTypes.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Getter for the absolute magnetic field strength in z direction ( in Tesla )
    FloatType getBFieldZMagnitude(const Vector2D& pos2D);

    /// Getter for the sign of the magnetic field in z direction
    SignType getBFieldZSign();

    /// Getter for the signed magnetic field stength in z direction ( in Tesla )
    FloatType getBFieldZ(const Vector2D& pos2D);

    /// Getter for the signed magnetic field stength in z direction ( in Tesla )
    FloatType getBFieldZ(const Vector3D& pos3D = Vector3D(0.0, 0.0, 0.0));

    /// Translater from magnetic field strength in Tesla to the alpha value.
    FloatType getAlphaFromBField(const double& bField);

    /// Getter for the signed alpha of the magnetic field in z direction
    FloatType getAlphaZ(const Vector2D& pos2D);

    /// Getter for the signed alpha of the magnetic field in z direction
    FloatType getAlphaZ(const Vector3D& pos3D);



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

    /// Conversion helper for momenta to two dimensional curvature
    FloatType absMom2DToCurvature(const FloatType& absMom2D,
                                  const FloatType& charge,
                                  const FloatType& bZ);

    /// Conversion helper for momenta to two dimensional curvature
    FloatType absMom2DToCurvature(const FloatType& absMom2D,
                                  const FloatType& charge,
                                  const Vector2D& pos2D);

    /// Conversion helper for momenta to two dimensional curvature
    FloatType absMom2DToCurvature(const FloatType& absMom2D,
                                  const FloatType& charge,
                                  const Vector3D& pos3D);

    /// Conversion helper for two dimensional curvature to momenta
    FloatType curvatureToAbsMom2D(const FloatType& curvature,
                                  const FloatType& bZ);

    /// Conversion helper for two dimensional curvature to momenta
    FloatType curvatureToAbsMom2D(const FloatType& curvature,
                                  const Vector2D& pos2D);

    /// Conversion helper for two dimensional curvature to momenta
    FloatType curvatureToAbsMom2D(const FloatType& curvature,
                                  const Vector3D& pos3D);

  } // namespace TrackFindingCDC
} // namespace Belle2
#endif // CDCBFIELD_H
