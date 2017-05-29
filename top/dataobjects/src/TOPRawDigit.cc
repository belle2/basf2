/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dataobjects/TOPRawDigit.h>
#include <math.h>

using namespace std;
using namespace Belle2;

double TOPRawDigit::timeErrorCoefficient(double y1, double y2) const
{
  double dy = y2 - y1;
  double ym = m_VPeak / 2.0;
  double a = 1.0 / (2.0 * dy);
  double b = (ym - y2) / (dy * dy);
  double c = (ym - y1) / (dy * dy);

  return sqrt(a * a + b * b + c * c);
}
