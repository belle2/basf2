/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz, Martin Heck                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <TLorentzVector.h>
#include <TLorentzRotation.h>
#include <framework/utilities/Boosts.h>

using namespace Belle2;

inline TLorentzRotation Boosts::getLab2CMSBoost(const float& energyHER, const float& energyLER, const float& crossAngle, const float& angleLER)
{
  return getLab2CMSBoost(energyHER, energyLER, crossAngle, angleLER).Inverse();
}

TLorentzRotation Boosts::getCMS2LabBoost(const float& energyHER, const float& energyLER, const float& crossAngle, const float& angleLER)
{
  // Lab frame Z axis is defined by B field;
  // its positive direction close to direction of
  // high energy beam, i.e. e- beam.
  double angleHerToB = crossAngle  - angleLER;
  double angleLerToB = TMath::Pi() - angleLER;

  // Getting the various LorentzVectors.
  TLorentzVector lorentzVecLER;
  lorentzVecLER.SetXYZM(energyLER * sin(angleLerToB), 0., energyLER * cos(angleLerToB), Unit::electronMass);

  TLorentzVector lorentzVecHER;
  lorentzVecHER.SetXYZM(energyHER * sin(angleHerToB), 0., energyHER * cos(angleHerToB), Unit::electronMass);

  TLorentzVector vectorLorentzY4S = lorentzVecHER + lorentzVecLER;

  // Transformation from Lab system to CMS system
  TLorentzRotation lab2cmsBoost(vectorLorentzY4S.BoostVector());

  // boost HER e- from Lab system to CMS system
  const TLorentzVector lorentzVecElectronCMS = lab2cmsBoost * lorentzVecHER;

  // now rotate CMS such that incomming e- is parallel to z-axis
  TVector3 zaxis(0., 0., 1.);
  TVector3 rotaxis = zaxis.Cross(lorentzVecElectronCMS.Vect()) * (1. / lorentzVecElectronCMS.Vect().Mag());
  double rotangle = TMath::ASin(rotaxis.Mag());

  // return rotated Vector.
  return (lab2cmsBoost.Rotate(-rotangle, rotaxis));
}
