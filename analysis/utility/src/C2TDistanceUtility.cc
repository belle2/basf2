// ******************************************************************
// Particle copy utility
// authors: M. Lubej (matic.lubej@ijs.si)
// ******************************************************************

#include <analysis/utility/C2TDistanceUtility.h>
#include <TMath.h>

using namespace Belle2;
using namespace std;

TVector3 C2TDistanceUtility::clipECLClusterPosition(TVector3 v1)
{
  double cylRadiusECLInCM = 125.80;
  double forwardZECLInCM = 196.5;
  double backwardZECLInCM = -102.0;
  float theta = v1.Theta();
  float mag = v1.Mag();

  if (theta < 0.555 and v1.Z() > forwardZECLInCM) {
    double alpha = TMath::PiOver2() - theta;
    float dz = v1.Z() - forwardZECLInCM;
    float dR = dz / sin(alpha);
    v1.SetMag(mag - dR);
  } else if (theta >= 0.555 and theta < TMath::PiOver2() and v1.Perp() > cylRadiusECLInCM) {
    double alpha = theta;
    float dcylR = v1.Perp() - cylRadiusECLInCM;
    float dR = dcylR / sin(alpha);
    v1.SetMag(mag - dR);
  } else if (theta >= TMath::PiOver2() and theta < 2.26 and v1.Perp() > cylRadiusECLInCM) {
    double alpha = TMath::Pi() - theta;
    float dcylR = v1.Perp() - cylRadiusECLInCM;
    float dR = dcylR / sin(alpha);
    v1.SetMag(mag - dR);
  } else if (theta >= 2.26 and v1.Z() < backwardZECLInCM) {
    double alpha = theta - TMath::PiOver2();
    float dz = backwardZECLInCM - v1.Z();
    float dR = dz / sin(alpha);
    v1.SetMag(mag - dR);
  }

  return v1;
}

TVector3 C2TDistanceUtility::getECLTrackHitPosition(const Helix& helix, const TVector3& v1)
{
  double cylRadiusECLInCM = 125.80;
  TVector3 badVector(1, 1, 1);
  badVector.SetMag(999.9);
  TVector3 v2;

  double arcLength = helix.getArcLength2DAtCylindricalR(cylRadiusECLInCM);

  if (!std::isnan(arcLength)) {
    double forwardZECLInCM = 196.5;
    double backwardZECLInCM = -102.0;
    v2 = helix.getPositionAtArcLength2D(arcLength);
    if (v2.Z() > forwardZECLInCM or v2.Z() < backwardZECLInCM) {
      arcLength = helix.getArcLength2DAtCylindricalR(v1.Perp());
      if (std::isnan(arcLength))
        return badVector;
      v2 = helix.getPositionAtArcLength2D(arcLength);
    } else
      v2 = helix.getPositionAtArcLength2D(arcLength);
  } else if (std::isnan(arcLength)) {
    arcLength = helix.getArcLength2DAtCylindricalR(v1.Perp());
    if (std::isnan(arcLength))
      return badVector;
    v2 = helix.getPositionAtArcLength2D(arcLength);
  }

  return v2;
}
