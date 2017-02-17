/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLMUMUECOLLECTORMODULE_H
#define ECLMUMUECOLLECTORMODULE_H

#include <calibration/CalibrationCollectorModule.h>

namespace Belle2 {

  //..Calibration collector module that uses muon pairs to do ECL single crystal energy calibration
  class eclMuMuECollectorModule : public CalibrationCollectorModule {

  public:

    //..Constructor: Sets the description, the properties and the parameters of the module.
    eclMuMuECollectorModule();
    void prepare();
    void collect();

  private:

    //..Parameters to control the job
    double m_minPairMass; // 9 GeV/c^2 minimum invariant mass of the muon pair
    double m_minTrackLength; // 30 cm minimum extrapolated track length in the crystal
    double m_MaxNeighborAmp; // 200 = 10 MeV maximum signal allowed in a neighboring crystal
    double m_thetaLabMinDeg; // 24 deg miniumum muon theta in lab (degrees); corresponds to L1 trigger
    double m_thetaLabMaxDeg; // 134 deg maximum muon theta in lab (degrees);  corresponds to L1 trigger
    bool m_useTrueEnergy; // false store MC true deposited energy instead of measured

    //..ECL geometry quantities calculated in prepare() and used in collect()
    short nNeighbors[8736] = {};  // number of neighbors for each crystal
    short ListOfNeighbors[8736][20] = {};  // list of the neighbors (cellID0)

    //..Some other useful quantities
    double cotThetaLabMin, cotThetaLabMax;  // M_thetaLabMinDeg and Max, converted to cot
    int pdgmuon = 13;  // muon PDG code
    double MCCalibConstant = 20000.;  // used to convert true deposited energy to ADC counts
    float MuPlab[8736] = {}; // p_lab of e+e- --> mu mu in this crystal
    int iEvent = 0; // event counter

  };
}

#endif /* ECLMUMUECOLLECTORMODULE_H */
