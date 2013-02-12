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
