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

#include <tracking/trackFindingCDC/numerics/ERotation.h>
#include <tracking/trackFindingCDC/numerics/ESign.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class Vector3D;
    class Vector2D;

    /// Helper functions to interact with the magnetic field
    class CDCBFieldUtil {
    public:
      /// Indicator if the magnetic field is off in the current geometry.
      static bool isOff();

      /// Getter for the sign of the magnetic field in z direction
      static ESign getBFieldZSign();

      /// Getter for the signed magnetic field stength in z direction at the origin ( in Tesla )
      static double getBFieldZ();

      /// Getter for the signed magnetic field stength in z direction ( in Tesla )
      static double getBFieldZ(const Vector2D& pos2D);

      /// Getter for the signed magnetic field stength in z direction ( in Tesla )
      static double getBFieldZ(const Vector3D& pos3D);

      /// Translater from magnetic field strength in Tesla to the alpha value.
      static double getAlphaFromBField(double bField);

      /// Getter for the signed alpha of the magnetic field in z direction
      static double getAlphaZ(const Vector2D& pos2D);

      /// Getter for the signed alpha of the magnetic field in z direction
      static double getAlphaZ(const Vector3D& pos3D);

      /**
       *  Conversion helper from clockwise or counterclockwise travel to the charge sign.
       *
       *  Return the charge sign based on the travel direction on the fitted circle.
       *  With the Lorentz force F = q * v x B.
       *  For positively charged particles we have:
       *
       *  *  Counterclockwise travel <-> Bz < 0
       *  *  Clockwise travel        <-> Bz > 0
       *  and opposite for negatively charged.
       *  Hence the charge sign is -ERotation * sign(Bz)
       */
      static ESign ccwInfoToChargeSign(ERotation ccwInfo);

      /// Conversion helper from the charge sign to clockwise or counterclockwise travel
      static ERotation chargeSignToERotation(ESign chargeSign);

      /// Conversion help for charges to clockwise or counterclockwise travel.
      static ERotation chargeToERotation(double charge);

      /// Conversion helper for momenta to two dimensional (absolute) bend radius
      static double absMom2DToBendRadius(double absMom2D,
                                         double bZ);

      /// Conversion helper for momenta to two dimensional (absolute) bend radius
      static double absMom2DToBendRadius(double absMom2D,
                                         const Vector2D& pos2D);

      /// Conversion helper for momenta to two dimensional (absolute) bend radius
      static double absMom2DToBendRadius(double absMom2D,
                                         const Vector3D& pos3D);

      /// Conversion helper for momenta to two dimensional curvature
      static double absMom2DToCurvature(double absMom2D,
                                        double charge,
                                        double bZ);

      /// Conversion helper for momenta to two dimensional curvature
      static double absMom2DToCurvature(double absMom2D,
                                        double charge,
                                        const Vector2D& pos2D);

      /// Conversion helper for momenta to two dimensional curvature
      static double absMom2DToCurvature(double absMom2D,
                                        double charge,
                                        const Vector3D& pos3D);

      /// Conversion helper for two dimensional curvature to momenta
      static double curvatureToAbsMom2D(double curvature,
                                        double bZ);

      /// Conversion helper for two dimensional curvature to momenta
      static double curvatureToAbsMom2D(double curvature,
                                        const Vector2D& pos2D);

      /// Conversion helper for two dimensional curvature to momenta
      static double curvatureToAbsMom2D(double curvature,
                                        const Vector3D& pos3D);
    };
  }
}
