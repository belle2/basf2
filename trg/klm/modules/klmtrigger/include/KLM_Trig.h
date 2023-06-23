/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once
#include <vector>
#include "trg/klm/modules/klmtrigger/KLMAxis.h"


namespace Belle2 {
  class KLM_TRG_definitions {
  public:
    enum SubdetectorSector {
      c_forward_eklm = 0,
      c_forward_bklm = 1,
      c_backward_bklm = 2,
      c_backward_eklm = 3
    };
  };


  Belle2::group_helper::KLM_trg_summery  make_trg(const  std::vector<Belle2::group_helper::KLM_Digit_compact>& hits, int eventNR,
                                                  int NLayerTrigger);

}