#pragma once
#include <vector>
#include "KLMAxis.h"


namespace Belle2 {
  class KLM_TRG_definitions {
  public:
    enum SubdetectorSector {
      i_forward_eklm = 0,
      i_forward_bklm = 1,
      i_backward_bklm = 2,
      i_backward_eklm = 3
    };
  };


  Belle2::KLM_trg_summery  make_trg(std::vector<Belle2::KLM_Digit_compact>& hits, int eventNR, int NLayerTrigger,
                                    const std::vector<int>& layersUsed);

}