/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz, Martin Heck                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "TLorentzVector.h"
#include "TLorentzRotation.h"
#include <framework/gearbox/Unit.h>
#include <framework/utilities/Boosts.h>

using namespace Belle2;

TLorentzRotation Boosts::getLab2CMSBoost(double EnergyHER, double EnergyLER, double crossAngle, double angle)
{

// Beam parameter
  double mEle = Unit::electron_mass;

// Lab frame Z axis is defined by B files
// its positive direction close to direction of
// high energy beam, i.e. e- beam

  double angleLerToB = TMath::Pi() - angle;
  double pLerZ = EnergyLER * cos(angleLerToB);
  double pLerX = EnergyLER * sin(angleLerToB);
  double pLerY = 0.;


  double angleHerToB = crossAngle - angle;

  double pHerZ = EnergyHER * cos(angleHerToB);
  double pHerX = EnergyHER * sin(angleHerToB);
  double pHerY = 0.;

  TLorentzVector vlLer;
  vlLer.SetXYZM(pLerX, pLerY, pLerZ, mEle);

  TLorentzVector vlHer;
  vlHer.SetXYZM(pHerX, pHerY, pHerZ, mEle);

  TLorentzVector vlY4s = vlHer + vlLer;


// ---------------------------------------------

// Transformation from Lab system to CMS system
  TVector3 cmsBoostVec = vlY4s.BoostVector();
  TLorentzRotation lab2cmsBoost(-cmsBoostVec);

// boost e- to CMS system
  TLorentzVector vlEleCms = lab2cmsBoost * vlHer;

// now rotate CMS such that incomming e- is parallel to z-axis
  TVector3 zaxis(0., 0., 1.);  TVector3 rotaxis = zaxis.Cross(vlEleCms.Vect()) * (1. / vlEleCms.Vect().Mag());
  double rotangle = TMath::ASin(rotaxis.Mag());
  TLorentzRotation lab2cmsBoostRotZ = lab2cmsBoost.Rotate(-rotangle, rotaxis);


// boost lab e- and e+ to properly rotated CMS system
  TLorentzVector vlEleCmsR = lab2cmsBoostRotZ * vlHer;
  TLorentzVector vlPosCmsR = lab2cmsBoostRotZ * vlLer;

// obtain the backward transformation
  TLorentzRotation cms2labBoostRotZ = lab2cmsBoostRotZ.Inverse();
  /*    for(int i=0; i<4; i++){
         for(int j=0;j<4;j++){
             std::cout <<  cms2labBoostRotZ(i,j) << " ";
         }
          std::cout << std::endl;
     }*/

  return cms2labBoostRotZ;
}

TLorentzRotation Boosts::getCMS2LabBoost(double EnergyHER, double EnergyLER, double crossAngle, double angle)
{
  return (Boosts::getLab2CMSBoost(EnergyHER, EnergyLER, crossAngle, angle).Inverse());
}
