#include "TLorentzVector.h"
#include "TLorentzRotation.h"
#include <framework/gearbox/Unit.h>

using namespace Belle2;

TLorentzRotation getBoost(double Eher, double Eler, double cross_angle, double angle)
{

// Beam parameter
  double Mele = Unit::electron_mass;

//std::cout << Mele << " " << Eher << " " << Eler << " " << cross_angle << " " << angle << std::endl;


// Lab frame Z axis is defined by B files
// its positive direction close to direction of
// high energy beam, i.e. e- beam

  double angle_LerToB = M_PI - angle;
  double Pler_z = Eler * cos(angle_LerToB);
  double Pler_x = Eler * sin(angle_LerToB);
  double Pler_y = 0.;


  double angle_HerToB = cross_angle - angle;

  double Pher_z = Eher * cos(angle_HerToB);
  double Pher_x = Eher * sin(angle_HerToB);
  double Pher_y = 0.;

  /*std::cout << Pler_x << " ";
  std::cout << Pler_y << " ";
  std::cout << Pler_z << " " << std::endl;
  std::cout << Pher_x << " ";
  std::cout << Pher_y << " ";
  std::cout << Pher_z << " " << std::endl;*/



  TLorentzVector vlLer;
  vlLer.SetXYZM(Pler_x, Pler_y, Pler_z, Mele);

  TLorentzVector vlHer;
  vlHer.SetXYZM(Pher_x, Pher_y, Pher_z, Mele);

  TLorentzVector vlY4s = vlHer + vlLer;
  /*
   std::cout << std::endl;
   std::cout << "M(Y4s)  =  " << vlY4s.M() << std::endl;
   std::cout << "E(Y4s)  =  " << vlY4s.E() << std::endl;
   std::cout << std::endl;
   std::cout << "Px(Y4s) =  " << vlY4s.Px() << std::endl;
   std::cout << "Py(Y4s) =  " << vlY4s.Py() << std::endl;
   std::cout << "Pz(Y4s) =  " << vlY4s.Pz() << std::endl;
   std::cout << std::endl;
   std::cout << "theta(Y4s) =  " << vlY4s.Theta() << " (rad)  " << vlY4s.Theta()*TMath::RadToDeg() << " (deg) " << std::endl;
   std::cout << "cosTh(Y4s) =  " << TMath::Cos(vlY4s.Theta()) << std::endl;
   std::cout << "phi(Y4s)   =  " << vlY4s.Phi() << std::endl;
   std::cout << "p_t(Y4s)   =  " << vlY4s.Pt() << std::endl;
   std::cout << "|p(Y4s)|   =  " << vlY4s.Vect().Mag() << std::endl;*/


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
