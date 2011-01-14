/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Zbynek Drasal                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdDigitizer/PXDcheater.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

double PXDcheater::getPitchZ(int iLayer, int iLadder, int iSensor) const
{
  switch (iLayer) {
    case 1:
      return m_pitchZ1 * Unit::um;
    case 2:
      return m_pitchZ2 * Unit::um;
    default:
      B2FATAL("PXDcheater: Incorrect layer number - should not happen!");
  }
  return 0.0; // in case...
}

double PXDcheater::getPitchRPhi(int iLayer, int iLadder, int iSensor) const
{
  switch (iLayer) {
    case 1:
      return m_pitchRPhi1 * Unit::um;
    case 2:
      return m_pitchRPhi2 * Unit::um;
    default:
      B2FATAL("PXDcheater: Incorrect layer number - should not happen!");
  }
  return 0.0; // in case...
}


double PXDcheater::getResolutionZ(int iLayer, int iLadder, int iSensor, double theta) const
{
  switch (iLayer) {
    case 1:
      return PXD_ResFunc_z1(theta / Unit::deg) * Unit::um;
    case 2:
      return PXD_ResFunc_z2(theta / Unit::deg) * Unit::um;
    default:
      B2FATAL("PXDcheater: Incorrect layer number - should not happen!");
  }
  return 0.0; // in case...
}

double PXDcheater::getResolutionRPhi(int iLayer, int iLadder, int iSensor, double theta) const
{
  switch (iLayer) {
    case 1:
      return PXD_ResFunc_rphi1(theta / Unit::deg) * Unit::um;
    case 2:
      return PXD_ResFunc_rphi2(theta / Unit::deg) * Unit::um;
    default:
      B2FATAL("PXDcheater: Incorrect layer number - should not happen!");
  }
  return 0.0; // in case...
}

/* Machine-generated spline interpolants */

double PXDcheater::PXD_ResFunc_rphi1(double x) const
{
  const int fNp = 33, fKstep = 0;
  const double fDelta = -1, fXmin = -180, fXmax = 180;
  const double fX[33] = { -180, -170, -160, -150, -140,
                          -130, -120, -110, -90, -70,
                          -60, -50, -40, -30, -20,
                          -10, 0, 10, 20, 30,
                          40, 50, 60, 70, 90,
                          110, 120, 130, 140, 150,
                          160, 170, 180
                        };
  const double fY[33] = { 0.10846, 0.110011, 0.110375, 0.110619, 0.110132,
                          0.111235, 0.111857, 0.109529, 0.0985761, 0.109529,
                          0.111857, 0.111235, 0.110132, 0.110619, 0.110375,
                          0.110011, 0.10846, 0.110011, 0.110375, 0.110619,
                          0.110132, 0.111235, 0.111857, 0.109529, 0.0985761,
                          0.109529, 0.111857, 0.111235, 0.110132, 0.110619,
                          0.110375, 0.110011, 0.10846
                        };
  const double fB[33] = { 0.000274818, 6.55893e-05, 3.74467e-05, -3.28356e-05, 2.09606e-05,
                          0.000133564, -3.78175e-05, -0.000493998, 9.50894e-09, 0.00049396,
                          3.79221e-05, -0.000133944, -1.95438e-05, 2.75486e-05, -1.77156e-05,
                          -0.000139227, -5.81022e-21, 0.000139227, 1.77156e-05, -2.75486e-05,
                          1.95438e-05, 0.000133944, -3.79221e-05, -0.00049396, -9.50894e-09,
                          0.000493998, 3.78175e-05, -0.000133564, -2.09606e-05, 3.28356e-05,
                          -3.74467e-05, -6.55893e-05, -0.000274818
                        };
  const double fC[33] = { -1.49886e-05, -5.93428e-06, 3.12002e-06, -1.01482e-05, 1.55279e-05,
                          -4.26752e-06, -1.28706e-05, -3.27474e-05, 5.74478e-05, -3.27503e-05,
                          -1.28535e-05, -4.33314e-06, 1.57732e-05, -1.1064e-05, 6.53753e-06,
                          -1.86886e-05, 3.26113e-05, -1.86886e-05, 6.53753e-06, -1.1064e-05,
                          1.57732e-05, -4.33314e-06, -1.28535e-05, -3.27503e-05, 5.74478e-05,
                          -3.27474e-05, -1.28706e-05, -4.26752e-06, 1.55279e-05, -1.01482e-05,
                          3.12002e-06, -5.93428e-06, 10
                        };
  const double fD[33] = { 3.0181e-07, 3.0181e-07, -4.42276e-07, 8.55871e-07, -6.59847e-07,
                          -2.8677e-07, -6.6256e-07, 1.50325e-06, -1.5033e-06, 6.63226e-07,
                          2.84013e-07, 6.70211e-07, -8.94572e-07, 5.86716e-07, -8.40872e-07,
                          1.71e-06, -1.71e-06, 8.40872e-07, -5.86716e-07, 8.94572e-07,
                          -6.70211e-07, -2.84013e-07, -6.63226e-07, 1.5033e-06, -1.50325e-06,
                          6.6256e-07, 2.8677e-07, 6.59847e-07, -8.55871e-07, 4.42276e-07,
                          -3.0181e-07, -3.0181e-07, 4.64102
                        };
  int klow = 0;
  if (x <= fXmin) klow = 0;
  else if (x >= fXmax) klow = fNp - 1;
  else {
    if (fKstep) {
      // Equidistant knots, use histogramming
      klow = int((x - fXmin) / fDelta);
      if (klow < fNp - 1) klow = fNp - 1;
    } else {
      int khig = fNp - 1, khalf;
      // Non equidistant knots, binary search
      while (khig - klow > 1)
        if (x > fX[khalf=(klow+khig)/2]) klow = khalf;
        else khig = khalf;
    }
  }
  // Evaluate now
  double dx = x - fX[klow];
  return (fY[klow] + dx*(fB[klow] + dx*(fC[klow] + dx*fD[klow])));
}

