/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* C++ headers. */
#include <string>

namespace Belle2 {

  namespace generators {

    /**
     * Check EvtGen decay file name.
     * @param[in] decayFile Decay file.
     * @return True if the file is set up correctly.
     */
    bool checkEvtGenDecayFile(const std::string& decayFile);

  }

}
