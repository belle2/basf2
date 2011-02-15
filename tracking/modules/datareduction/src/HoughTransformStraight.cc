#include "HoughTransformStraight.h"
#include <cmath>

#include <list>


using namespace std;


HoughTransformStraight::HoughTransformStraight() : HoughTransformBasic()
{
  _acceptanceAngles[0] = 0.2967; //17 degree
  _acceptanceAngles[1] = 2.6179; //150 degree
  _zRange[0] = -1.0;
  _zRange[1] = 1.0;
}


HoughTransformStraight::~HoughTransformStraight()
{

}


void HoughTransformStraight::setAcceptanceAngles(double minAngle, double maxAngle)
{
  _acceptanceAngles[0] = minAngle;
  _acceptanceAngles[1] = maxAngle;
}


void HoughTransformStraight::setZRange(double minZ, double maxZ)
{
  _zRange[0] = minZ;
  _zRange[1] = maxZ;
}


void HoughTransformStraight::getInitialHoughBoxParams(double& min_a, double& max_a, double& min_b, double& max_b)
{
  double tanPos = tan((0.5 * M_PI) - _acceptanceAngles[0]);
  double tanNeg = tan((0.5 * M_PI) - _acceptanceAngles[1]);
  min_a = min(tanPos, tanNeg);
  max_a = max(tanPos, tanNeg);

  min_b = min(_zRange[0], _zRange[1]);
  max_b = max(_zRange[0], _zRange[1]);
}


bool HoughTransformStraight::isHitInHoughBox(HoughTransformBox& houghBox, rzHit& hit)
{
  double tLeft  = hit.z - (houghBox.left * hit.r);
  double tRight = hit.z - (houghBox.right * hit.r);

  if ((tLeft >= houghBox.bottom) && (tLeft <= houghBox.top)) return true;
  if ((tRight >= houghBox.bottom) && (tRight <= houghBox.top)) return true;
  if ((tLeft < houghBox.bottom) && (tRight > houghBox.top)) return true;
  if ((tLeft > houghBox.top) && (tRight < houghBox.bottom)) return true;

  return false;
}


double HoughTransformStraight::getZValue(double a, double b, double r)
{
  return a*r + b;
}



