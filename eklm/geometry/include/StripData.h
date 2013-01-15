/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMSTRIPDATA_H
#define EKLMSTRIPDATA_H

namespace Belle2 {

  namespace EKLM {

    /**
     * Get strip length.
     * @param[in] strip Number of strip (numbers start from 1).
     * @return Strip length (cm).
     */
    double getStripLength(int strip);

  }

}

#endif

