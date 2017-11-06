/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani, Eugenio Paoloni, Giulia Casarosa             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 * WARNING, ALBEIT INTENSE COHERENT LIGHT IS NOT USED AT ALL HERE:        *
 * PLEASE, PLEASE, PLEASE WEAR ALWAYS PROTECTIVE GLASSES                  *
 * WHEN LOOKING AT THIS FILE                                              *
 **************************************************************************/

#pragma once

namespace Belle2 {

  struct SVDTriggerBinDependentConstants {

    /** correction for trigger bin 0
     */
    float bin0;

    /** correction for trigger bin 1
     */
    float bin1;

    /** correction for trigger bin 2
     */
    float bin2;

    /** correction for trigger bin 3
     */
    float bin3;


    ClassDef(SVDTriggerBinDependentConstants, 1);

  };

}
