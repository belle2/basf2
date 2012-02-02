/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// Implementation:
//     *  using cellId internally, hitId is only for PANTHER i/o
//
//     1. init parameters(cuts, eatables, sw)
//     2. accumulate hits
//     3. search CRs
//     4. loop over CRs
//        4-1  seeds search
//        4-2  hit assignments(shower recon)
//        4-3  initial shower attributes(e, position)
//     [] if corr then E/Angle corr
//     5. CR/Shower Attrs

#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/rec_lib/TEclCFCR.h>
#include <ecl/rec_lib/TRecEclCF.h>

#include <cmath>
#include <boost/format.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <functional>
#include <utility>
//#include <stack>
#include <vector>
#include <map>

#include "TVector3.h"

using namespace std;
using namespace boost;
using namespace Belle2;

/* for new correction */
const float TRecEclCFParameters::fgEthX3[69] = {
  2.50, 2.50, 2.50, 2.50, 2.50, 2.40, 2.29, 2.18, 2.07, 1.98,
  1.88, 1.78, 1.69, 1.61, 1.57, 1.56, 1.57, 1.58, 1.60, 1.62,
  1.63, 1.64, 1.65, 1.65, 1.65, 1.65, 1.65, 1.65, 1.64, 1.64,
  1.63, 1.62, 1.61, 1.60, 1.59, 1.57, 1.56, 1.54, 1.53, 1.51,
  1.49, 1.47, 1.45, 1.43, 1.41, 1.39, 1.37, 1.35, 1.33, 1.31,
  1.29, 1.28, 1.28, 1.28, 1.29, 1.32, 1.38, 1.46, 1.56, 1.67,
  1.80, 1.95, 2.12, 2.24, 2.24, 2.35, 2.50, 2.50, 2.50
};
//
// constructors and destructor
//
TRecEclCFParameters::TRecEclCFParameters() :
  fEthX3(1)
{
  fEthHit  = 0.0005;   // 500 KeV zero suppression

// makes too many spurious showers
// do not have any illusions about using ~10MeV showers in physics...
//   fEthSeed = 0.005;    // 5 MeV
//
  fEthSeed = 0.010;    // 10 MeV

  fEthCluster = 0.02;   // NA  10~20 MeV?  z.s. or join
  fEthCR = 0.02;        // NA  20 MeV?  z.s. or join
  fEthCRSearch = fEthHit;  // 2 MeV in cleo, we are using diff algo...

  // correction table should be prepared.
  fEnergyAngleCorrection = 0;   // true; basf does not allow bool...

  /* moved to TComEclParas */
  //fGarbageCollection = false;
}

// TRecEclCFParameters::TRecEclCFParameters( const TRecEclCFParameters& )
// {
// }

TRecEclCFParameters::~TRecEclCFParameters()
{
}
