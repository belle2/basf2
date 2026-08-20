/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>
#include <TObject.h>

namespace Belle2 {

  class CDCTriggerNDFinderLUTs : public TObject {
  public:
    std::vector<unsigned short> axial;
    std::vector<unsigned short> stereo;

    ClassDef(CDCTriggerNDFinderLUTs, 1);
  };

}
