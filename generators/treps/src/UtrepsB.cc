/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kiyoshi Hayasaka                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <TLorentzVector.h>
#include <generators/treps/UtrepsB.h>

using namespace Belle2;


UtrepsB::UtrepsB(void) : TrepsB()
{
}

void UtrepsB::initg(void)
{
  B2DEBUG(10, " UtrepsB initg  Pmodel " << TrepsB::pmodel);

  //parametrization of pi+pi- partial waves

  static double data001[21] = { 20., 15., 9., 8., 8., 7., 5.3, 5.2,
                                5.2, 5.2, 5.2, 4., 3., 2., 2.,
                                2., 2., 2., 1., 0., 0.
                              };
  static double data201[21] = { 1., 5., 5, 5., 4., 4., 4., 4.,
                                3., 3., 3., 2., 1., 1., 1.,
                                1., 0., 0., 0., 1., 1.
                              };
  static double data221[21] = { 1., 5., 5., 6., 7., 7.18, 6.68, 6.27,
                                5.91, 5.61, 5., 5., 4., 3., 2.,
                                2., 2., 1., 0., 0., 0.
                              };

  static double data002[15] = { 2., 2., 2., 2., 2., 2., 3., 4.,
                                4.5, 5., 5., 5., 4.5, 4.5, 4.3
                              };
  static double data202[15] = { 1., 2., 3., 4., 5., 5., 5., 5.,
                                5., 5., 5., 4., 3.0, 2.2, 1.
                              };
  static double data222[15] = { 0., 0., 0., 0., 0., 0., -1., -3.,
                                -3., -3., -3., -4., -3.5, -3., -2.5
                              };

  static double data003[9] = { 2.3, 2.3, 7.26, 7.94, 8.53, 7.38, 3.25, 1.98, 2.30};
  static double data203[9] = { 9.63, 9.63, 10.73, 8.02, 6.18, 3.37, 0.63, 0.10, 0.66};
  static double data223[9] = { 1.48, 1.48, -4.62, -6.12, -6.78, -5.35, -1.82, -1.02, -1.63};


  if (TrepsB::pmodel == 251) {
    // pi+pi-
    b00 = Interps(0.3, 2.3, 20, data001);
    b20 = Interps(0.3, 2.3, 20, data201);
    b22 = Interps(0.3, 2.3, 20, data221);
  }

  if (TrepsB::pmodel == 252) {
    // K+K-
    b00 = Interps(1.0, 2.4, 14, data002);
    b20 = Interps(1.0, 2.4, 14, data202);
    b22 = Interps(1.0, 2.4, 14, data222);
  }

  if (TrepsB::pmodel == 253) {
    // ppbar
    b00 = Interps(2.05, 2.85, 8, data003);
    b20 = Interps(2.05, 2.85, 8, data203);
    b22 = Interps(2.05, 2.85, 8, data223);
  }


  return;
}

double UtrepsB::d2func(double y) const
{
  return 1. / (9. + 3.*y * y + y * y * y * y);
}

double UtrepsB::qfunc(double y, double mm) const
{
  return sqrt(0.25 * y - mm * mm);
}

