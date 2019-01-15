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

  struct SVDLocalConfigParameters {

    /** charge in electrons injected in each strip to calibrate the pulse gain during the calibration run
     */
    float injectedCharge;

    /** Accelerator RF expressed in ns used as APV clock time unit
     */
    float timeUnits;

    /** masking bitmap
     */
    int maskFilter;

    /** time stamp with date and hour of whne the local runs for the current calibration have been taken
     */
    std::string calibDate;

    ClassDef(SVDLocalConfigParameters, 1);

  };

}
