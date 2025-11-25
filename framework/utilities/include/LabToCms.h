/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <Math/Boost.h>
#include <Math/AxisAngle.h>
#include <Math/LorentzRotation.h>
#include <Math/Vector3D.h>

namespace Belle2 {

  /** Class implementing general Lorentz Transformation between
      LAB and CMS system.
      The transformation is defined by 5 parameters, the remaining
      6th parameter (the rotation around phi) is fixed such that
      phiLAB and phiCMS are as similar as achievable.
      These 5 parameters are:
         - Velocity of the CM system (3D vector)
         - Direction of the pHER in the CM system obtained by pure boost (2 angles)
   **/
  class LabToCms {

  public:


    /**
     * Function takes 3D boostVector and angles of the HER momentum
     * in the CM system obtained by pure boost.
     * The angles are defined as cmsAngleXZ = atan(pxCM/pzCM)
     *                           cmsAngleYZ = atan(pyCM/pzCM)
     * Returns Lorentz transformation from Lab to CMS
     * @return the Lorentz rotation matrix
     */
    static ROOT::Math::LorentzRotation  rotateLabToCms(const ROOT::Math::XYZVector& boostVector,
                                                       double cmsAngleXZ, double  cmsAngleYZ)
    {
      // Boost to CM frame
      ROOT::Math::LorentzRotation boost(ROOT::Math::Boost(-1.*boostVector));

      // Target pHER orientation (unit 3D vector)
      ROOT::Math::XYZVector zaxis(0., 0., 1.);

      // The current pHER orientation (unit 3D vector)
      double tanAngleXZ = tan(cmsAngleXZ);
      double tanAngleYZ = tan(cmsAngleYZ);
      double Norm   = 1 / sqrt(1 + pow(tanAngleXZ, 2) + pow(tanAngleYZ, 2));
      ROOT::Math::XYZVector electronCMS(Norm * tanAngleXZ, Norm * tanAngleYZ, Norm);

      // Axis to rotate current pHER direction to the target one
      ROOT::Math::XYZVector rotAxis = zaxis.Cross(electronCMS);
      double rotangle = asin(rotAxis.R()); // rotation angle

      // Rotation that makes pHER align with z-axis
      ROOT::Math::LorentzRotation rotation(ROOT::Math::AxisAngle(rotAxis, -rotangle));

      // First boost, then rotation
      ROOT::Math::LorentzRotation trans = rotation * boost;
      return trans;
    }

  };

}
