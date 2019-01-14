/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani, Eugenio Paoloni                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 * WARNING, ALBEIT INTENSE COHERENT LIGHT IS NOT USED AT ALL HERE:        *
 * PLEASE, PLEASE, PLEASE WEAR ALWAYS PROTECTIVE GLASSES                  *
 * WHEN LOOKING AT THIS FILE                                              *
 **************************************************************************/

#pragma once

namespace Belle2 {

  struct SVDGlobalConfigParameters {

    /** zero suppression value which define the S/N threshold of data mode acquisition during global run in ZS mode
     */
    float zeroSuppression;

    /** integer number of buffer cells to go back in the buffer ring to correctly sampling the signal pulse when the trigger is received.
     */
    int latency;

    ClassDef(SVDGlobalConfigParameters, 1);

  };

}
