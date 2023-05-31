/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/dataobjects/MCInitialParticles.h>

using namespace Belle2;

std::string MCInitialParticles::getGenerationFlagString(const std::string& separator) const
{
  std::string flags = "";
  const std::vector<std::pair<int, std::string>> flagvalues{
    {c_generateCMS, "generateCMS"},
    {c_smearBeamEnergy, "smearBeamEnergy"},
    {c_smearBeamDirection, "smearBeamDirection"},
    {c_smearVertex, "smearVertex"}
  };
  for (auto& i : flagvalues) {
    if (hasGenerationFlags(i.first)) {
      if (flags.size() > 0) flags += separator;
      flags += i.second;
    }
  }
  return flags;
}

ROOT::Math::LorentzRotation MCInitialParticles::cmsToLab(double bX, double bY, double bZ, double angleXZ, double angleYZ)
{
  //boost to CM frame
  ROOT::Math::LorentzRotation boost(ROOT::Math::Boost(-1.*ROOT::Math::XYZVector(bX, bY, bZ)));


  //rotation such that the collision axis is aligned with z-axis
  ROOT::Math::XYZVector zaxis(0., 0., 1.); //target collision axis

  double tanAngleXZ = tan(angleXZ);
  double tanAngleYZ = tan(angleYZ);
  double Norm   = 1 / sqrt(1 + pow(tanAngleXZ, 2) + pow(tanAngleYZ, 2));
  ROOT::Math::XYZVector electronCMS(Norm * tanAngleXZ, Norm * tanAngleYZ, Norm); //current collision axis

  ROOT::Math::XYZVector rotAxis = zaxis.Cross(electronCMS);
  double rotangle = asin(rotAxis.R());

  ROOT::Math::LorentzRotation rotation(ROOT::Math::AxisAngle(rotAxis, -rotangle));


  ROOT::Math::LorentzRotation trans  = rotation * boost;
  ROOT::Math::LorentzRotation transI = trans.Inverse();
  return transI;
}
