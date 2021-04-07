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
  struct SVDChargeSimCal {

    /** Geant4-electron to real-electron weight. */
    float electronWeight;

    /** Coupling constant; used to parametrise signal coupling among strips in simulation.
     * Currently we have 4 couplings implemented:
     * couplingConstant["C0"]  |  Coupling from readout implant to its readout channel
     * couplingConstant["C1"]  |  Coupling from first neighbour implant (floating) to readout channel
     * couplingConstant["C2"]  |  Coupling from second neighbour implant (readout) to readout channel
     * couplingConstant["C3"]  |  Coupling from third neighbour implant (floating) to readout channel
     */
    std::map<std::string, float> couplingConstant;

    ClassDef(SVDChargeSimCal, 1); /**< needed by root*/

  };

}
