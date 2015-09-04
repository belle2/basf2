/*
 * ColorMaps.cc
 *
 *  Created on: Feb 26, 2015
 *      Author: dschneider
 */

#include <tracking/trackFindingCDC/display/ColorMaps.h>

using namespace Belle2;
using namespace TrackFindingCDC;

double HLSToRGB::hueToRgb(const double p, const double q, double t)
{
  if (t < 0) t += 1;
  if (t > 1) t -= 1;
  if (t < 1. / 6.) {
    return p + (q - p) * 6 * t;
  }
  if (t < 1. / 2.) {
    return q;
  }
  if (t < 2. / 3.) {
    return p + (q - p) * (2. / 3. - t) * 6;
  }
  return p;
}

std::vector<double> HLSToRGB::hlsToRgb(const double h, const double l, const double s)
{
  std::vector<double> result;

  if (s == 0) {
    result.push_back(l);
    result.push_back(l);
    result.push_back(l);
  } else {
    double q = l < 0.5 ? l * (1 + s) : l + s - l * s;
    double p = 2 * l - q;
    result.push_back(hueToRgb(p, q, h + 1. / 3.)); //r
    result.push_back(hueToRgb(p, q, h));//g
    result.push_back(hueToRgb(p, q, h - 1. / 3.)); //b
  }
  return result;
}
