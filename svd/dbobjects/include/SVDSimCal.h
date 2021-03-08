/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {

  /** contains the simulation calibration parameters*/
  struct SVDSimCal {

    /** Geant4-electron to real-electron weight. */
    float electronWeight;

    /** Coupling constant; used to parametrise signal coupling among strips in simulation.*/
    std::map<std::string, float> couplingConstant;

    ClassDef(SVDSimCal, 1); /**< needed by root*/

  };

}
