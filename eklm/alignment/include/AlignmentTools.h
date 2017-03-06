/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMALIGNMENTTOOLS_H
#define EKLMALIGNMENTTOOLS_H

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMAlignment.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * Fill EKLMAlignment with zero displacements.
     * @param[in,out] alignment EKLMAlignment dbobject.
     */
    void fillZeroDisplacements(EKLMAlignment* alignment);

  }

}

#endif

