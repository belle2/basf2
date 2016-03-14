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
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>
#include <iostream>

using namespace Belle2;

bool PCmsLabTransform::m_initialized(false);
TLorentzRotation PCmsLabTransform::m_cms2lab;
TLorentzRotation PCmsLabTransform::m_lab2cms;
TLorentzVector PCmsLabTransform::m_boost;
double PCmsLabTransform::m_cmsEnergy(0);

TLorentzVector PCmsLabTransform::labToCms(const TLorentzVector& vector)
{
  static PCmsLabTransform T;
  return T.rotateLabToCms() * vector;
}

TLorentzVector PCmsLabTransform::cmsToLab(const TLorentzVector& vector)
{
  static PCmsLabTransform T;
  return T.rotateCmsToLab() * vector;
}

PCmsLabTransform::PCmsLabTransform()
{
  if (m_initialized) return;

  GearDir LER("/Detector/SuperKEKB/LER/");
  GearDir HER("/Detector/SuperKEKB/HER/");
  double Eher = HER.getEnergy("energy");
  double Eler = LER.getEnergy("energy");
  double cross_angle = HER.getAngle("angle") - LER.getAngle("angle");
  double angle = -LER.getAngle("angle");
  update(Eher, Eler, cross_angle, angle);
  m_initialized = true;
}


void PCmsLabTransform::update(double Eher, double Eler, double cross_angle, double angle_ler)
{
  // this function is a polished copy of generators/utilities/src/cm2LabBoost.cc

  // Low energy beam
  double angleLerToB = M_PI - angle_ler;
  double pLerZ = Eler * cos(angleLerToB);
  double pLerX = Eler * sin(angleLerToB);
  double pLerY = 0.;
  TLorentzVector vlLer;
  vlLer.SetXYZM(pLerX, pLerY, pLerZ, Const::electronMass);

  // High energy beam
  double angleHerToB = cross_angle - angle_ler;
  double pHerZ = Eher * cos(angleHerToB);
  double pHerX = Eher * sin(angleHerToB);
  double pHerY = 0.;
  TLorentzVector vlHer;
  vlHer.SetXYZM(pHerX, pHerY, pHerZ, Const::electronMass);

  // Boost vector
  m_boost = vlHer + vlLer;

  // Transformation from Lab system to CMS system
  TVector3 cmsBoostVec = m_boost.BoostVector();
  TLorentzRotation lab2cmsBoost(-cmsBoostVec);

  // boost e- to CMS system
  TLorentzVector vlEleCms = lab2cmsBoost * vlHer;

  // now rotate CMS such that incoming e- is parallel to z-axis
  TVector3 zaxis(0., 0., 1.);
  TVector3 rotaxis = zaxis.Cross(vlEleCms.Vect()) * (1. / vlEleCms.Vect().Mag());
  double rotangle = TMath::ASin(rotaxis.Mag());
  m_lab2cms = lab2cmsBoost.Rotate(-rotangle, rotaxis);

  // obtain the backward transformation
  m_cms2lab = m_lab2cms.Inverse();

  m_cmsEnergy = m_boost.M();

  B2INFO("PCmsLabTransform: " << Eher << " " << Eler << " " << cross_angle << " " << angle_ler);

}