double PXDcheater::PXD_ResFunc_rphi2(double x) const
{
  const int fNp = 31, fKstep = 0;
  const double fDelta = -1, fXmin = -180, fXmax = 160;
  const double fX[31] = { -180, -170, -160, -150, -140,
                          -130, -120, -110, -90, -70,
                          -60, -50, -40, -30, -20,
                          -10, 0, 10, 20, 30,
                          40, 50, 60, 70, 90,
                          110, 120, 130, 140, 150,
                          160
                        };
  const double fY[31] = { 0.129534, 0.12987, 0.130208, 0.130039, 0.13369,
                          0.13089, 0.130378, 0.13089, 0.117801, 0.13089,
                          0.130378, 0.13089, 0.13369, 0.130039, 0.130208,
                          0.12987, 0.129534, 0.12987, 0.130208, 0.130039,
                          0.13369, 0.13089, 0.130378, 0.13089, 0.117801,
                          0.13089, 0.130378, 0.13089, 0.13369, 0.130039,
                          0.130208
                        };
  const double fB[31] = { -7.49043e-05, 8.79636e-05, -7.45539e-05, 0.000260916, 7.53403e-05,
                          -0.000306965, 0.000158999, -0.000329029, -2.59176e-09, 0.000329039,
                          -0.000159027, 0.000307069, -7.57265e-05, -0.000259475, 6.9176e-05,
                          -6.78931e-05, 5.78703e-12, 6.78931e-05, -6.91759e-05, 0.000259475,
                          7.57276e-05, -0.000307073, 0.000159043, -0.000329097, 3.21457e-07,
                          0.000327812, -0.000155507, 0.000294215, -2.78305e-05, -0.000438205,
                          0.000736199
                        };
  const double fC[31] = { 1.6278e-05, 8.76175e-09, -1.62605e-05, 4.98075e-05, -6.83651e-05,
                          3.01346e-05, 1.64618e-05, -6.52645e-05, 8.17158e-05, -6.52638e-05,
                          1.64572e-05, 3.01524e-05, -6.8432e-05, 5.00571e-05, -1.7192e-05,
                          3.48508e-06, 3.30423e-06, 3.48508e-06, -1.7192e-05, 5.00571e-05,
                          -6.84318e-05, 3.01517e-05, 1.64599e-05, -6.52739e-05, 8.17448e-05,
                          -6.53703e-05, 1.70385e-05, 2.79337e-05, -6.01382e-05, 1.91007e-05,
                          10
                        };
  const double fD[31] = { -5.42309e-07, -5.42309e-07, 2.20227e-06, -3.93909e-06, 3.28332e-06,
                          -4.55758e-07, -2.72421e-06, 2.44967e-06, -2.44966e-06, 2.72403e-06,
                          4.56509e-07, -3.28615e-06, 3.94964e-06, -2.24164e-06, 6.89236e-07,
                          -6.02858e-09, 6.02847e-09, -6.89235e-07, 2.24163e-06, -3.94963e-06,
                          3.28612e-06, -4.56395e-07, -2.72446e-06, 2.45031e-06, -2.45192e-06,
                          2.74696e-06, 3.63172e-07, -2.93573e-06, 2.6413e-06, 2.6413e-06,
                          4.64097
                        };
  int klow = 0;
  if (x <= fXmin) klow = 0;
  else if (x >= fXmax) klow = fNp - 1;
  else {
    if (fKstep) {
      // Equidistant knots, use histogramming
      klow = int((x - fXmin) / fDelta);
      if (klow < fNp - 1) klow = fNp - 1;
    } else {
      int khig = fNp - 1, khalf;
      // Non equidistant knots, binary search
      while (khig - klow > 1)
        if (x > fX[khalf=(klow+khig)/2]) klow = khalf;
        else khig = khalf;
    }
  }
  // Evaluate now
  double dx = x - fX[klow];
  return (fY[klow] + dx*(fB[klow] + dx*(fC[klow] + dx*fD[klow])));
}

