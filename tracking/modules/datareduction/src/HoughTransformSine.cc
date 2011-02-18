#include <tracking/modules/datareduction/HoughTransformSine.h>
#include <cmath>


using namespace std;
using namespace Belle2;


HoughTransformSine::HoughTransformSine() : HoughTransformBasic()
{
  _acceptanceAngles[0] = 0.2967; //17 degree
  _acceptanceAngles[1] = 2.6179; //150 degree
  _radius[0] = 1e2;
  _radius[1] = 1e3;
}


HoughTransformSine::~HoughTransformSine()
{

}


void HoughTransformSine::setRadius(double minRadius, double maxRadius)
{
  _radius[0] = minRadius;
  _radius[1] = maxRadius;
}


void HoughTransformSine::setAcceptanceAngles(double minAngle, double maxAngle)
{
  _acceptanceAngles[0] = minAngle;
  _acceptanceAngles[1] = maxAngle;
}


void HoughTransformSine::getInitialHoughBoxParams(double& min_a, double& max_a, double& min_b, double& max_b)
{
  min_b = 1 / (2.0 * max(_radius[0], _radius[1]));
  max_b = 1 / (2.0 * min(_radius[0], _radius[1]));
  min_a = -0.078; //-0.1002;//max_b * tan(_acceptanceAngles[1]);
  max_a = 0.078; //0.1002;//max_b * tan(_acceptanceAngles[0]);
  //max_a = 7 * cos(_acceptanceAngles[0])*1.5/2;
  //min_a = 4 * cos(_acceptanceAngles[1])*1.5/2;
}


bool HoughTransformSine::isHitInHoughBox(HoughTransformBox& houghBox, rzHit& hit)
{
  double bLeft  = sin(houghBox.left * hit.z) / hit.r;
  double bRight = sin(houghBox.right * hit.z) / hit.r;

  if ((bLeft >= houghBox.bottom) && (bLeft <= houghBox.top)) return true;
  if ((bRight >= houghBox.bottom) && (bRight <= houghBox.top)) return true;
  if ((bLeft < houghBox.bottom) && (bRight > houghBox.top)) return true;
  if ((bLeft > houghBox.top) && (bRight < houghBox.bottom)) return true;

  double period = fabs(2 * M_PI / hit.z);
  double offset = hit.z > 0 ? 0.25 : 0.75;
  double n = floor(houghBox.left / period);

  //Both sides below, maybe maximum in between
  if ((bLeft < houghBox.bottom) && (bRight < houghBox.bottom)) {
    double nextMax = (n + offset) * period;
    while (nextMax < houghBox.left) nextMax += period;
    if (nextMax < houghBox.right) {
      if (1 / hit.r > houghBox.bottom) return true;
    }
  }

  //Both sides above, maybe minimum in between
  if ((bLeft > houghBox.bottom) && (bRight > houghBox.bottom)) {
    double nextMin = (n + 1 - offset) * period;
    while (nextMin < houghBox.left) nextMin += period;
    if (nextMin < houghBox.right) {
      if (houghBox.top >= 0) return true;
    }
  }

  return false;
}


double HoughTransformSine::getZValue(double a, double b, double r)
{
  return (1 / a) * asin(b*r);
}
