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

  double Eher = 7.0 * Unit::GeV;
  double Eler = 4.0 * Unit::GeV;
  double cross_angle = 83 * Unit::mrad;
  double angle = 41.5 * Unit::mrad;
  update(Eher, Eler, cross_angle, angle);
  m_initialized = true;
}


void PCmsLabTransform::update(double Eher, double Eler, double cross_angle, double angle)
{
  // this function is a polished copy of generators/utilities/src/cm2LabBoost.cc

  // Low energy beam
  double angleLerToB = M_PI - angle;
  double pLerZ = Eler * cos(angleLerToB);
  double pLerX = Eler * sin(angleLerToB);
  double pLerY = 0.;
  TLorentzVector vlLer;
  vlLer.SetXYZM(pLerX, pLerY, pLerZ, Const::electronMass);

  // High energy beam
  double angleHerToB = cross_angle - angle;
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

  // now rotate CMS such that z-axis is oriented as incomming e-
  TVector3 zaxis(0., 0., 1.);
  TVector3 rotaxis = zaxis.Cross(vlEleCms.Vect()) * (1. / vlEleCms.Vect().Mag());
  double rotangle = TMath::ASin(rotaxis.Mag());
  m_lab2cms = lab2cmsBoost.Rotate(-rotangle, rotaxis);

  // obtain the backward transformation
  m_cms2lab = m_lab2cms.Inverse();

  m_cmsEnergy = m_boost.M();

  B2INFO("PCmsLabTransform: " << Eher << " " << Eler << " " << cross_angle << " " << angle);

}