double PXDcheater::PXD_ResFunc_z1(double x) const
{
  const int fNp = 33, fKstep = 0;
  const double fDelta = -1, fXmin = -180, fXmax = 180;
  const double fX[33] = { -180, -170, -160, -150, -140,
                          -130, -120, -110, -90, -70,
                          -60, -50, -40, -30, -20,
                          -10, 0, 10, 20, 30,
                          40, 50, 60, 70, 90,
                          110, 120, 130, 140, 150,
                          160, 170, 180
                        };
  const double fY[33] = { 0.075188, 0.0884956, 0.115741, 0.178253, 0.195695,
                          0.145985, 0.113507, 0.0995025, 0.0497512, 0.0995025,
                          0.113507, 0.145985, 0.195695, 0.178253, 0.115741,
                          0.0884956, 0.075188, 0.0884956, 0.115741, 0.178253,
                          0.195695, 0.145985, 0.113507, 0.0995025, 0.0497512,
                          0.0995025, 0.113507, 0.145985, 0.195695, 0.178253,
                          0.115741, 0.0884956, 0.075188
                        };
  const double fB[33] = { 0.00231214, 0.00118851, 0.00509966, 0.00534013, -0.002474,
                          -0.00512445, -0.0016844, -0.00208282, 8.00022e-08, 0.0020825,
                          0.00168528, 0.00512125, 0.00248592, -0.00538462, -0.00493365,
                          -0.00180805, -2.78891e-19, 0.00180805, 0.00493365, 0.00538462,
                          -0.00248592, -0.00512125, -0.00168528, -0.0020825, -8.00022e-08,
                          0.00208282, 0.0016844, 0.00512445, 0.002474, -0.00534013,
                          -0.00509966, -0.00118851, -0.00231214
                        };
  const double fC[33] = { -0.000182051, 6.96878e-05, 0.000321427, -0.000297379, -0.000484034,
                          0.000218989, 0.000125015, -0.000164857, 0.000269002, -0.000164881,
                          0.000125159, 0.000218437, -0.00048197, -0.000305083, 0.00035018,
                          -3.76191e-05, 0.000218424, -3.76191e-05, 0.00035018, -0.000305083,
                          -0.00048197, 0.000218437, 0.000125159, -0.000164881, 0.000269002,
                          -0.000164857, 0.000125015, 0.000218989, -0.000484034, -0.000297379,
                          0.000321427, 6.96878e-05, 10
                        };
  const double fD[33] = { 8.3913e-06, 8.3913e-06, -2.06269e-05, -6.22185e-06, 2.34341e-05,
                          -3.13247e-06, -9.6624e-06, 7.23097e-06, -7.23137e-06, 9.668e-06,
                          3.10927e-06, -2.33469e-05, 5.89624e-06, 2.18421e-05, -1.29266e-05,
                          8.53476e-06, -8.53476e-06, 1.29266e-05, -2.18421e-05, -5.89624e-06,
                          2.33469e-05, -3.10927e-06, -9.668e-06, 7.23137e-06, -7.23097e-06,
                          9.6624e-06, 3.13247e-06, -2.34341e-05, 6.22185e-06, 2.06269e-05,
                          -8.3913e-06, -8.3913e-06, 4.64102
                        };
  int klow = 0;
  if (x <= fXmin) klow = 0;
  else if (x >= fXmax) klow = fNp - 1;
  else {
    if (fKstep) {
      // Equidistant knots, use histogramming
      klow = int((x - fXmin) / fDelta);
      if (klow < fNp - 1) klow = fNp - 1;
    } else {
      int khig = fNp - 1, khalf;
      // Non equidistant knots, binary search
      while (khig - klow > 1)
        if (x > fX[khalf=(klow+khig)/2]) klow = khalf;
        else khig = khalf;
    }
  }
  // Evaluate now
  double dx = x - fX[klow];
  return (fY[klow] + dx*(fB[klow] + dx*(fC[klow] + dx*fD[klow])));
}

