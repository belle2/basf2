#pragma once
#include <vector>
#include "KLMAxis.h"


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


  Belle2::KLM_trg_summery  make_trg(std::vector<Belle2::KLM_Digit_compact>& hits, int eventNR, int NLayerTrigger,
                                    const std::vector<int>& layersUsed);

}