#include "TLorentzVector.h"
#include "TLorentzRotation.h"
#include <framework/gearbox/Const.h>
#include <generators/utilities/cm2LabBoost.h>

using namespace Belle2;

TLorentzRotation getBoost(double Eher, double Eler, double cross_angle, double angle)
{

// Lab frame Z axis is defined by B files
// its positive direction close to direction of
// high energy beam, i.e. e- beam

  double angleLerToB = M_PI - angle;
  double pLerZ = Eler * cos(angleLerToB);
  double pLerX = Eler * sin(angleLerToB);
  double pLerY = 0.;


  double angleHerToB = cross_angle - angle;

  double pHerZ = Eher * cos(angleHerToB);
  double pHerX = Eher * sin(angleHerToB);
  double pHerY = 0.;

  TLorentzVector vlLer;
  vlLer.SetXYZM(pLerX, pLerY, pLerZ, Const::electronMass);

  TLorentzVector vlHer;
  vlHer.SetXYZM(pHerX, pHerY, pHerZ, Const::electronMass);

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
  TLorentzVector vlEleCmsR;
  TLorentzVector vlPosCmsR;

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

double getBeamEnergySpreadCM(double E1, double dE1,
                             double E2, double dE2,
                             double crossing_angle)
{
  // To get fluctuation of root(s) from
  // the beam energy, its energy spread and crossing angle
  // at the lab. frame.
  double m = Const::electronMass;
  double ca = cos(crossing_angle);

  double P1 = sqrt(E1 * E1 - m * m);
  double P2 = sqrt(E2 * E2 - m * m);
  Double_t Etotcm = sqrt(2.*m * m + 2.*(E1 * E2 + P1 * P2 * ca));
  Double_t dEdE1 = (P1 * E2 + P2 * E1 * ca) / P1 / Etotcm;
  Double_t dEdE2 = (P1 * E2 * ca + P2 * E1) / P2 / Etotcm;
  Double_t dEtotcm = sqrt((dE1 * dEdE1) * (dE1 * dEdE1) + (dE2 * dEdE2) * (dE2 * dEdE2));
  return dEtotcm;
}
