/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *                                                                        *
 * 7/31: Poyuan                                                           *
 *                                                                        *
 *                                                                        *
 **************************************************************************/
#ifndef TRECECLCFPARAMETERS_H
#define TRECECLCFPARAMETERS_H

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/rec_lib/TEclCFShower.h>
#include "TVector3.h"


namespace Belle2 {

  struct TRecEclCFParameters {
  public:
    TRecEclCFParameters();
    virtual ~TRecEclCFParameters();

  public:
    /** Initial Parameters,
     *need different thresholds for endcap & barrel eventually
     */
//@{
    //EclGeV
    /// single-crystal threshold;(for MC data for now)
    static
    const float fgEthX3[69];

    double fEthHit;

    int     fEthX3;

    //EclGeV
    /// for a seed candidate,
    double fEthSeed;

    //EclGeV
    /// for a cluster(NA)
    double fEthCluster;

    //EclGeV
    /// for a connected region(NA)
    double fEthCR;

    //EclGeV
    ///(not used yet)
    double fEthCRSearch;
//@}
  public:
//Control switches
    /// corr, cone, garbage collection ...
    int fEnergyAngleCorrection;

    //we cannot use "bool" to use basf define_parameters...
    // and we cannot use user defined type also...
    //and we need to open backdoor, cannot hide data-members
    // to define user parameters...

  };

} // end of namespace Belle2

#endif
