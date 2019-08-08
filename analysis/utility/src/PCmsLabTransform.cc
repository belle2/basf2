/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/utility/PCmsLabTransform.h>
#include <framework/gearbox/GearDir.h>
#include <memory>

using namespace Belle2;

TLorentzVector PCmsLabTransform::labToCms(const TLorentzVector& vector)
{
  PCmsLabTransform T;
  return T.rotateLabToCms() * vector;
}

TLorentzVector PCmsLabTransform::cmsToLab(const TLorentzVector& vector)
{
  PCmsLabTransform T;
  return T.rotateCmsToLab() * vector;
}

PCmsLabTransform::PCmsLabTransform() = default;

const BeamParameters& PCmsLabTransform::getBeamParams() const
{
  static std::shared_ptr<BeamParameters> beamParamsFromGearbox;

  if (beamParamsFromGearbox) {
    return *beamParamsFromGearbox.get();
  }

  //check if any BeamParameters are available in the database
  if (m_beamParams) {
    return *m_beamParams;
  }

  //No BeamParameters available, load from gearbox
  GearDir LER("/Detector/SuperKEKB/LER/");
  GearDir HER("/Detector/SuperKEKB/HER/");
  double Eher = HER.getEnergy("energy");
  double Eler = LER.getEnergy("energy");

  beamParamsFromGearbox = std::make_shared<BeamParameters>();
  beamParamsFromGearbox->setHER(Eher, HER.getAngle("angle"), std::vector<double>());
  beamParamsFromGearbox->setLER(Eler, LER.getAngle("angle"), std::vector<double>());

  return *beamParamsFromGearbox.get();
}
