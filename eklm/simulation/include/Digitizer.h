/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMDIGITIZER_H
#define EKLMDIGITIZER_H

#include <eklm/dbobjects/EKLMDigitizationParameters.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * Set default digitization parameters.
     * @param[out] digPar Digitization parameters.
     */
    void setDefDigitizationParams(EKLMDigitizationParameters* digPar);

  }

}

#endif