double PXDcheater::PXD_ResFunc_z2(double x) const
{
  const int fNp = 33, fKstep = 0;
  const double fDelta = -1, fXmin = -180, fXmax = 180;
  const double fX[33] = { -180, -170, -160, -150, -140,
                          -130, -120, -110, -90, -70,
                          -60, -50, -40, -30, -20,
                          -10, 0, 10, 20, 30,
                          40, 50, 60, 70, 90,
                          110, 120, 130, 140, 150,
                          160, 170, 180
                        };
  const double fY[33] = { 0.0526316, 0.0591716, 0.070922, 0.0925926, 0.137363,
                          0.143472, 0.109769, 0.0884956, 0.0442478, 0.0884956,
                          0.109769, 0.143472, 0.137363, 0.0925926, 0.070922,
                          0.0591716, 0.0526316, 0.0591716, 0.070922, 0.0925926,
                          0.137363, 0.143472, 0.109769, 0.0884956, 0.0442478,
                          0.0884956, 0.109769, 0.143472, 0.137363, 0.0925926,
                          0.070922, 0.0591716, 0.0526316
                        };
  const double fB[33] = { 0.000269465, 0.00097653, 0.00131154, 0.00380362, 0.0034062,
                          -0.00216457, -0.00302585, -0.00222497, 9.32371e-09, 0.00222493,
                          0.00302595, 0.0021642, -0.00340481, -0.0038088, -0.00129219,
                          -0.00104873, -9.29636e-20, 0.00104873, 0.00129219, 0.0038088,
                          0.00340481, -0.0021642, -0.00302595, -0.00222493, -9.32371e-09,
                          0.00222497, 0.00302585, 0.00216457, -0.0034062, -0.00380362,
                          -0.00131154, -0.00097653, -0.000269465
                        };
  const double fC[33] = { 4.46547e-05, 2.60518e-05, 7.44902e-06, 0.000241759, -0.0002815,
                          -0.000275577, 0.000189449, -0.000109362, 0.000220611, -0.000109365,
                          0.000189466, -0.000275641, -0.00028126, 0.000240861, 1.08e-05,
                          1.3546e-05, 9.13273e-05, 1.3546e-05, 1.08e-05, 0.000240861,
                          -0.00028126, -0.000275641, 0.000189466, -0.000109365, 0.000220611,
                          -0.000109362, 0.000189449, -0.000275577, -0.0002815, 0.000241759,
                          7.44902e-06, 2.60518e-05, 10
                        };
  const double fD[33] = { -6.20094e-07, -6.20094e-07, 7.81032e-06, -1.7442e-05, 1.9746e-07,
                          1.55009e-05, -9.96037e-06, 5.49954e-06, -5.49959e-06, 9.96102e-06,
                          -1.55036e-05, -1.87297e-07, 1.7404e-05, -7.66869e-06, 9.15332e-08,
                          2.59271e-06, -2.59271e-06, -9.15332e-08, 7.66869e-06, -1.7404e-05,
                          1.87297e-07, 1.55036e-05, -9.96102e-06, 5.49959e-06, -5.49954e-06,
                          9.96037e-06, -1.55009e-05, -1.9746e-07, 1.7442e-05, -7.81032e-06,
                          6.20094e-07, 6.20094e-07, 4.64102
                        };
  int klow = 0;
  if (x <= fXmin) klow = 0;
  else if (x >= fXmax) klow = fNp - 1;
  else {
    if (fKstep) {
      // Equidistant knots, use histogramming
      klow = int((x - fXmin) / fDelta);
      if (klow < fNp - 1) klow = fNp - 1;
    } else {
      int khig = fNp - 1, khalf;
      // Non equidistant knots, binary search
      while (khig - klow > 1)
        if (x > fX[khalf=(klow+khig)/2]) klow = khalf;
        else khig = khalf;
    }
  }
  // Evaluate now
  double dx = x - fX[klow];
  return (fY[klow] + dx*(fB[klow] + dx*(fC[klow] + dx*fD[klow])));
}

