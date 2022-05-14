/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

const unsigned c_maxNSenseLayers            =    56; /**< Maximum number of sense layers. */
const unsigned c_maxNDriftCells             =   384; /**< Maximum number of drift cells per layer. */
const unsigned c_maxNFieldLayers            =    55; /**< Maximum number of field layers. */
const unsigned c_nSenseWires                = 14336; /**< Total number of sense wires. */
const unsigned c_nFieldWires                = 42240; /**< Total number of field wires. */
const unsigned c_nSuperLayers               =     9; /**< Number of super layers. */
const unsigned c_maxWireLayersPerSuperLayer =     8; /**< Maximum number of wire layers per super layers. */
const unsigned c_nBoards                    =   300; /**< Number of readout boards. */
//const unsigned nAlphaPoints  =    19;
const unsigned c_maxNAlphaPoints            =    18; /**< Number of alpha points. */
const unsigned c_maxNThetaPoints            =     7; /**< Number of theta points. */
const unsigned c_nXTParams                  =
  9; /**< Number of x-t-parameters for drift distance calibration. */ //#fit params + 1
const unsigned c_nSigmaParams               =
  9; /**< Number of sigma parameters for drift distance calibration. */ //7 is used now; 2 for spare
