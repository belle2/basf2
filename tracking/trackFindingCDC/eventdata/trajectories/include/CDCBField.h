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

#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/numerics/ERotation.h>
#include <cmath>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Helper functions to interact with the magnetic field
    class CDCBFieldUtil {
    public:

      /// Indicator if the magnetic field is off in the current geometry.
      static bool isOff();
    };

    /// Getter for the absolute magnetic field strength in z direction ( in Tesla )
    double getBFieldZMagnitude(const Vector2D& pos2D);

    /// Getter for the sign of the magnetic field in z direction
    ESign getBFieldZSign();

    /// Getter for the signed magnetic field stength in z direction ( in Tesla )
    double getBFieldZ(const Vector2D& pos2D);

    /// Getter for the signed magnetic field stength in z direction ( in Tesla )
    double getBFieldZ(const Vector3D& pos3D = Vector3D(0.0, 0.0, 0.0));

    /// Translater from magnetic field strength in Tesla to the alpha value.
    double getAlphaFromBField(const double bField);

    /// Getter for the signed alpha of the magnetic field in z direction
    double getAlphaZ(const Vector2D& pos2D);

    /// Getter for the signed alpha of the magnetic field in z direction
    double getAlphaZ(const Vector3D& pos3D);

    /// Conversion helper from clockwise or counterclockwise travel to the charge sign.
    /** Return the charge sign based on the travel direction on the fitted circle. \n
     *  With the Lorentz force F = q * v x B \n
     *  For positively charged particles we have \n
     *  Counterclockwise travel <-> Bz < 0 \n
     *  Clockwise travel        <-> Bz > 0 \n
     *  and opposite for negatively charged. \n
     *  Hence the charge sign is -ERotation * sign(Bz) */
    ESign ccwInfoToChargeSign(const ERotation ccwInfo);

    /// Conversion helper from the charge sign to clockwise or counterclockwise travel
    ERotation chargeSignToERotation(const ESign& chargeSign);

    /// Conversion help for charges to clockwise or counterclockwise travel.
    ERotation chargeToERotation(const double charge);

    /// Conversion helper for momenta to two dimensional (absolute) bend radius
    double absMom2DToBendRadius(const double absMom2D,
                                const double bZ);

    /// Conversion helper for momenta to two dimensional (absolute) bend radius
    double absMom2DToBendRadius(const double absMom2D,
                                const Vector2D& pos2D);

    /// Conversion helper for momenta to two dimensional (absolute) bend radius
    double absMom2DToBendRadius(const double absMom2D,
                                const Vector3D& pos3D);

    /// Conversion helper for momenta to two dimensional curvature
    double absMom2DToCurvature(const double absMom2D,
                               const double charge,
                               const double bZ);

    /// Conversion helper for momenta to two dimensional curvature
    double absMom2DToCurvature(const double absMom2D,
                               const double charge,
                               const Vector2D& pos2D);

    /// Conversion helper for momenta to two dimensional curvature
    double absMom2DToCurvature(const double absMom2D,
                               const double charge,
                               const Vector3D& pos3D);

    /// Conversion helper for two dimensional curvature to momenta
    double curvatureToAbsMom2D(const double curvature,
                               const double bZ);

    /// Conversion helper for two dimensional curvature to momenta
    double curvatureToAbsMom2D(const double curvature,
                               const Vector2D& pos2D);

    /// Conversion helper for two dimensional curvature to momenta
    double curvatureToAbsMom2D(const double curvature,
                               const Vector3D& pos3D);

  } // namespace TrackFindingCDC
} // namespace Belle2
