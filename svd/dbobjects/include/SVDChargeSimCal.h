/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

namespace Belle2 {

  /** contains the simulation calibration parameters*/
  struct SVDChargeSimCal {

    /** Geant4-electron to real-electron weight. */
    float electronWeight;

    /** Coupling constant; used to parametrise signal coupling among strips in simulation.
     * Currently we have 4 couplings implemented:
     * couplingConstant["C0"]           |  Coupling from readout implant to its readout channel
     * couplingConstant["C1"]           |  Coupling from first neighbour implant (floating) to readout channel
     * couplingConstant["C2"]           |  Coupling from second neighbour implant (readout) to readout channel
     * couplingConstant["C3"]           |  Coupling from third neighbour implant (floating) to readout channel
     * couplingConstant["APVCoupling"]  |  Coupling between two adjacent channels done at APV level after charge sharing
     */
    std::map<std::string, float> couplingConstant;

    ClassDef(SVDChargeSimCal, 1); /**< needed by root*/

  };

}
